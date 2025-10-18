// Copyright Two Bit Studios 2025. All Rights Reserved.

#include "PathFindingComponent.h"
#include "GridSubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

#include "TimerManager.h"

// Sets default values for this component's properties
UPathFindingComponent::UPathFindingComponent()
{
	
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

// Called when the game starts
void UPathFindingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create Utils object
	GridUtils = NewObject<UGridUtils>(this, UGridUtils::StaticClass());
	GridSubsystem = GridUtils->GetGridSubsystem();

	// Precomputes movement costs from data table to a TMap.
	// Only called during begin play, so if you update the table during runtime,
	// make sure to call it in relevant places. 
	PrecomputeMovementCosts();
}

void UPathFindingComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	bStopPathFinding = true; // Stop pathfinding if not already stopped.

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UPathFindingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UPathFindingComponent::IsGridGenerated()
{
	if (!GridSubsystem || !GridSubsystem->IsValidLowLevel())
	{
		return false;
	}

	FScopeLock Lock(&GridSubsystem->Mutex);

	int32 GridLength = GridSubsystem->GetGridData().Num();

	if (GridLength == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Grid is not generated. Please create grid!"));
		return false;
	}

	return true;
}

float UPathFindingComponent::GetDistance(FIntPoint Current, FIntPoint Target)
{
	float Distance;

	if (!GridSubsystem->GetIsHex())
	{
		int32 DeltaX = FMath::Abs(Current.X - Target.X);
		int32 DeltaY = FMath::Abs(Current.Y - Target.Y);
		
		if (!bUseDiagonals)
		{
			// Square Heuristic: Manhattan Distance
			Distance = DeltaX + DeltaY;
		}
		else 
		{
			if (bUseWeightedDiagonals)
			{
				// Square Diagonal Heuristic: Chebyshev Distance w Diagonal Weight
				Distance = (DeltaX + DeltaY) + (DiagonalCost - 2) * FMath::Min(DeltaX, DeltaY);
			}
			else 
			{
				// Square Diagonal Heuristic: Chebyshev Distance
				Distance = FMath::Max(DeltaX, DeltaY);
			}
		}
	}

	else
	{
		// Axial Distance for hexagon
		Distance = GetHexDistance(Current, Target);
	}

	return Distance;
}

float UPathFindingComponent::GetHexDistance(FIntPoint Current, FIntPoint Target)
{
	// Convert coordinates to axial for better accuracy and speed.

	Current = UGridUtils::DoubleToAxial(Current);
	Target = UGridUtils::DoubleToAxial(Target);

	return 1.1 * (FMath::Abs(Current.X - Target.X)
		+ FMath::Abs(Current.X + Current.Y - Target.X - Target.Y)
		+ FMath::Abs(Current.Y - Target.Y)) / 2;

}

FTileData UPathFindingComponent::GetLowestFCostNode(TArray<FTileData>& OpenList) const
{
	// Start by assuming the first element is the lowest
	FTileData LowestNode = OpenList[0];
	int32 LowestIndex = 0; // Keep track of the index for removing the node later

	for (int32 i = 1; i < OpenList.Num(); ++i)
	{
		const FTileData& Node = OpenList[i];

		// Compare cost and choose the one with the lowest FCost
		if (Node.FCost < LowestNode.FCost ||
			(Node.FCost == LowestNode.FCost && Node.HCost < LowestNode.HCost))
		{
			LowestNode = Node;
			LowestIndex = i;
		}
	}

	return LowestNode;

}

TArray<FIntPoint> UPathFindingComponent::GeneratePath(FIntPoint StartIndex, FIntPoint TargetIndex,
	TMap<FIntPoint, FTileData>& GridData)
{
	TArray<FIntPoint> TempPath;
	FIntPoint CurrentIndex = TargetIndex;

	if (!GridSubsystem)
	{
		return TArray<FIntPoint>();
	}

	check(GridSubsystem)
	check(&GridSubsystem->Mutex);
	FScopeLock Lock(&GridSubsystem->Mutex);

	while (CurrentIndex != StartIndex)
	{
		FTileData* CurrentNode = GridData.Find(CurrentIndex);

		if (!CurrentNode)
		{
			break;
		}

		TempPath.Add(CurrentIndex);

		// Move to the parent node
		CurrentIndex = CurrentNode->ParentIndex;
	}

	// Reverse the TempPath
	Algo::Reverse(TempPath);

	return TempPath;
}

bool UPathFindingComponent::IsTileValid(FIntPoint Index)
{
	// NOTE: Function may seem redundant but this is for BP exposure.

	TMap<FIntPoint, FTileData>& GridData = GridSubsystem->GetGridData();

	if (GridSubsystem)
	{
		return IsTileWalkable(Index, GridData);
	}
	
	return false;
}

bool UPathFindingComponent::IsTileWalkable(FIntPoint Index, 
	TMap<FIntPoint, FTileData>& GridData)
{
	// Doing this if the task is still running and cleanup begins.
	if (!GridSubsystem)
	{
		return false;
	}

	check(GridSubsystem)
	check(&GridSubsystem->Mutex);
	FScopeLock Lock(&GridSubsystem->Mutex);

	FTileData* TileData = GridData.Find(Index);

	if (TileData)
	{
		// Check if the index has any types we need to block.
		for (FName Type : TypesToBlock)
		{
			if (TileData->TileType == Type)
			{
				return false;
			}
		}

		// Check if the index has any states we need to block.
		for (FName State : StatesToBlock)
		{
			if (TileData->TileState.Contains(State))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

float UPathFindingComponent::GetMovementCost(FIntPoint TileIndex)
{	
	QUICK_SCOPE_CYCLE_COUNTER(STAT_Pathfinding_GetMovementCost);

	if (!GridSubsystem)
	{
		return 999;
	}

	check(GridSubsystem);
	check(&GridSubsystem->Mutex);
	FScopeLock Lock(&GridSubsystem->Mutex);

	if (!GridSubsystem)
	{
		return 0;
	}

	if (!GridSubsystem->GetGridData().Contains(TileIndex)) {
		return 999;
	}

	FTileData* Node = GridSubsystem->GetGridData().Find(TileIndex);
	FName TileType = Node->TileType;

	// If the tile contains any types to ignore, then return 0
	if (!TypesToIgnore.IsEmpty())
	{
		for (FName Type : TypesToIgnore)
		{
			if (TileType == Type)
			{
				return 0;
			}
		}
	}

	float* Cost = TypeMovementCostMap.Find(TileType);

	if (!Cost)
	{
		return 0;
	}

	return *Cost;
}

TArray<FIntPoint> UPathFindingComponent::GetPrecomputedNeighbors(FIntPoint Index)
{
	// Uses precomputed Neighbors for each tile.
	// NOTE: This pathfinding and grid implementation doesn't use precomputed neighbors.
	// If you want to save a very small amount of performance you can use this method but
	// make sure to precompute each neighbor in GridManager's ProcessTileBatch() method.

	TArray<FIntPoint> Neighbors;
	
	if (!GridSubsystem || !GridSubsystem->IsValidLowLevel())
	{
		return Neighbors;
	}

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (GridSubsystem->GetGridData().Contains(Index)) 
	{
		Neighbors = GridSubsystem->GetTileData(Index).Neighbors;
	}
	
	return Neighbors;
}

void UPathFindingComponent::StopPathFindingThread()
{
	if (PathfindingThread)
	{
		// Tell the thread to stop
		PathfindingThread->Stop();
		// Wait for the thread to finish
		if (CurrentRunningThread)
		{
			CurrentRunningThread->WaitForCompletion();
			delete CurrentRunningThread;
			CurrentRunningThread = nullptr;
		}

		// Clean up the runnable
		delete PathfindingThread;
		PathfindingThread = nullptr;
	}
}

void UPathFindingComponent::PrecomputeMovementCosts()
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

FPathStruct UPathFindingComponent::FindPath(FIntPoint StartIndex, FIntPoint TargetIndex)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_Pathfinding);

	TArray<FIntPoint> EmptyArray;

	if (!GridSubsystem || !GridSubsystem->IsValidLowLevel())
	{
		return FPathStruct(EmptyArray, EPathCompleteReason::GridNotGenerated);
	}

	FScopeLock Lock(&GridSubsystem->Mutex);

	// If there's no GridData, return
	if (!IsGridGenerated())
	{
		return FPathStruct(EmptyArray, EPathCompleteReason::GridNotGenerated);
	}

	// Reload movement costs if the table has changed.
	PrecomputeMovementCosts();

	TArray<FTileData> OpenArray;
	TSet<FIntPoint> ClosedArray;

	TMap<FIntPoint, FTileData>& GridData = GridSubsystem->GetGridData();

	float GridFactor = FMath::Clamp(GridData.Num() / HeuristicSizeMultiplier, 1.0f, 2.0f); // Adjust based on grid size.
	float HeuristicWeight = DistanceBias > 0 ? DistanceBias * GridFactor : 1; // Scale weight dynamically.

	LastPath.Empty();

	int CurrentIterations = 0;

	// If distance to target is too far, return
	if (GetDistance(StartIndex, TargetIndex) > MaxPathLength) 
	{
		return FPathStruct(EmptyArray, EPathCompleteReason::TargetTooFar);
	}

	bool StartValidity = IsTileWalkable(StartIndex, GridData);
	bool TargetValidity = IsTileWalkable(TargetIndex, GridData);

	// If either start or target is not walkable, return
	if (StartValidity == false || TargetValidity == false) 
	{
		return FPathStruct(EmptyArray, EPathCompleteReason::InvalidStartOrEnd);
	}

	// If there's no target, return
	if (StartIndex == TargetIndex)
	{
		return FPathStruct(EmptyArray, EPathCompleteReason::InvalidStartOrEnd);
	}

	float TotalMovementCost = 0;

	// Initialize start and target tiles
	FTileData StartNode = FTileData(StartIndex, 0, 0, 0, 0, NULL, TArray<FIntPoint>{},
		FTransform(), FName("Default"), TArray<FName>{ FName("Path") }, true);
	FTileData TargetNode = FTileData(TargetIndex, 0, 0, 0, 0, NULL, TArray<FIntPoint>{},
		FTransform(), FName("Default"), TArray<FName>{ FName("Path") }, true);

	if (bUseHeap)
	{
		AddToHeap(OpenArray, StartNode); // Improved accuracy.
	}
	else
	{
		OpenArray.Add(StartNode);
	}
	
	// Actual pathfinding loop
	while (OpenArray.Num() > 0 && CurrentIterations < MaxIterations && !bStopPathFinding) {

		QUICK_SCOPE_CYCLE_COUNTER(STAT_Pathfinding_WhileLoop);

		CurrentIterations++;
		FTileData CurrentNode;
		
		// Get the lowest FCost node from OpenArray, starts with just the start tile
		if (bUseHeap)
		{
			CurrentNode = PopFromHeap(OpenArray); // Improved accuracy.
		}

		else 
		{
			CurrentNode = GetLowestFCostNode(OpenArray);
		}
		

		// If the current node is the target, then path is successfully found!
		if (CurrentNode.Index == TargetNode.Index)
		{
			// Trace back path
			LastPath = GeneratePath(StartNode.Index, TargetNode.Index, GridData); // Save path
			return FPathStruct(LastPath, EPathCompleteReason::PathFound);
		}

		// If not, move the current node from open list to closed. To avoid redundancy.
		OpenArray.Remove(CurrentNode);
		ClosedArray.Add(CurrentNode.Index);

		// For each current node, we observe its neighbors
		for (FIntPoint NeighborIndex : UGridUtils::GetNeighboringIndices(CurrentNode.Index,
			bIsHex, bUseDiagonals))
		{
			// If we've already considered the neighbor, then continue
			if (ClosedArray.Contains(NeighborIndex) || !IsTileWalkable(NeighborIndex, GridData))
				continue;

			FTileData* CurrentNeighbor = GridData.Find(NeighborIndex);

			if (CurrentNeighbor)
			{
				// If neighbor is not walkable, continue
				if (!IsTileWalkable(NeighborIndex, GridData))
				{
					continue;
				}

				// Calculate GCost by getting it from DataTable, using preloaded movement costs for speed
				float NewGCost = GetMovementCost(NeighborIndex) * MovementCostBias;
				
				if (bUseMovementCost)
				{
					// Adding Current GCost to new GCost results in far more accurate paths, but is slow
					NewGCost += CurrentNode.GCost;
				}
				
				if (OpenArray.ContainsByPredicate([NeighborIndex](const FTileData& Node) 
					{ return Node.Index == NeighborIndex; }))
				{
					if (NewGCost < CurrentNeighbor->GCost)
					{
						CurrentNeighbor->GCost = NewGCost;
						CurrentNeighbor->ParentIndex = CurrentNode.Index;
						CurrentNeighbor->HCost = HeuristicWeight * GetDistance(NeighborIndex, TargetIndex);
						CurrentNeighbor->FCost = CurrentNeighbor->GCost + CurrentNeighbor->HCost;
						CurrentNeighbor->bIsWalkable = IsTileWalkable(NeighborIndex, GridData);
					}
				}
				else
				{
					CurrentNeighbor->GCost = NewGCost;
					CurrentNeighbor->ParentIndex = CurrentNode.Index;
					CurrentNeighbor->HCost = HeuristicWeight * GetDistance(NeighborIndex, TargetIndex);
					CurrentNeighbor->FCost = CurrentNeighbor->GCost + CurrentNeighbor->HCost;
					CurrentNeighbor->bIsWalkable = IsTileWalkable(NeighborIndex, GridData);

					if (bUseHeap)
					{
						AddToHeap(OpenArray, *CurrentNeighbor); // Improved accuracy.
					}
					else
					{
						OpenArray.Add(*CurrentNeighbor);
					}
				}
			}	
		}
	}

	if (CurrentIterations >= MaxIterations) 
	{	
		return FPathStruct(EmptyArray, EPathCompleteReason::MaxIterationsReached);
	}	

	return FPathStruct(EmptyArray, EPathCompleteReason::PathNotFound);
}

void UPathFindingComponent::FindPathAsync(FIntPoint StartIndex, FIntPoint TargetIndex)
{
	StopPathFindingThread();

	PathfindingThread = new FPathfindingThread(StartIndex, TargetIndex, this);
	PathfindingThread->Init();
	CurrentRunningThread = FRunnableThread::Create(PathfindingThread, TEXT("Pathfinding Runnable"));
}

void UPathFindingComponent::FindPathAsyncConditioning(FIntPoint StartIndex, FIntPoint TargetIndex)
{
	if (bIsTaskRunning)
	{
		return;
	}

	bIsTaskRunning = true;
	bStopPathFinding = true;

	for (int i = 0; i < NumCalls; ++i)
	{
		bStopPathFinding = false;

		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
			// Lambda wrapped in TUniqueFunction
			TUniqueFunction<void()>([this, StartIndex, TargetIndex]()
				{
					// Pathfinding logic goes here
					FPathStruct Result = FindPath(StartIndex, TargetIndex);

					// Once pathfinding completes, update the game thread
					AsyncTask(ENamedThreads::GameThread, [this, Result]()
						{
							bIsTaskRunning = false;
							if (!bStopPathFinding)
							{
								OnPathComplete.Broadcast(Result);
							}
						});
				}),
			// StatId can be left as default or use a custom one
			TStatId(),
			// No prerequisites
			nullptr,
			// Desired thread type (Background task thread)
			ENamedThreads::AnyBackgroundThreadNormalTask
		);
	}

}

float UPathFindingComponent::GetTotalMovementCost(TArray<FIntPoint> Path)
{
	float TotalMovementCost = 0;

	if (!GridSubsystem)
	{
		return TotalMovementCost;
	}

	FScopeLock Lock(&GridSubsystem->Mutex);

	// Go through and add each tile's movement cost.
	for (auto& Index : Path)
	{
		FTileData TileData = GridSubsystem->GetTileData(Index);
		TotalMovementCost += GetMovementCost(Index);
	}
	
	return TotalMovementCost;
}

void UPathFindingComponent::ReuseSplineMeshes(USplineComponent* PathSpline, int32 RequiredCount)
{

	// If the spline meshes available is less than needed
	while (SplineMeshes.Num() < RequiredCount)
	{
		// Create a new spline mesh component.
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->AttachToComponent(PathSpline, FAttachmentTransformRules::KeepRelativeTransform);
		SplineMeshComponent->RegisterComponent();

		if (SplineBodyMesh && SplineMeshMaterial)
		{
			SplineMeshComponent->SetStaticMesh(SplineBodyMesh);
			SplineMeshComponent->SetMaterial(0, SplineMeshMaterial);
		}

		SplineMeshes.Add(SplineMeshComponent);
	}

	// If it's more, hide the unused ones.
	for (int32 i = RequiredCount; i < SplineMeshes.Num(); ++i)
	{
		SplineMeshes[i]->SetVisibility(false);
	}
}

void UPathFindingComponent::ClearSplinePath(USplineComponent* PathSpline, bool bHideMesh)
{
	if (PathSpline)
	{
		PathSpline->ClearSplinePoints(); // Clear spline points first.
	}

	// Destroy or hide spline meshes.
	for (auto& Mesh : SplineMeshes)
	{
		if (!bHideMesh)
		{
			Mesh->DestroyComponent();
		}
		
		else 
		{
			Mesh->SetVisibility(false, false);
		}
	}

	if (ArrowHeadMesh)
	{
		ArrowHeadMesh->SetVisibility(false, false);
	}

	if (bHideMesh)
	{
		SplineMeshes.Empty();
	}
}

void UPathFindingComponent::StartMovingOnPath(TArray<FVector> Path, double Interval, double LerpSpeed, double Delay)
{
	if (!Path.IsEmpty())
	{
		FVector PreviousLocation = Path[0];
		FVector NextLocation = Path[0];

		FRotator PreviousRotation;
		FRotator NextRotation;

		float ElapsedTime = 0.0f; // Tracks time for interpolation
		float LerpDuration = LerpSpeed;
		bool bDelayBetweenTiles = true; // Enable or disable delay option
		float DelayDuration = Delay;    // Delay between each tile movement
		bool bIsDelaying = false;       // Tracks whether we're currently in a delay state

		GetWorld()->GetTimerManager().SetTimer(PathMovementTimerHandle, [this, Path, 
			PreviousLocation, NextLocation, LerpDuration, 
			ElapsedTime, bDelayBetweenTiles, 
			DelayDuration, bIsDelaying, PreviousRotation, NextRotation]() mutable
			{
				if (Path.Num() > 0)
				{
					if (bIsDelaying)
					{
						// Reduce delay time
						ElapsedTime += GetWorld()->GetDeltaSeconds();
						if (ElapsedTime >= DelayDuration)
						{
							bIsDelaying = false; // End delay
							ElapsedTime = 0.0f; // Reset for the next movement
						}
						return; // Skip movement while delaying
					}

					// Increment elapsed time
					ElapsedTime += GetWorld()->GetDeltaSeconds();

					// Calculate Alpha (normalized time between 0 and 1)
					float Alpha = FMath::Clamp(ElapsedTime / LerpDuration, 0.0f, 1.0f);

					// Interpolate between PreviousLocation and NextLocation
					FVector CurrentLocation = FMath::Lerp(PreviousLocation, NextLocation, Alpha);
					FRotator CurrentRotation = FMath::Lerp(PreviousRotation, NextRotation, Alpha);

					FTransform MoveTransform(CurrentRotation, CurrentLocation, FVector(1));
					
					// Notify about movement
					NotifyFollowPath(MoveTransform);

					// Check if the lerp is complete
					if (Alpha >= 1.0f)
					{
						// Snap to the target location
						CurrentLocation = NextLocation;
						CurrentRotation = NextRotation;

						// Move to the next segment
						PreviousLocation = NextLocation;
						PreviousRotation = NextRotation;

						Path.RemoveAt(0);

						if (Path.Num() > 0)
						{
							NextLocation = Path[0];

							FVector Direction = (NextLocation - PreviousLocation).GetSafeNormal();
							NextRotation = Direction.Rotation();

							ElapsedTime = 0.0f; // Reset time for the next segment

							// Trigger delay if enabled
							if (bDelayBetweenTiles)
							{
								bIsDelaying = true;
							}
						}
						else
						{
							// Path is complete, stop the timer
							GetWorld()->GetTimerManager().ClearTimer(PathMovementTimerHandle);
						}
					}
				}
				else
				{
					// Path is empty, stop the timer
					GetWorld()->GetTimerManager().ClearTimer(PathMovementTimerHandle);
				}
			}, Interval, true);
	}
}

void UPathFindingComponent::DrawSplinePath(USplineComponent* PathSpline, TArray<FVector> Path, 
	float HeightOffset, int32 NumSamples)
{
	// At least two points are needed to draw a spline
	if (Path.Num() < 2 || !PathSpline || !GridSubsystem) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Path has fewer than 2 points; cannot draw spline."));
		return;
	}

	// Clear existing spline points
	PathSpline->ClearSplinePoints();

	// Add new spline points
	for (int i = 0; i < Path.Num() - 1; ++i) // Iterate through pairs of points
	{
		FVector Start = Path[i];
		FVector End = Path[i + 1];

		// Add the first point of the pair
		PathSpline->AddSplinePoint(Start + FVector(0, 0, HeightOffset), ESplineCoordinateSpace::World, true);

		for (int32 j = 1; j < NumSamples; ++j)
		{
			float Alpha = (float)j / (float)NumSamples;
			FVector InterpolatedPoint = FMath::Lerp(Start, End, Alpha);

			// Perform a line trace downward to find the terrain surface
			
			// Adjust height for trace start, lower values may be quicker
			FHitResult HitResult;
			FVector TraceStart = InterpolatedPoint + FVector(0, 0, 9999); 
			FVector TraceEnd = InterpolatedPoint - FVector(0, 0, 9999);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, GridSubsystem->GridSurfaceChannel))
			{
				FVector AdjustedPoint = HitResult.Location + FVector(0, 0, HeightOffset);
				PathSpline->AddSplinePoint(AdjustedPoint, ESplineCoordinateSpace::World, true);
			}
		}
	}

	// Add the last point of the path
	FVector LastPoint = Path.Last();
	PathSpline->AddSplinePoint(LastPoint + FVector(0, 0, HeightOffset), ESplineCoordinateSpace::World, true);

	// Set all spline points to curve type
	for (int32 i = 0; i < PathSpline->GetNumberOfSplinePoints(); ++i)
	{
		PathSpline->SetSplinePointType(i, ESplinePointType::Curve, true);
	}

	PathSpline->UpdateSpline();

	ReuseSplineMeshes(PathSpline, PathSpline->GetNumberOfSplineSegments());

	for (int SegmentIndex = 0; SegmentIndex < PathSpline->GetNumberOfSplineSegments(); ++SegmentIndex)
	{
		USplineMeshComponent* SplineMeshComponent = SplineMeshes[SegmentIndex];
		SplineMeshComponent->SetVisibility(true);

		// Configure mesh, tangents, and attach points
		FVector StartLoc, StartTangent, EndLoc, EndTangent;
		PathSpline->GetLocationAndTangentAtSplinePoint(SegmentIndex, StartLoc, StartTangent, ESplineCoordinateSpace::Local);
		PathSpline->GetLocationAndTangentAtSplinePoint(SegmentIndex + 1, EndLoc, EndTangent, ESplineCoordinateSpace::Local);
		SplineMeshComponent->SetStartAndEnd(StartLoc, StartTangent, EndLoc, EndTangent, true);
	}

	if (ArrowHeadMesh)
	{
		ArrowHeadMesh->DestroyComponent();
	}

	if (SplineEndMesh)
	{
		// Create a static mesh component for the cone
		ArrowHeadMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());

		// Set the static mesh to the cone (make sure you have a cone mesh asset assigned in the editor)
		ArrowHeadMesh->SetStaticMesh(SplineEndMesh);
		ArrowHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Get the last point of the spline to position the cone
		FVector LastLocation = PathSpline->GetLocationAtSplinePoint(PathSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
		FVector LastTangent;
		PathSpline->GetLocationAndTangentAtSplinePoint(PathSpline->GetNumberOfSplinePoints() - 1, LastLocation, LastTangent, ESplineCoordinateSpace::World);

		// Position and rotate the cone to face the direction of the path
		ArrowHeadMesh->SetWorldLocation(LastLocation);
		ArrowHeadMesh->SetWorldRotation(LastTangent.Rotation());

		// Attach the cone mesh to the spline component (or any other appropriate parent component)
		ArrowHeadMesh->AttachToComponent(PathSpline, FAttachmentTransformRules::KeepWorldTransform);

		// Register the cone component so it appears in the scene
		ArrowHeadMesh->RegisterComponent();
	}
}

void UPathFindingComponent::NotifyPathfindingComplete(const FPathStruct& Path)
{
	// Notifies the bound delegate to trigger event.
	if (OnPathComplete.IsBound()) {
		OnPathComplete.Broadcast(Path);
	}
}

void UPathFindingComponent::NotifyFollowPath(FTransform Transform)
{
	// Notifies bound delegate to trigger event
	if (FollowPath.IsBound())
	{
		FollowPath.Broadcast(Transform);
	}
}

void UPathFindingComponent::HeapifyUp(TArray<FTileData>& Heap, int Index)
{
	if (Index == 0) return; // Root node

	int32 ParentIndex = (Index - 1) / 2;

	if (Heap[Index].FCost < Heap[ParentIndex].FCost ||
		(Heap[Index].FCost == Heap[ParentIndex].FCost &&
			Heap[Index].HCost < Heap[ParentIndex].HCost)) // Tie-breaking
	{
		Swap(Heap[Index], Heap[ParentIndex]);
		HeapifyUp(Heap, ParentIndex);
	}
}

void UPathFindingComponent::HeapifyDown(TArray<FTileData>& Heap, int Index)
{
	int32 LeftChild = 2 * Index + 1;
	int32 RightChild = 2 * Index + 2;
	int32 Smallest = Index;

	// Compare with left child
	if (LeftChild < Heap.Num() && Heap[LeftChild].FCost < Heap[Smallest].FCost)
	{
		Smallest = LeftChild;
	}
	else if (LeftChild < Heap.Num() &&
		Heap[LeftChild].FCost == Heap[Smallest].FCost &&
		Heap[LeftChild].HCost < Heap[Smallest].HCost) // Tie-breaking
	{
		Smallest = LeftChild;
	}

	// Compare with right child
	if (RightChild < Heap.Num() && Heap[RightChild].FCost < Heap[Smallest].FCost)
	{
		Smallest = RightChild;
	}
	else if (RightChild < Heap.Num() &&
		Heap[RightChild].FCost == Heap[Smallest].FCost &&
		Heap[RightChild].HCost < Heap[Smallest].HCost) // Tie-breaking
	{
		Smallest = RightChild;
	}

	// If the smallest is not the current index, swap and recurse
	if (Smallest != Index)
	{
		Swap(Heap[Index], Heap[Smallest]);
		HeapifyDown(Heap, Smallest);
	}
}

void UPathFindingComponent::AddToHeap(TArray<FTileData>& Heap, const FTileData& Node)
{
	if (bStopPathFinding)
	{
		return;
	}

	FScopeLock HeapLock(&GridSubsystem->HeapMutex);

	Heap.Add(Node);
	HeapifyUp(Heap, Heap.Num() - 1);
}

FTileData UPathFindingComponent::PopFromHeap(TArray<FTileData>& Heap)
{
	if (bStopPathFinding)
	{
		return FTileData();
	}

	FScopeLock HeapLock(&GridSubsystem->HeapMutex);

	if (Heap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Heap is empty"));
		return FTileData();
	}

	FTileData RootNode = Heap[0];
	Heap[0] = Heap.Last();
	Heap.Pop();
	HeapifyDown(Heap, 0);

	return RootNode;
}