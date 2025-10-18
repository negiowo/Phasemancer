// Copyright Two Bit Studios 2025. All Rights Reserved.

#include "GridManager.h"
#include "GridSubsystem.h"
#include "Async/Async.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

// Sets default values
AGridManager::AGridManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	World = GetWorld();
	GridSubsystem = UGridUtils::GetGridSubsystem();
	GridSubsystem->SetRuntimeMode(true);

	ClearAllStates(false);
}

void AGridManager::EndPlay(EEndPlayReason::Type EndReason)
{
	Super::EndPlay(EndReason);

	FScopeLock Lock(&GridSubsystem->Mutex);
	// Set bIsRuntime to false.
	GridSubsystem->SetRuntimeMode(false);
}

void AGridManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Using OnConstruction for when BP compiles.
	GridSubsystem = UGridUtils::GetGridSubsystem();
}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AGridManager::IsCoreDependenciesValid()
{
	if (!IsValid(GridVisuals) || !IsValid(GridSubsystem) || GridVisuals->HasAnyFlags(RF_Transient))
	{
		UE_LOG(LogTemp, Error, TEXT("GridManager Core Dependencies Not Valid"));

		return false;
	}

	return true;
}

void AGridManager::SetGridSize(FVector2D GridSize)
{
	// Sets grid size. This will wipe existing tile visuals.
	// Consider remapping data to new size.
	
	if (!IsCoreDependenciesValid())
	{
		return;
	}
	
	GridSubsystem->SetGridSize(GridSize);
	GridVisuals->SetGridSize(GridSize);	
	GridVisuals->DestroyVisuals(EGridVisualContext::Editor);
}

void AGridManager::SetTileCount(FVector2D TileCount)
{
	// Sets tile count. This will wipe existing tile visuals.
	// Consider remapping data to new tile count.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->SetTileCount(TileCount);
	GridSubsystem->SetTileCount(FIntPoint(TileCount.X, TileCount.Y));
	GridVisuals->DestroyVisuals(EGridVisualContext::Editor);
}

void AGridManager::SetLineWidth(float Width)
{
	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->SetLineWidth(Width);
	GridSubsystem->SetLineWidth(Width);
}

void AGridManager::SetLineColor(FLinearColor Color)
{
	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->SetColor(Color, true);
	GridSubsystem->SetColor(Color, true);
}

void AGridManager::SetLineOpacity(float Opacity)
{
	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->SetOpacity(Opacity, true);
	GridSubsystem->SetOpacity(Opacity, true);
}

TMap<FIntPoint, FTileData> AGridManager::GetGridData()
{
	// Gets the grid data from GridSubsystem. Save specific.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid()) 
	{
		return TMap<FIntPoint, FTileData>();
	}
	
	return GridSubsystem->GetGridData();
}

FTileData AGridManager::GetTileData(FIntPoint Index)
{
	// Gets the tile data from GridSubsystem. Save specific.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return FTileData();
	}

	return GridSubsystem->GetTileData(Index);
}

TArray<FIntPoint> AGridManager::GetAllTilesOfType(FName Type)
{
	// Gets all tiles in the grid of given type.
	
	if (!IsCoreDependenciesValid())
	{
		return TArray<FIntPoint>{};
	}

	TArray<FIntPoint> TilesOfType;
	TMap<FIntPoint, FTileData>& GridDataRef = GridSubsystem->GetGridData();

	for (auto It = GridDataRef.CreateIterator(); It; ++It)
	{
		FIntPoint Key = It.Key();      
		FTileData& Value = It.Value();   

		if (Value.TileType == Type)
		{
			TilesOfType.Add(Key);
		}
	}
	
	return TilesOfType;
}

TArray<FIntPoint> AGridManager::GetAllTilesOfState(FName State)
{
	// Gets all tiles in the grid of given state.

	if (!IsCoreDependenciesValid())
	{
		return TArray<FIntPoint>{};
	}

	TArray<FIntPoint> TilesOfState;
	TMap<FIntPoint, FTileData>& GridDataRef = GridSubsystem->GetGridData();

	for (auto It = GridDataRef.CreateIterator(); It; ++It)
	{
		FIntPoint Key = It.Key();         
		FTileData& Value = It.Value();

		if (Value.TileState.Contains(State))
		{
			TilesOfState.Add(Key);
		}
	}

	return TilesOfState;
}

void AGridManager::SetGridColor(FLinearColor Color)
{
	// Sets grid color. Save specific.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->SetColor(Color, false);
	GridSubsystem->SetColor(Color, false);
}

void AGridManager::SetGridOpacity(float Opacity)
{
	// Sets grid opacity. Save specific.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->SetOpacity(Opacity, false);
	GridSubsystem->SetOpacity(Opacity, false);
}

void AGridManager::SetGridData(TMap<FIntPoint, FTileData> NewGridData)
{
	// Sets / overwrites grid data in existing save file.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridSubsystem->SetGridData(NewGridData);
}

void AGridManager::SetTileData(FTileData NewTileData)
{
	// Sets / overwrites tile data of the containing tile index.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridSubsystem->SetTileData(NewTileData);
}

void AGridManager::SetGridShape(EGridShape Shape)
{
	// Sets current grid shape

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->SwitchGridShape(Shape);
	GridSubsystem->SetIsHex(Shape == EGridShape::Hex);
}

void AGridManager::SetSurfaceTraceChannel(ECollisionChannel Channel)
{
	// Sets trace channel to be used as grid surface.

	if (!GridSubsystem)
	{
		GridSubsystem = UGridUtils::GetGridSubsystem();
	}

	GridSubsystem->GridSurfaceChannel = Channel;
}

void AGridManager::SetAutoMapTraceChannel(ECollisionChannel Channel)
{
	// Sets trace channel to be used for auto mapping tiles.

	if (!GridSubsystem)
	{
		GridSubsystem = UGridUtils::GetGridSubsystem();
	}

	GridSubsystem->AutoMapChannel = Channel;
}

void AGridManager::NotifyGridChanged()
{
	// NOTE: Leaving broadcast unwrapped from WITH_EDITOR because
	// it can be used in runtime systems. Idk how, but it can be.

	OnShapeChanged.Broadcast();
}

void AGridManager::AddTileVisualToGrid(FIntPoint Index, EGridVisualContext Context)
 {
	// Adds or updates tile visual.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	// Resolve Context if default value of Auto is used.
	Context = UGridUtils::ResolveGridVisualContext(Context);

	if (Context == EGridVisualContext::Runtime)
	{
		GridVisuals->AddTileVisualRuntime(Index);
	}

#if WITH_EDITOR

	else
	{
		GridVisuals->AddTileVisualEditor(Index);
	}

#endif
}

void AGridManager::ClearAllTilesOfState(FName State, bool bReloadTiles)
{
	// State related functions are only meant to be used in runtime.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	TMap<FIntPoint, FTileData>& GridDataRef = GridSubsystem->GetGridData();

	for (auto It = GridDataRef.CreateIterator(); It; ++It)
	{
		FIntPoint Key = It.Key();        
		FTileData& Value = It.Value();   

		if (Value.TileState.Contains(State))
		{
			Value.TileState.Remove(State);

			if (bReloadTiles)
			{
				if (GridSubsystem->GetIsRuntime())
				{
					GridVisuals->AddTileVisualRuntime(Key);
				}
				else
				{
					// This editor context is exclusive to internal code, not recommended for
					// development.
#if WITH_EDITOR
					GridVisuals->AddTileVisualEditor(Key);
#endif
				}
			}
		}
	}
}

void AGridManager::ClearAllStates(bool bReloadTiles)
{
	// State related functions are only meant to be used in runtime.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	FScopeLock Lock(&GridSubsystem->Mutex);
	TMap<FIntPoint, FTileData>& GridDataRef = GridSubsystem->GetGridData();

	for (auto It = GridDataRef.CreateIterator(); It; ++It)
	{
		FIntPoint Key = It.Key();
		FTileData& Value = It.Value();

		Value.TileState.Empty();

		if (bReloadTiles)
		{
			if (GridSubsystem->GetIsRuntime())
			{
				GridVisuals->AddTileVisualRuntime(Key);
			}
			else
			{
				// This editor context is exclusive to internal code, not recommended for
				// development.

#if WITH_EDITOR
				GridVisuals->AddTileVisualEditor(Key);
#endif
			}
		}
	}
}

void AGridManager::ClearAllTypes(EGridVisualContext Context, 
	bool bReloadTiles)
{

	// Clear all types of the entire grid. 
	// Resets to Default.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	// Resolve Context if default value of Auto is used.
	Context = UGridUtils::ResolveGridVisualContext(Context);

	TMap<FIntPoint, FTileData>& GridDataRef = GridSubsystem->GetGridData();

	for (auto It = GridDataRef.CreateIterator(); It; ++It)
	{
		FIntPoint Key = It.Key();
		FTileData& Value = It.Value();
		Value.TileType = "Default";
	}

	// Once data is modified, pass reload visuals to GridVisuals.
	if (bReloadTiles)
	{
		GridVisuals->ProcessVisualsAsync(Context);
	}
}

void AGridManager::ClearAllTilesOfType(FName Type, EGridVisualContext Context,
	bool bReloadTiles)
{

	// Clear all tiles of type.
	// Resets to Default.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	// Resolve Context if default value of Auto is used.
	Context = UGridUtils::ResolveGridVisualContext(Context);

	TMap<FIntPoint, FTileData>& GridDataRef = GridSubsystem->GetGridData();

	for (auto It = GridDataRef.CreateIterator(); It; ++It)
	{
		FIntPoint Key = It.Key();         
		FTileData& Value = It.Value(); 

		if (Value.TileType == Type)
		{
			Value.TileType = "Default";
		}

		if (bReloadTiles)
		{
			if (GridSubsystem->GetIsRuntime())
			{
				GridVisuals->AddTileVisualRuntime(Key);
			}
			else
			{
				// This editor context is exclusive to internal code, not recommended for
				// development.

#if WITH_EDITOR
				GridVisuals->AddTileVisualEditor(Key);
#endif
			}
		}
	}

	
}

void AGridManager::AddStateToTile(FIntPoint Index, FName State, bool bReloadTile, 
	bool bScopeLock)
{
	// Adds the given state to tile.
	// Each tile can multiple states.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	if (bScopeLock)
	{
		FScopeLock Lock(&GridSubsystem->Mutex);
	}

	// Get current grid data reference.
	TMap<FIntPoint, FTileData>& GridData = GridSubsystem->GetGridData();

	// Find the corresponding tile index.
	FTileData* Node = GridData.Find(Index);

	if (Node)
	{
		// if found add the new state.
		Node->TileState.AddUnique(State);
	}
	
	if (bReloadTile)
	{
		// Once added, visuals can be refreshed.
		GridVisuals->AddTileVisualRuntime(Index);
	}
}

void AGridManager::RemoveStateFromTile(FIntPoint Index, FName State, bool bReloadTile,
	bool bScopeLock)
{
	// Removes given state from tile.
	// Each tile can have multiple states.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	if (bScopeLock)
	{
		FScopeLock Lock(&GridSubsystem->Mutex);
	}

	// Get current grid data reference.
	TMap<FIntPoint, FTileData>& GridData = GridSubsystem->GetGridData();
	
	// Find the corresponding tile index.
	FTileData* Node = GridData.Find(Index);

	if (Node)
	{	
		// If found remove the state.
		Node->TileState.Remove(State);
	}

	if (bReloadTile)
	{
		// Once removed, visuals can be refreshed.
		GridVisuals->AddTileVisualRuntime(Index);
	}
	
}

void AGridManager::SetTileType(FIntPoint TileIndex, FName Type,
	EGridVisualContext Context, bool bReloadTile)
{
	// Sets given type to the tile.
	// Each tile can only have one type.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	Context = UGridUtils::ResolveGridVisualContext(Context);
	
	// Get current grid data reference.
	TMap<FIntPoint, FTileData>& GridData = GridSubsystem->GetGridData();
	
	// Find the corresponding tile index.
	FTileData* Node = GridData.Find(TileIndex);

	if (Node)
	{
		// If found set the type
		Node->TileType = Type;
	}

	if (bReloadTile)
	{
		// After updates, visuals can be refreshed.
		if (Context == EGridVisualContext::Runtime)
		{
			GridVisuals->AddTileVisualRuntime(TileIndex);
		}
		else 
		{

#if WITH_EDITOR
			GridVisuals->AddTileVisualEditor(TileIndex);
#endif
		}	
	}
	
}

void AGridManager::UpdateTileTransform(EGridVisualContext Context, FIntPoint TileIndex, FTransform InTransform)
{
	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	FTileData TileData = GetTileData(TileIndex);
	TileData.TileTransform = InTransform;

	GridVisuals->UpdateVisualTransform(TileIndex, InTransform, Context);
	
}

void AGridManager::RemoveTileType(FIntPoint TileIndex, FName Type,
	EGridVisualContext Context, bool bReloadTile)
{
	// Removes given type from the tile.
	// Each tile can only have one type. 
	// Resets to Default.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	Context = UGridUtils::ResolveGridVisualContext(Context);
	
	// Get current grid data reference.
	TMap<FIntPoint, FTileData>& GridData = GridSubsystem->GetGridData();

	// Find the corresponding tile index.
	FTileData* TileData = GridData.Find(TileIndex);

	if (!TileData)
	{
		return;
	}

	if (Context == EGridVisualContext::Runtime)
	{
		// Checking if type given is All or None, if true, then remove any and
		// all types. Else, remove only specified type.

		if (Type == "All" || Type.IsNone())
		{
			TileData->TileType = FName("Default");
			
			if (!bReloadTile)
				return;

			// After updates, visuals can be refreshed.
			GridVisuals->AddTileVisualRuntime(TileIndex);
		}

		else if (TileData->TileType == Type)
		{
			TileData->TileType = FName("Default");

			if (!bReloadTile)
				return;

			// After updates, visuals can be refreshed.
			GridVisuals->AddTileVisualRuntime(TileIndex);
		}
	}

#if WITH_EDITOR

	else
	{
		// Checking if type given is All or None, if true, then remove any and
		// all types. Else, remove only specified type.
		if (Type == "All" || Type.IsNone())
		{
			TileData->TileType = FName("Default");

			if (!bReloadTile)
				return;

#if WITH_EDITOR
			// After updates, visuals can be refreshed.
			GridVisuals->AddTileVisualEditor(TileIndex);
#endif
		}

		else if (TileData->TileType == Type)
		{
			TileData->TileType = FName("Default");

			if (!bReloadTile)
				return;
#if WITH_EDITOR
			// After updates, visuals can be refreshed.
			GridVisuals->AddTileVisualEditor(TileIndex);
#endif
		}
	}

#endif

}

void AGridManager::SetStateVisibility(bool Visibility, FName TileState)
{
	// Sets visibility for given state. 
	// Doesn't affect types, if the tile has
	// a type other than Default, it will be rendered instead.

	// Call ForceReload after this to update visuals.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	int32 BatchSize = 50;
	static TArray<FIntPoint> GridKeys;
	static int32 CurrentIndex = 0;

	FScopeLock Lock(&GridSubsystem->Mutex);

	// Save hidden states to avoid during color selection
	if (!Visibility)
	{
		GridVisuals->StateToNotRender.AddUnique(TileState);
	}
	else
	{
		GridVisuals->StateToNotRender.Remove(TileState);
	}
}

void AGridManager::SetTypeVisibility(EGridVisualContext Context, bool Visibility, FName TileType)
{
	// Sets visibility for given type. 
	// Doesn't affect states, if the tile has
	// a state, it will be rendered instead.

	// Call ForceReload after this to update visuals.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	FScopeLock Lock(&GridSubsystem->Mutex);

	Context = UGridUtils::ResolveGridVisualContext(Context);

	int32 BatchSize = 50;
	static TArray<FIntPoint> GridKeys;
	static int32 CurrentIndex = 0;

	// Save hidden types to avoid during color selection
	if (!Visibility)
	{
		GridVisuals->TypesToNotRender.AddUnique(TileType);
	}
	else
	{
		GridVisuals->TypesToNotRender.Remove(TileType);
	}
}

void AGridManager::GenerateGrid(FIntPoint TileCount, bool bAutoMapObstacles,
	bool bRandomObstacles, bool bRemap)
{
	// Main function for creating grid.
	// This function generates grid data and visuals to go along with it.
	
	if (TileCount.X == 0 || TileCount.Y == 0)
	{
		return;
	}

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	GridVisuals->PrecomputeColors();
	PrecomputeMovementCosts();
	
#if WITH_EDITOR
	World = GEditor->GetEditorWorldContext().World();
#endif

	if (!World) // Fallback to game world if not in editor
	{
		World = GetWorld();
	}

	const TMap<FIntPoint, FTileData>& OriginalGridData = GridSubsystem->GetGridData();

	bool bIsHex = GridSubsystem->GetIsHex();

	// Destroy previous grid
	GridSubsystem->ClearGridData();
	GridVisuals->DestroyVisuals(EGridVisualContext::Editor);

	// Tile Count is limited to 300x300 for practical purposes
	TileCount.X = FMath::Clamp(TileCount.X, 0, 300);
	TileCount.Y = FMath::Clamp(TileCount.Y, 0, 300);

	UE_LOG(LogTemp, Warning, TEXT("Clamped TC: %s"), *TileCount.ToString());

	FVector2D GridSize = GridSubsystem->GetGridSize();
	GridSize = FVector2D(FMath::Floor(GridSize.X), FMath::Floor(GridSize.Y));

	float OffsetX = (int(TileCount.X) % 2 == 0) ? 0.5f * GridSize.X : 0.0f;
	float OffsetY = (int(TileCount.Y) % 2 == 0) ? 0.5f * GridSize.Y : 0.0f;

	GridSubsystem->SetGridOffset(FVector2D(OffsetX, OffsetY));

	FVector2D GridOffset = GridSubsystem->GetGridOffset();
	TArray<FVector> TileLocations;
	FVector BottomLeftCorner = FVector(GridOffset.X, GridOffset.Y, 0) -
		FVector((TileCount.X / 2) * GridSize.X, (TileCount.Y / 2) * GridSize.Y, 0);

	float ZScale;
	
	// Generate Tile Locations
	if (!bIsHex)
	{
		// Square grid locations.
		for (int32 Row = 0; Row < TileCount.X; ++Row)
		{
			for (int32 Col = 0; Col < TileCount.Y; ++Col)
			{
				FVector Location = FVector(BottomLeftCorner.X + (Row * GridSize.X), BottomLeftCorner.Y + (Col * GridSize.Y), 0);	

				/* Use line trace to calculate Z Scale and Height offset for
				* tile instances.
				*/

				FHitResult HitResult;
				FCollisionQueryParams CollisionParams;
				bool bHit;
				
				bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Location + FVector(0, 0, 99999),
					Location - FVector(0, 0, 99999), GridSubsystem->GridSurfaceChannel, CollisionParams);

				if (bHit && HitResult.GetActor()->ActorHasTag("GridSurface"))
				{
					Location = HitResult.Location;

					// Scale each instance based on hit normal
					ZScale = UGridUtils::GetScaleBasedOnNormal(HitResult.ImpactNormal,
						TileMinScale, TileMaxScale);

					// Offset location z of each instance based on Z Scale
					float HeightOffset = (ZScale / FMath::Clamp(ZScale, 2, 1)) * 100;
					Location = Location - FVector(0, 0, HeightOffset);

					TileLocations.Add(Location);
					PendingTileLocations.Add(FTransform(FRotator(), Location, FVector(1, 1, ZScale)));
				}				
			}
		}
	}

	else
	{	
		// Hex grid locations.
		TArray<FIntPoint> Indices;

		int FloorX = FMath::Floor(TileCount.X / 2);

		for (int32 Q = (TileCount.X % 2 == 0 ? -FloorX + 1 : -FloorX);
			Q <= FloorX; ++Q)
		{
			// Loop through the row (R) axis
			for (int32 R = -TileCount.Y; R < TileCount.Y; ++R)
			{
				// Only create valid doubled coordinates if (col + row) % 2 == 0
				if ((abs(Q) + abs(R)) % 2 == 0)
				{
					// Convert normal coordinates (Q, R) to doubled coordinates (doubledX, doubledY)
					int32 DoubledX = Q; // Double the column (Q) value
					int32 DoubledY = R; // Row (R) remains the same

					// Getting index here so we can determine the 
					// start and end index for grid boundaries.
					FIntPoint HexIndex = FIntPoint(DoubledX, DoubledY);
					Indices.Add(HexIndex);
					FVector Location = UGridUtils::GetLocationFromIndex(HexIndex, true);
					
					/* Use line trace to calculate Z Scale and Height offset for
					* tile instances.
					*/

					FHitResult HitResult;
					FCollisionQueryParams CollisionParams;
					bool bHit;

					bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Location + FVector(0, 0, 99999),
						Location - FVector(0, 0, 99999), GridSubsystem->GridSurfaceChannel, CollisionParams);

					if (bHit && HitResult.GetActor()->ActorHasTag("GridSurface"))
					{
						Location = HitResult.Location;

						// Scale each instance based on hit normal
						ZScale = UGridUtils::GetScaleBasedOnNormal(HitResult.ImpactNormal,
							TileMinScale, TileMaxScale);

						// Offset location z of each instance based on Z Scale
						float HeightOffset = (ZScale / FMath::Clamp(ZScale, 2, 1)) * 100;
						Location = Location - FVector(0, 0, HeightOffset);

						TileLocations.Add(Location);
						PendingTileLocations.Add(FTransform(FRotator(), Location, FVector(1, 1, ZScale)));;
					}					
				}
			}
		}

		StartTile = Indices[0];
		EndTile = Indices[Indices.Num() - 1];

		GridSubsystem->StartIndex = StartTile;
		GridSubsystem->EndIndex = EndTile;

		Indices.Empty();
	}

	UE_LOG(LogTemp, Warning, TEXT("Started Batch Processing."));

	// Send tiles for Batch Processing
	ProcessTileBatch(bAutoMapObstacles, bRandomObstacles, bRemap);
}

void AGridManager::ForceReloadTiles(EGridVisualContext Context)
{
	// Reloads tile visuals from existing GridData

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	Context = UGridUtils::ResolveGridVisualContext(Context);

	// Recompute colors from tables
	GridVisuals->PrecomputeColors();
	GridVisuals->ProcessVisualsAsync(Context);
}

void AGridManager::PrecomputeMovementCosts()
{
	if (!TileTypeMapping)
	{
		return;
	}

	TypeMovementCostMap.Empty();

	// Get all row names from the DataTable
	const TArray<FName> RowNames = TileTypeMapping->GetRowNames();

	static const FString ContextString(TEXT("Tile Mapping Context"));

	// Iterate over each row and populate the map
	for (const FName& RowName : RowNames)
	{
		FTileTypeMapping* FoundRow = TileTypeMapping->FindRow<FTileTypeMapping>(RowName, ContextString, true);
		if (FoundRow)
		{
			// Add the row to the map
			TypeMovementCostMap.Add(FoundRow->TileTypeName, FoundRow->MovementCost);
		}
	}
}

void AGridManager::NotifyOnDataTableAssigned()
{
#if WITH_EDITOR
	OnDataTableAssigned.Broadcast();
#endif
}

void AGridManager::DestroyGrid(EGridVisualContext Context)
{
	// Destroys entire grid including visuals.

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	Context = UGridUtils::ResolveGridVisualContext(Context);

	GridSubsystem->ClearGridData();
	GridVisuals->DestroyVisuals(Context);
	
	UE_LOG(LogTemp, Warning, TEXT("Grid has been Cleared!"));
}

float AGridManager::GetMovementCost(FIntPoint TileIndex)
{
	// Get the movement cost for given tile as defined in data table.
	// Requires PrecomputeMovementCosts to be called after data table has changed.
	// WARNING: NOT SUITABLE FOR ASYNC CALLS or MULTITHREADING.

	if (!IsCoreDependenciesValid())
	{
		return 0;
	}

	TMap<FIntPoint, FTileData> GridData = GridSubsystem->GetGridData();

	if (!GridData.Contains(TileIndex)) {
		return 999;
	}

	const FTileData* Node = GridData.Find(TileIndex);

	if (!Node)
	{
		return 0;
	}

	FName TileType = Node->TileType;

	float* Cost = TypeMovementCostMap.Find(TileType);

	if (!Cost)
	{
		return 0;
	}

	return *Cost;
	
}

void AGridManager::RemapExistingTiles(EGridVisualContext Context)
{	
	// Remaps existing grid data to new one. 
	// Used in case of tile count, grid size changes.

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}
	
	Context = UGridUtils::ResolveGridVisualContext(Context);
	
	// Recompute colors from tables
	GridVisuals->PrecomputeColors();

	bool bIsHex = GridSubsystem->GetIsHex();
	FIntPoint TileCount = GridSubsystem->GetTileCount();

	TileCount.X = FMath::Clamp(TileCount.X, 0, bIsHex ? 300 : 300);
	TileCount.Y = FMath::Clamp(TileCount.Y, 0, bIsHex ? 300 : 300);

	FVector2D GridSize = GridSubsystem->GetGridSize();
	GridSize = FVector2D(FMath::Floor(GridSize.X), FMath::Floor(GridSize.Y));

	float OffsetX = (int(TileCount.X) % 2 == 0) ? 0.5f * GridSize.X : 0.0f;
	float OffsetY = (int(TileCount.Y) % 2 == 0) ? 0.5f * GridSize.Y : 0.0f;

	GridSubsystem->SetGridOffset(FVector2D(OffsetX, OffsetY));
	
	float ZScale = 1;
	TMap<FIntPoint, FTileData> NewGridData;
	TMap<FIntPoint, FTileData> GridDataCopy = GridSubsystem->GetGridData();

	// Go through the existing grid data and remap it to new transforms and new data.
	for (auto& Elem : GridDataCopy)
	{
		FTileData NewGridNode = Elem.Value;
		FVector Location = Elem.Value.TileTransform.GetLocation();

		FIntPoint NewIndex = UGridUtils::GetIndexFromLocation(Location, bIsHex);
		
		if (UGridUtils::IsTileWithinBounds(NewIndex, bIsHex))
		{
			bool bOutHit;
			Location = UGridUtils::GetLocationFromIndex(NewIndex, bIsHex);
			Location = UGridUtils::GetSurfaceLocation(GetWorld(), Location, this, bOutHit);

			NewGridNode.Index = NewIndex;
			NewGridNode.TileTransform.SetLocation(Location);
			NewGridData.Add(NewIndex, NewGridNode);
		}
	}

	// Remapped grid data becomes new data
	RemappedGridData = NewGridData;
	GridSubsystem->ClearGridData();
	GridSubsystem->SetGridData(NewGridData);

	// Visual call is done via BP, calls GenerateGrid.
}

TArray<FVector> AGridManager::GenerateGridLocations(FIntPoint TileCount, bool bUseSurface)
{
	// Only generates grid locations.
	
	TArray<FVector> TileLocations;

	if (!IsCoreDependenciesValid())
	{
		return TileLocations;
	}
	
	bool bSurfaceHit;
	bool bIsHex = GridSubsystem->GetIsHex();

	TileCount.X = FMath::Clamp(TileCount.X, 0, 305);
	TileCount.Y = FMath::Clamp(TileCount.Y, 0, 305);

	FVector2D GridSize = GridSubsystem->GetGridSize();
	GridSize = FVector2D(FMath::Floor(GridSize.X), FMath::Floor(GridSize.Y));

	float OffsetX = (int(TileCount.X) % 2 == 0) ? 0.5f * GridSize.X : 0.0f;
	float OffsetY = (int(TileCount.Y) % 2 == 0) ? 0.5f * GridSize.Y : 0.0f;

	GridSubsystem->SetGridOffset(FVector2D(OffsetX, OffsetY));

	FVector2D GridOffset = GridSubsystem->GetGridOffset();
	FVector BottomLeftCorner = FVector(GridOffset.X, GridOffset.Y, 0) - FVector((TileCount.X / 2) * GridSize.X, (TileCount.Y / 2) * GridSize.Y, 0);

	if (!bIsHex)
	{
		for (int32 Row = 0; Row < TileCount.X; ++Row)
		{
			for (int32 Col = 0; Col < TileCount.Y; ++Col)
			{
				FVector Location = FVector(BottomLeftCorner.X + (Row * GridSize.X), BottomLeftCorner.Y + (Col * GridSize.Y), 0);
							
				if (bUseSurface)
				{
					Location = UGridUtils::GetSurfaceLocation(GetWorld(), Location, this, bSurfaceHit);

					if (bSurfaceHit)
					{
						TileLocations.Add(Location);
						continue;
					}
				}
				else 
				{
					TileLocations.Add(Location);
				}
				
			}
		}
	}

	else
	{
		TArray<FIntPoint> Indices;

		int FloorX = FMath::Floor(TileCount.X / 2);

		for (int32 Q = (TileCount.X % 2 == 0 ? -FloorX + 1 : -FloorX);
			Q <= FloorX; ++Q)
		{
			// Loop through the row (R) axis
			for (int32 R = -TileCount.Y; R < TileCount.Y; ++R)
			{
				// Only create valid doubled coordinates if (col + row) % 2 == 0
				if ((abs(Q) + abs(R)) % 2 == 0)
				{
					// Convert normal coordinates (Q, R) to doubled coordinates (doubledX, doubledY)
					int32 DoubledX = Q; // Double the column (Q) value
					int32 DoubledY = R; // Row (R) remains the same

					// Add index to your array/map
					FIntPoint HexIndex = FIntPoint(DoubledX, DoubledY);
					FVector Location = UGridUtils::GetLocationFromIndex(HexIndex, true);			

					if (bUseSurface)
					{
						Location = UGridUtils::GetSurfaceLocation(GetWorld(), Location, this, bSurfaceHit);

						if (bSurfaceHit)
						{
							TileLocations.Add(Location);
							continue;
						}
					}

					else
					{
						TileLocations.Add(Location);
					}
				}
			}
		}
	}

	return TileLocations;
}

void AGridManager::RecalculateTileLocations(EGridVisualContext Context)
{
	// Recalculates tile locations in existing grid data.
	// Useful for terrain modifications.
	
	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreDependenciesValid())
	{
		return;
	}

	Context = UGridUtils::ResolveGridVisualContext(Context);
	
	float ZScale = 1;
	bool bIsHex = GridSubsystem->GetIsHex();
	TMap<FIntPoint, FTileData> GridData = GridSubsystem->GetGridData();

	// Recompute colors from tables
	GridVisuals->PrecomputeColors();

	for (auto& Elem : GridData)
	{
		FTileData Tile = Elem.Value;
		FVector Location = Elem.Value.TileTransform.GetLocation();

		// Using custom line trace here to accommodate for 
		// offset calculations, when not required use UGridUtils::GetSurfaceLocation()	
		bool bOutHit;
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		FCollisionQueryParams QueryParams;

		bOutHit = GetWorld()->LineTraceSingleByChannel(HitResult, Location + FVector(0, 0, 99999),
			Location - FVector(0, 0, 99999), GridSubsystem->GridSurfaceChannel, CollisionParams);

		if (HitResult.bBlockingHit)
		{
			Location = HitResult.Location;

			// Scale each instance based on hit normal
			ZScale = UGridUtils::GetScaleBasedOnNormal(HitResult.ImpactNormal,
				TileMinScale, TileMaxScale);
		}

		// Offset location z of each instance based on Z Scale
		float HeightOffset = (ZScale / FMath::Clamp(ZScale, 2, 1)) * 100;
		Location -= FVector(0, 0, HeightOffset);
		
		Tile.TileTransform.SetLocation(Location);
		Tile.TileTransform.SetScale3D(FVector(1, 1, ZScale));
		GridSubsystem->SetTileData(Tile);
	}

	GridVisuals->DestroyVisuals(Context);
	GridVisuals->ProcessVisualsAsync(Context);
}

void AGridManager::ProcessTileBatch(bool bAutoMapTiles, bool bRandomObstacles,
	bool bRemap)
{
	// Processes the entire grid data in batches.

	FScopeLock Lock(&GridSubsystem->Mutex);

	const int32 BatchSize = 50; // Number of tiles to process per frame
	int32 ProcessedTiles = 0;
	bool bIsHex = GridSubsystem->GetIsHex();
	FVector2D GridSize = GridSubsystem->GetGridSize();

	if (!bPreviousBatchComplete)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Batch still waiting to be completed..."));
		return;
	}

	if (!TileTypeMapping) 
	{ 
		return;
	}

	// Copy a batch of locations to process
	TArray<FTransform> TileBatch;
	const int32 BatchCount = FMath::Min(BatchSize, PendingTileLocations.Num());
	TileBatch.Append(PendingTileLocations.GetData(), BatchCount);

	// Remove the processed locations from the pending list
	PendingTileLocations.RemoveAt(0, BatchCount);

	// Process the batch asynchronously
	Async(EAsyncExecution::ThreadPool, [this, TileBatch, bAutoMapTiles, 
		bRandomObstacles, GridSize, bIsHex, bRemap]()
		{
			TArray<FProcessedTileData> ProcessedData; // Custom struct for processed data
			FProcessedTileData TileData;
			FTileTypeMapping* FoundRow;
			static const FString ContextString(TEXT("Tile Mapping Context"));

			for (const FTransform& Transform : TileBatch)
			{
				FIntPoint Index = UGridUtils::GetIndexFromLocation(Transform.GetLocation(), GridSubsystem->GetIsHex());
				FVector Location = Transform.GetLocation();

				TileData.TileIndex = Index;
				TileData.Type = FName("Default");
				TileData.MovementCost = 0;
				TileData.Location = Location;
				TileData.ZScale = Transform.GetScale3D().Z;

				if (bRemap)
				{	
					FTileData* PreviousTile = RemappedGridData.Find(Index);

					if (PreviousTile)
					{
						float* Cost = TypeMovementCostMap.Find(PreviousTile->TileType);

						if (Cost)
						{
							TileData.Type = PreviousTile->TileType;
							TileData.MovementCost = *Cost;
						};

						ProcessedData.Add(TileData);
						continue;
					}
				}

				if (bAutoMapTiles)
				{	
					FHitResult HitResult;
					FCollisionQueryParams CollisionParams;
					bool bHit;

					// The below is an accurate shape based trace, a more accurate for AutoMapping but less 
					// accurate when it comes to tile offset calculations.
					if (bUseShapeTrace)
					{
						FCollisionShape Shape = bIsHex ? FCollisionShape::MakeSphere(GridSize.X * HexAutoMapExtent) :
							FCollisionShape::MakeBox(FVector3f((GridSize.X / 2) * SquareAutoMapExtent, 
								(GridSize.Y / 2) * SquareAutoMapExtent, GridSize.X));

						bHit = GetWorld()->SweepSingleByChannel(
							HitResult,
							Location + FVector(0, 0, 9999),
							Location - FVector(0, 0, 9999),
							FQuat::Identity,
							GridSubsystem->AutoMapChannel,
							Shape,
							CollisionParams
						);
					}

					// Other wise use regular line trace, auto mapping accuracy is decreased significantly
					// when not using blocking volumes. Offset accuracy is increased. 
					else
					{
						bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Location + FVector(0, 0, 9999),
							Location - FVector(0, 0, 9999), GridSubsystem->AutoMapChannel, CollisionParams);
					}

					if (HitResult.bBlockingHit)
					{
						for (FName TileType : TypesToAutoMap)
						{
							if (HitResult.GetActor()->ActorHasTag(TileType))
							{
								// Attempt to find the row with the given TileName
								FoundRow = TileTypeMapping->FindRow<FTileTypeMapping>(TileType, ContextString, true);

								if (FoundRow)
								{
									float* Cost = TypeMovementCostMap.Find(TileType);

									if (Cost)
									{
										TileData.Type = TileType;
										TileData.MovementCost = *Cost;
									};
								}
							}
						}
					}

					ProcessedData.Add(TileData);
				}

				else if (bRandomObstacles) 
				{
					if (FMath::RandRange(0.0, 1.0) < ObstacleSpawnRate)
					{
						// Attempt to find the row with the given TileName
						FoundRow = TileTypeMapping->FindRow<FTileTypeMapping>(FName("Obstacle"), ContextString, true);

						if (FoundRow)
						{
							float* Cost = TypeMovementCostMap.Find(FName("Obstacle"));

							if (Cost)
							{
								TileData.Type = FName("Obstacle");
								TileData.MovementCost = *Cost;
							};

							ProcessedData.Add(TileData);
						}
					}

					else
					{
						// Attempt to find the row with the given TileName
						FoundRow = TileTypeMapping->FindRow<FTileTypeMapping>(FName("Default"), ContextString, true);

						if (FoundRow)
						{
							float* Cost = TypeMovementCostMap.Find(FName("Default"));

							if (Cost)
							{
								TileData.Type = FName("Default");
								TileData.MovementCost = *Cost;
							};

							ProcessedData.Add(TileData);
						}
					}
					
				}

				
				else
				{
					// Attempt to find the row with the given TileName
					FoundRow = TileTypeMapping->FindRow<FTileTypeMapping>(FName("Default"), ContextString, true);

					if (FoundRow)
					{
						float* Cost = TypeMovementCostMap.Find(FName("Default"));

						if (Cost)
						{
							TileData.Type = FName("Default");
							TileData.MovementCost = *Cost;
						};

						ProcessedData.Add(TileData);
					}
				}
				
			}

			// Update visuals and grid data on the game thread
			AsyncTask(ENamedThreads::GameThread, [this, ProcessedData, 
				bAutoMapTiles, bRandomObstacles, bRemap]()
				{
					TArray<FIntPoint> Indices;

					for (const FProcessedTileData TileData : ProcessedData)
					{
						FTileData CurrentTileData(TileData.TileIndex, 0, 0, 0, 0, NULL, TArray<FIntPoint>(),
							FTransform(FRotator(0), TileData.Location, FVector(1, 1, TileData.ZScale)), 
							TileData.Type, TArray<FName>{ "None" }, TileData.bIsWalkable);
						
						GridSubsystem->SetTileData(CurrentTileData);

						if (GridSubsystem->GetIsRuntime())
						{
							GridVisuals->AddTileVisualRuntime(TileData.TileIndex);
						}
						else 
						{
#if WITH_EDITOR
							GridVisuals->AddTileVisualEditor(TileData.TileIndex);
#endif
						}
						
					}

					bPreviousBatchComplete = true;

					if (PendingTileLocations.Num() > 0)
					{
						ProcessTileBatch(bAutoMapTiles, bRandomObstacles, bRemap);
					}	
				});
		});
}
