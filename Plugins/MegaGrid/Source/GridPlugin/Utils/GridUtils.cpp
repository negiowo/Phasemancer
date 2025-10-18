// Copyright Two Bit Studios 2025. All Rights Reserved.

#include "GridUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GridPlugin/Public/GridSubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

#include <cmath>

UGridSubsystem* UGridUtils::CachedGridSubsystem = nullptr;

// Sets default values
UGridUtils::UGridUtils()
{
	CachedGridSubsystem = GetGridSubsystem();
}

UGridSubsystem* UGridUtils::GetGridSubsystem()
{
	// Gets a new or cached GridSubsystem.

	if (!CachedGridSubsystem && GEngine)
	{
		CachedGridSubsystem = GEngine->GetEngineSubsystem<UGridSubsystem>();
	}

	return CachedGridSubsystem;
}

FVector2D UGridUtils::GetGridSize()
{
	// Returns the current save's grid size.

	return GetGridSubsystem()->GridSize;;
}

bool UGridUtils::GetIsHex()
{	
	// Returns the current save's bIsHex.

	return GetGridSubsystem()->GetIsHex();
}

FIntPoint UGridUtils::GetTileCount()
{
	// Returns current save's tile count.

	if (UGridSubsystem* GridSubsystem = GetGridSubsystem())
	{
		return GridSubsystem->GetTileCount();
	}
	
	return FIntPoint();
}

float UGridUtils::GetLineOpacity()
{
	// Returns current save's line opacity.

	if (UGridSubsystem* GridSubsystem = GetGridSubsystem())
	{
		return GridSubsystem->LineOpacity;
	}

	return 1;
}

float UGridUtils::GetLineWidth()
{
	// Returns current save's line width.

	if (UGridSubsystem* GridSubsystem = GetGridSubsystem())
	{
		return GridSubsystem->GetLineWidth();
	}

	return 10;
}

float UGridUtils::GetTileOpacity()
{
	// Returns current save's tile opacity.

	if (UGridSubsystem* GridSubsystem = GetGridSubsystem())
	{
		return GridSubsystem->TileOpacity;
	}

	return 0;
}

FLinearColor UGridUtils::GetTileColor()
{
	// Returns current save's tile color.

	if (UGridSubsystem* GridSubsystem = GetGridSubsystem())
	{
		return GridSubsystem->TileColor;
	}

	return FLinearColor::Yellow;
}

FLinearColor UGridUtils::GetLineColor()
{
	// Returns current save's line color.

	if (UGridSubsystem* GridSubsystem = GetGridSubsystem())
	{
		return GridSubsystem->LineColor;
	}

	return FLinearColor::Black;
}

FVector2D UGridUtils::GetGridOffset()
{
	// Returns current grid offset.

	return GetGridSubsystem()->GridOffset;
}

bool UGridUtils::IsEven(const int& Num)
{
	// Checks if the given int is even.

	if (abs(Num) < 0)
	{
		return true;
	}	
	else 
	{
		if (Num % 2 == 0)
		{
			return true;
		}
	}
	
	return false;
}

bool UGridUtils::IsTileWithinBounds(FIntPoint TileIndex, bool bIsHex)
{
	// Checks if the given tile index is within tile boundaries.
	// NOTE: Hex grid requires precomputed Start and End tile indices,
	// these are usually calculated during grid generation.

	UGridSubsystem* GridSubsystem = GetGridSubsystem();

	FIntPoint TileCount = GridSubsystem->GetTileCount();
	FIntPoint StartIndex = GridSubsystem->StartIndex;
	FIntPoint EndIndex = GridSubsystem->EndIndex;

	if (TileCount.X % 2 == 1 && TileCount.Y % 2 == 1)
	{
		EndIndex += FIntPoint(0, 1);
	}

	int MinX = -TileCount.X / 2;
	int MaxX = (TileCount.X % 2 == 0) ? (TileCount.X / 2 - 1) : TileCount.X / 2;

	int MinY = -TileCount.Y / 2;
	int MaxY = (TileCount.Y % 2 == 0) ? (TileCount.Y / 2 - 1) : TileCount.Y / 2;

	if (bIsHex)
	{
		int32 FloorX = FMath::Floor(TileCount.X / 2);
		int32 MinQ = (TileCount.X % 2 == 0 ? -FloorX + 1 : -FloorX);
		int32 MaxQ = FloorX;

		// Bounds for R (Index.Y)
		int32 MinR = -TileCount.Y;
		int32 MaxR = TileCount.Y - 1; // Exclusive in original loop, so subtract 1

		// Check if Q is within bounds
		if (TileIndex.X < MinQ || TileIndex.X > MaxQ)
		{
			return false;
		}

		// Check if R is within bounds
		if (TileIndex.Y < MinR || TileIndex.Y > MaxR)
		{
			return false;
		}

		// Parity check for hex grid validity
		if ((FMath::Abs(TileIndex.X) + FMath::Abs(TileIndex.Y)) % 2 != 0)
		{
			return false;
		}

		// Index is within bounds
		return true;

	}
	else
	{
		return TileIndex.X >= MinX && TileIndex.X <= MaxX &&
			TileIndex.Y >= MinY && TileIndex.Y <= MaxY;
	}
}

FIntPoint UGridUtils::DoubleToAxial(FIntPoint Index)
{
	// Convert doubled coordinates into axial.
	
	int q = Index.X;
	int r = (Index.Y - Index.X) / 2;;

	return FIntPoint(q, r);
}

FIntPoint UGridUtils::AxialToDouble(FIntPoint Axial)
{
	// Convert axial coordinates to doubled.

	int col = Axial.X;
	int row = 2 * Axial.Y + Axial.X;
	
	return FIntPoint(col, row);
}

FIntPoint UGridUtils::ConvertLocationToAxial(FVector Location)
{	
	// Convert given location to axial coordinates.

	FVector2D GridSize = GetGridSize();
	
	float q = (sqrt(3.0f) / 3.0f * Location.X - 1.0f / 3.0f * Location.Y) / GridSize.X;
	float r = (2.0f / 3.0f * Location.Y) / GridSize.Y;

	// Round q and r to nearest integers to get the axial coordinates
	int32 roundedQ = FMath::RoundToInt(q);
	int32 roundedR = FMath::RoundToInt(r);
	
	return FIntPoint(roundedQ, roundedR);
}

FVector UGridUtils::AxialToCube(FIntPoint Axial)
{	
	// Convert axial coordinates to cube.

	int q = Axial.X;
	int r = Axial.Y;
	int s = -q - r;

	return FVector(q, r, s);
}

FIntPoint UGridUtils::CubeToAxial(FVector Cube)
{	
	// Convert cube coordinates to axial.

	int q = Cube.X;
	int r = Cube.Y;

	return FIntPoint(q, r);
}

TArray<FIntPoint> UGridUtils::GetIndicesUnderBrush(FIntPoint CursorIndex, int32 BrushSize, FIntPoint Offset)
{
	// Get tile indices under brush area.

	TArray<FIntPoint> AffectedIndices;

	bool bIsHex = GetGridSubsystem()->GetIsHex();

	if (!bIsHex)
	{
		for (int32 X = -BrushSize + Offset.X; X <= BrushSize; ++X)
		{
			for (int32 Y = -BrushSize + Offset.Y; Y <= BrushSize; ++Y)
			{
				FIntPoint NewIndex = CursorIndex + FIntPoint(X, Y);
			
				if (IsTileWithinBounds(NewIndex, false))
				{
					AffectedIndices.Add(NewIndex);
				}	
			
			}
		}
	}

	else
	{
		FIntPoint Center = UGridUtils::DoubleToAxial(CursorIndex); // Convert to Axial coordinates for Brush calculations

		// Hexagonal directions in axial (q, r) coordinates
		FIntPoint Directions[6] = {
			FIntPoint(1, 1),
			FIntPoint(1, -1),
			FIntPoint(0, 2),
			FIntPoint(0, -2),
			FIntPoint(-1, 1),
			FIntPoint(-1, -1)
		};

		for (int32 q = -BrushSize; q <= BrushSize; ++q)
		{
			// Calculate bounds for r
			int32 rMin = FMath::Max(-BrushSize, -q - BrushSize);
			int32 rMax = FMath::Min(BrushSize, -q + BrushSize);

			for (int32 r = rMin; r <= rMax; ++r)
			{
				// Calculate axial coordinates (q, r)
				FIntPoint HexCoordinate = Center + FIntPoint(q, r);
				HexCoordinate = UGridUtils::AxialToDouble(HexCoordinate); // Convert back to Double Coordinates

				if (IsTileWithinBounds(HexCoordinate, true))
				{
					AffectedIndices.Add(HexCoordinate);
				}
			}
		}
	}

	return AffectedIndices;
}

TArray<FIntPoint> UGridUtils::GetNeighboringIndices(FIntPoint TileIndex, bool bIsHex, bool bUseDiagonals)
{
	// Get neighboring tile indices for the given tile index.
	
	UGridSubsystem* GridSubsystem = GetGridSubsystem();
	TArray<FIntPoint> Neighbors;

	int32 TileCountX = GridSubsystem->GetTileCount().X;
	int32 TileCountY = GridSubsystem->GetTileCount().Y;

	auto IsValidIndex = [bIsHex](FIntPoint Index) -> bool {
		return UGridUtils::IsTileWithinBounds(Index, bIsHex);
		};

	// Square

	if (!bIsHex)
	{
		FIntPoint North = TileIndex + FIntPoint(1, 0);
		FIntPoint East = TileIndex + FIntPoint(0, 1);
		FIntPoint West = TileIndex + FIntPoint(0, -1);
		FIntPoint South = TileIndex + FIntPoint(-1, 0);

		if (IsValidIndex(North)) Neighbors.Add(North);
		if (IsValidIndex(East)) Neighbors.Add(East);
		if (IsValidIndex(West)) Neighbors.Add(West);
		if (IsValidIndex(South)) Neighbors.Add(South);

		// Add extra neighbors when using diagonals.
		if (bUseDiagonals) {
			FIntPoint NorthEast = TileIndex + FIntPoint(1, 1);
			FIntPoint NorthWest = TileIndex + FIntPoint(1, -1);
			FIntPoint SouthEast = TileIndex + FIntPoint(-1, 1);
			FIntPoint SouthWest = TileIndex + FIntPoint(-1, -1);

			if (IsValidIndex(NorthEast)) Neighbors.Add(NorthEast);
			if (IsValidIndex(SouthEast)) Neighbors.Add(SouthEast);
			if (IsValidIndex(NorthWest)) Neighbors.Add(NorthWest);
			if (IsValidIndex(SouthWest)) Neighbors.Add(SouthWest);
		}

		return Neighbors;
	}

	//Hex 

	else
	{

		FIntPoint NorthEast = TileIndex + FIntPoint(1, 1);
		FIntPoint NorthWest = TileIndex + FIntPoint(1, -1);
		FIntPoint East = TileIndex + FIntPoint(0, 2);
		FIntPoint West = TileIndex + FIntPoint(0, -2);
		FIntPoint SouthEast = TileIndex + FIntPoint(-1, 1);
		FIntPoint SouthWest = TileIndex + FIntPoint(-1, -1);

		if (IsValidIndex(NorthEast)) Neighbors.Add(NorthEast);
		if (IsValidIndex(NorthWest)) Neighbors.Add(NorthWest);
		if (IsValidIndex(West)) Neighbors.Add(West);
		if (IsValidIndex(East)) Neighbors.Add(East);
		if (IsValidIndex(SouthEast)) Neighbors.Add(SouthEast);
		if (IsValidIndex(SouthWest)) Neighbors.Add(SouthWest);

		return Neighbors;
	}
}

EGridVisualContext UGridUtils::ResolveGridVisualContext(EGridVisualContext Context)
{	
	// Automatically resolves grid visual context. 
	// Useful if you don't know when to use Runtime or Editor modes.

	if (Context == EGridVisualContext::Auto)
	{
		if (GetGridSubsystem()->GetIsRuntime())
		{
			return EGridVisualContext::Runtime;
		}

		else
		{
			return EGridVisualContext::Editor;
		}
	}
	
	return Context;
}

float UGridUtils::GetScaleBasedOnNormal(const FVector& Normal, const float Min, const float Max)
{
	// Returns a scale factor determined by the hit normal.
	// Used to scale HISMC instances to avoid artifacts on terrain slopes.

	float SlopeFactor = FVector::DotProduct(Normal, FVector::UpVector); // Value between 0 and 1
	float NewScale = FMath::Lerp(Max, Min, SlopeFactor); // Adjust scale

	return NewScale;
}

FIntPoint UGridUtils::GetIndexFromLocation(const FVector& Location, bool bIsHexagon)
{
	// Gets the corresponding tile index for the given world position.
	// Procedurally determined.

	FIntPoint Index;
	FVector2D GridSize = UGridUtils::GetGridSize();
	FVector2D Offset = UGridUtils::GetGridOffset();
	FVector GridOffset(Offset.X, Offset.Y, 0);

	// Square
	if (!bIsHexagon) {

		FVector SnappedLocation = SnapVectorToGrid(Location, GridSize, GridOffset);

		Index = FIntPoint(FMath::FloorToInt(SnappedLocation.X / GridSize.X),
			FMath::FloorToInt(SnappedLocation.Y / GridSize.Y));

		return Index;
	}

	// Hex
	// Dear Genie, please make my hex implementation look small, please. 

	else 
	{
		float x, y, z = 0;

		x = round((Location.X / GridSize.X) * (2.0 / 3.0));

		if (!IsEven(static_cast<int>(FMath::RoundToInt(x))))
		{
			z = GridSize.X / 1.25;
		}

		y = round(((1.0 / 3.0) * (sqrtf(3.0) * (Location.Y + z)) - (1.0 / 3.0)) / GridSize.Y);

		// 

		if (!IsEven(static_cast<int>(FMath::RoundToInt(x))) && y <= 0)
		{
			y -= 1.0;
		}

		//

		if (IsEven(static_cast<int>(FMath::RoundToInt(x))))
		{
			y *= 2.0;
		}

		else
		{
			
			if (y < 0.0)
			{
				y = ((abs(y) * 2.0) - 1.0) * -1.0;
			}
			else
			{
				y = (abs(y) * 2.0) - 1.0;
			}
		}

		Index = FIntPoint(x, y);
		return Index;
	}

	return Index;
}

FVector UGridUtils::GetLocationFromIndex(const FIntPoint& Index, bool bIsHexagon)
{
	// Gets the tile location for the given tile index. Always snapped to grid.

	FVector Location;
	FVector2D GridSize = UGridUtils::GetGridSize();
	FVector2D Offset = UGridUtils::GetGridOffset();
	FVector GridOffset(Offset.X, Offset.Y, 0);

	// Square

	if (!bIsHexagon) {

		Location = FVector(GridOffset.X + (Index.X * GridSize.X), GridOffset.Y + (Index.Y * GridSize.Y), 0);
		return Location;
	}

	// Hex

	else
	{
		float x, y;

		if (!IsEven(Index.X) && IsEven(Index.Y))
		{
			x = Index.X - 1.0;
			y = FMath::RoundToInt((Index.Y / 2.0) - 1.0) + 0.5;
		}

		//

		else if (IsEven(Index.X) && !IsEven(Index.Y))
		{
			x = Index.X + 1.0;
			y = FMath::RoundToInt(Index.Y / 2.0) - 0.5;	
		}

		//

		else 
		{
			x = Index.X;
			
			if (IsEven(Index.X))
			{
				y = (Index.Y / 2.0);
			}
			else 
			{
				y = FMath::RoundToInt((Index.Y / 2.0) - 1.0);
			}
		}

		float HX, HY;

		float HW = 0.0;

		if (!IsEven(Index.X))
		{
			HW = 0.5;
		}

		HX = GridSize.X * (x * 1.5);
		HY = GridSize.Y * (sqrt(3.0) * (y + HW));

		Location = FVector(HX, HY, 0);
		return Location;
	}

	return Location;
}

FVector UGridUtils::GetSurfaceLocation(UObject* WorldContextObject, const FVector& Location, 
	UObject* IgnoredObject, bool& bOutHit)
{
	// Returns the surface hit location.
	// Uses the GridSurface trace channel.

	FVector Start = Location + FVector(0, 0, 99999);
	FVector End = Location - FVector(0, 0, 99999);
	bool bIsHex = GetIsHex();


	UWorld* World = WorldContextObject->GetWorld();

	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		return FVector();
	}

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	ECollisionChannel TraceChannel = GetGridSubsystem()->GridSurfaceChannel;
	
	// Ignore given object.
	if (IgnoredObject)
	{
		if (AActor* IgnoredActor = Cast<AActor>(IgnoredObject))
		{
			CollisionParams.AddIgnoredActor(IgnoredActor);
		}
		else if (UPrimitiveComponent* IgnoredComponent = Cast<UPrimitiveComponent>(IgnoredObject))
		{
			CollisionParams.AddIgnoredComponent(IgnoredComponent);
		}
	}

	FCollisionQueryParams QueryParams;
	if (WorldContextObject->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, CollisionParams))
	{
		if (HitResult.bBlockingHit && HitResult.GetActor()->ActorHasTag("GridSurface"))
		{
			bOutHit = true;

			// Return the hit location
			return HitResult.Location;
		}
	}

	bOutHit = false;
	return Location;
}

FIntPoint UGridUtils::GetTileUnderCursor(UObject* WorldContextObject, APlayerController* PlayerController)
{
	// Gets the tile index under mouse cursor.
	// Requires player controller.
	
	FHitResult HitResult;
	FIntPoint TileIndex = FIntPoint();

	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{

		UE_LOG(LogTemp, Warning, TEXT("No World"));
		return TileIndex;
	}

	ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(GetGridSubsystem()->GridSurfaceChannel);
	PlayerController->GetHitResultUnderCursorByChannel(TraceChannel, true, HitResult);

	if (HitResult.bBlockingHit)
	{
		TileIndex = GetIndexFromLocation(HitResult.ImpactPoint, GetIsHex());
		return TileIndex;
	}

	return FIntPoint(999, 999);
}

FVector UGridUtils::SnapVectorToGrid(const FVector& Vector, const FVector2D& Grid, const FVector& Offset)
{	
	// Snaps a vector (location) to a square grid. 
	// I don't recommend using this for hex grids or dynamic grids.
	// If you want a snap system see SnapLocationToGrid().
	// However this is very inexpensive.

	FVector OffsetVector = Vector + Offset;
	
	float SnappedX = FMath::GridSnap(OffsetVector.X, Grid.X);
	float SnappedY = FMath::GridSnap(OffsetVector.Y, Grid.Y);

	OffsetVector = FVector(SnappedX, SnappedY, Offset.Z);
	OffsetVector -= Offset;

	return OffsetVector;
}

FVector UGridUtils::SnapLocationToGrid(const FVector& Location, bool LimitToGrid)
{
	// Snaps the given world location to grid. 
	// Doesn't account for surface fluctuations, use
	// GetSurfaceLocation() after this.
	
	FVector SnappedLocation;
	FIntPoint TileIndex;
	bool bIsHex = GetGridSubsystem()->GetIsHex();

	// Convert the location to tile index.
	TileIndex = GetIndexFromLocation(Location, bIsHex);
	
	// Should we allow extra-boundary snaps?
	if (LimitToGrid)
	{
		if (IsTileWithinBounds(TileIndex, bIsHex))
		{
			SnappedLocation = GetLocationFromIndex(TileIndex, bIsHex);
			return SnappedLocation;
		}

		else 
		{
			return SnappedLocation;
		}
	}

	// Get back a snapped location from tile index.
	SnappedLocation = GetLocationFromIndex(TileIndex, bIsHex);

	return SnappedLocation;
}

bool UGridUtils::IsTileOfState(FIntPoint TileIndex, FName State)
{
	// Checks if the given tile contains the given state.

	TMap<FIntPoint, FTileData>& GridData = CachedGridSubsystem->GetGridData();

	if (CachedGridSubsystem && !GridData.IsEmpty())
	{
		FTileData* TempNode = GridData.Find(TileIndex);
		
		if (TempNode)
		{			
			if (TempNode->TileState.Contains(State))
			{
				return true;
			}

			return false;
		}
	}

	return false;
}

bool UGridUtils::IsTileOfType(FIntPoint TileIndex, FName Type)
{
	// Checks if the tile is of type.

	TMap<FIntPoint, FTileData>& GridData = CachedGridSubsystem->GetGridData();

	if (CachedGridSubsystem && !GridData.IsEmpty())
	{
		FTileData* TempNode = GridData.Find(TileIndex);

		if (TempNode != nullptr)
		{
			if (TempNode->TileType == Type)
			{
				return true;
			}

			return false;
		}
	}
	return false;
}
