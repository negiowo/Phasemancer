// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Async/Async.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ThreadObject.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

#include "GridDataStructures.h"
#include "GridUtils.h"
#include "Delegates/DelegateCombinations.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "PathFindingComponent.generated.h"

class UGridSubsystem;

/// <summary>
/// Pathfinding complete reason. Why did the pathfinding stop?
/// </summary>
UENUM(BlueprintType)
enum class EPathCompleteReason : uint8
{
	PathFound UMETA(DisplayName = "PathFound"),
	MaxIterationsReached  UMETA(DisplayName = "MaxIterationsReached"),
	InvalidStartOrEnd UMETA(DisplayName = "InvalidStartOrEnd"),
	PathNotFound UMETA(DisplayName = "PathNotFound"),
	GridNotGenerated UMETA(DisplayName = "GridNotGenerated"),
	TargetTooFar UMETA(DisplayName = "TargetTooFar")
};

/// <summary>
/// Struct that holds the returning path and the complete reason.
/// </summary>
USTRUCT(BlueprintType)
struct FPathStruct
{
	GENERATED_BODY()

	// Member variables
	UPROPERTY(BlueprintReadWrite, Category = "Path")
	TArray<FIntPoint> Path;

	UPROPERTY(BlueprintReadWrite, Category = "Path")
	EPathCompleteReason PathCompleteReason;

	// Constructor
	FPathStruct()
		: PathCompleteReason(EPathCompleteReason::InvalidStartOrEnd) // Default value
	{}

	FPathStruct(const TArray<FIntPoint>& InPath, EPathCompleteReason InReason)
		: Path(InPath), PathCompleteReason(InReason)
	{}
};

/// <summary>
/// Used to notify others when async pathfinding is complete.
/// </summary>
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPathCompleteDelegate, const FPathStruct&, PathOut);

/// <summary>
/// Called every iteration through StartMovingOnPath. If you're using you're own
/// movement functions, this is of little interest.
/// </summary>
/// 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPathFollowDelegate, FTransform, Transform);


UCLASS( ClassGroup=(TerrainGrid), meta=(BlueprintSpawnableComponent))
class GRIDPLUGIN_API UPathFindingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPathFindingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Data");
	bool bIsHex;

	/// <summary>
	/// Tells pathfinding how many iterations before its stops.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Data",
		meta = (ToolTip = "How many iterations before pathfinding stops. Higher values may be slower."));
	int32 MaxIterations = 25000; 
	
	/// <summary>
	/// Maximum length of a path. If exceeded, pathfinding is stopped.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Data",
		meta = (ToolTip = "Pathfinding will terminate if a path exceeds this length."));
	int32 MaxPathLength = 300;

	/// <summary>
	/// Whether to use heap optimization or not. Negligible performance difference.
	/// May improve accuracy.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Data",
		meta = (ToolTip = "Use heap optimization? Negligible performance difference. May improve accuracy."));
	bool bUseHeap;

private:

	/// <summary>
	/// Below are thread specific variables. Recommended to not touch.
	/// </summary>
	FRunnable* CurrentRunnable = nullptr;
	FRunnableThread* CurrentThread = nullptr;

	/// <summary>
	/// Core References.
	/// </summary>
	UGridUtils* GridUtils;
	UGridSubsystem* GridSubsystem;

public:
	
	/// <summary>
	/// Tells if the Grid exists and is generated.
	/// Used to prematurely cut off pathfinding.
	/// </summary>
	/// <returns></returns>
	bool IsGridGenerated();

public:

	/// <summary>
	/// Calculate distance between two points.
	/// Used as HCost in Pathfinding, otherwise known as Heuristic function. 
	/// </summary>
	/// <param name="CurrentIndex"></param>
	/// <param name="TargetIndex"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Pathfinding",
		meta = (ToolTip = "Get distance between two tiles. Heuristic or HCost"))
	float GetDistance(FIntPoint CurrentIndex, FIntPoint TargetIndex);

	/// <summary>
	/// Same as the prior function, except for Hexagons.
	/// </summary>
	/// <param name="CurrentIndex"></param>
	/// <param name="TargetIndex"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Pathfinding",
		meta = (ToolTip = "Get hex distance between two tiles. Heuristic or HCost"))
	float GetHexDistance(FIntPoint CurrentIndex, FIntPoint TargetIndex);


	/// <summary>
	/// Gets the lowest FCost in a given array.
	/// Used in Pathfinding to find the fittest node.
	/// </summary>
	/// <param name="OpenList"></param>
	/// <returns></returns>
	FTileData GetLowestFCostNode(TArray<FTileData>& OpenList) const;

	/// <summary>
	/// Generates path from start to end.
	/// </summary>
	/// <param name="Start"></param>
	/// <param name="Target"></param>
	/// <param name="IndexNodeMap"></param>
	/// <returns></returns>
	TArray<FIntPoint> GeneratePath(FIntPoint StartIndex, FIntPoint TargetIndex,
		TMap<FIntPoint, FTileData>& GridData);
	
	/// <summary>
	/// Checks if a tile is walkable.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="GridData"></param>
	/// <returns></returns>
	bool IsTileWalkable(FIntPoint TileIndex, TMap<FIntPoint, FTileData>& GridData);

	/// <summary>
	/// Checks if a tile is walkable. Needs a separate function
	/// for BP calls.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Pathfinding",
		meta = (ToolTip = "Tells if the given tile is walkable or not."))
	bool IsTileValid(FIntPoint TileIndex);

	/// <summary>
	/// Gets movement cost from precomputed cost map.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="GridData"></param>
	/// <returns></returns>
	float GetMovementCost(FIntPoint TileIndex);

	/// <summary>
	/// Gets precomputed neighbors for the given tile.
	/// NOTE: This is not used in the current implementation. If you want to use this, 
	/// precompute neighbors during grid generation.
	/// Not for async. Will crash if used.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <returns></returns>
	TArray<FIntPoint> GetPrecomputedNeighbors(FIntPoint TileIndex);


public:	

	/// <summary>
	/// Thread used for Async pathfinding.
	/// </summary>
	class FPathfindingThread* PathfindingThread;
	FRunnableThread* CurrentRunningThread;
	
	bool bIsTaskRunning;
	bool bStopPathFinding;
	void StopPathFindingThread();

	/// <summary>
	/// BP ready event call for when async pathfinding completes.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Pathfinding",
		meta = (ToolTip = "Called every time async pathfinding completes."))
	FPathCompleteDelegate OnPathComplete;

	/// <summary>
	/// BP ready event call for each iteration of StartMovingOnPath().
	/// NOTE: Because of the nature of this implementation, I recommend adapting the same 
	/// StartMovingOnPath() into a custom BP function for more flexibility.
	/// </summary>
	UPROPERTY(BlueprintAssignable, Category = "Path Movement", 
		meta = (ToolTip = "Called every time FTransform is calculated"))
	FPathFollowDelegate FollowPath;

	void NotifyPathfindingComplete(const FPathStruct& Path);
	void NotifyFollowPath(FTransform Transform);

	/// <summary>
	/// Last successful path.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding",
		meta = (ToolTip = "Last valid path"));
	TArray<FIntPoint> LastPath;

	/// <summary>
	/// All the types the pathfinding will treat as obstacles.
	/// Agent specific, so if you have 5 characters each with its own PathfindingComponent,
	/// then each can have its own TypesToBlock.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding",
		meta = (ToolTip = "Types to be treated as obstacles."))
	TArray<FName> TypesToBlock;

	/// <summary>
	/// All the types the pathfinding will treat as None, i.e, with 0 movement cost.
	/// Agent specific, so if you have 5 characters each with its own PathfindingComponent,
	/// then each can have its own TypesToIgnore.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding",
		meta = (ToolTip = "Types to be treated with no movement costs."))
	TArray<FName> TypesToIgnore;

	/// <summary>
	/// Same as the above, but for TileStates, imagine a temporary area selection
	/// where the agent cannot traverse.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding",
		meta = (ToolTip = "States to be treated as obstacles"))
	TArray<FName> StatesToBlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding",
		meta = (ToolTip = "DataTable containing Tile Name, Type, Color and Movement Costs."))
	UDataTable* TileTypeMapping;

	UPROPERTY(EditAnywhere, Category = "Advanced Pathfinding",
		meta = (ToolTip = "Use diagonals for Pathfinding?"))
	bool bUseDiagonals = false;
	
	/// <summary>
	/// Whether to tax diagonal movement. 
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Pathfinding",
		meta = (ToolTip = "Turning this ON will add DiagonalCost to diagonal movements."))
	bool bUseWeightedDiagonals = false;

	/// <summary>
	/// Tax levied on diagonal movement.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Pathfinding",
		meta = (ToolTip = "Increasing this will make diagonal movements more expensive."))
	float DiagonalCost = 1;

	/// <summary>
	/// Movement Costs from Data Table are loaded
	/// here for faster retrieval. 
	/// </summary>
	TMap<FName, float> TypeMovementCostMap;

	/// <summary>
	/// Precomputes movement costs from the data table to a TMap.
	/// Only called during begin play, so if you update the table during runtime,
	/// make sure to call it in relevant places.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Pathfinding", 
		meta = (ToolTip = "Pre-Loads movement costs for each tile into TypeMovementCostMap."))
	void PrecomputeMovementCosts();

	/// <summary>
	/// Whether to take movement (tile) costs into account when pathfinding.
	/// When OFF, pathfinding speed is improved dramatically at the cost of accuracy.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Pathfinding",
		meta = (ToolTip = "Tells the pathfinding to use MovementCosts"))
	bool bUseMovementCost = true;
	
	/// <summary>
	/// How much does the pathfinding lean towards using HCost (Distance)?
	/// Higher amounts may be faster but have less accuracy.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Pathfinding", 
		meta = (ToolTip = "How much bias does the pathfinder have towards Distance? Lower = Higher Accuracy (Slower). Higher = Lower Accuracy (Faster)."))
	float DistanceBias = 0;

	/// <summary>
	/// Dynamically calculated multiplier for DistanceBias. Bigger for 
	/// higher Tile Counts.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Pathfinding",
		meta = (ToolTip = "Multiplier for influencing Distance. Lower for TileCounts, Vice Versa"))
	float HeuristicSizeMultiplier = 500;

	/// <summary>
	/// How much does the pathfinding lean towards MovementCost (GCost)?
	/// Higher values ([0, 1]) are more accurate while taking a hit on performance.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Pathfinding",
		meta = (ToolTip = "How much bias does the pathfinder have towards MovementCost (GCost)? Lower = Higher Accuracy(Slower), Higher = Lower Accuracy(Faster)"))
	float MovementCostBias = 1;


public:

	/// <summary>
	/// Finds a path from start to end.
	/// Sync function, will slow down game for longer paths.
	/// </summary>
	/// <param name="StartIndex"></param>
	/// <param name="TargetIndex"></param>
	/// <returns> Path </returns>
	UFUNCTION(BlueprintCallable, Category = "Pathfinding",
		meta = (ToolTip = "Sync pathfinding, long paths may bog down FPS!"))
	FPathStruct FindPath(FIntPoint StartIndex, FIntPoint TargetIndex);

	/// <summary>
	/// Same as FindPath but runs on a separate thread. 
	/// Has no effect on performance even for large paths. 
	/// Value is returned to OnPathComplete Delegate.
	/// </summary>
	/// <param name="StartIndex"></param>
	/// <param name="TargetIndex"></param>
	UFUNCTION(BlueprintCallable, Category = "Pathfinding",
		meta = (ToolTip = "Async pathfinding, 60FPS guaranteed! (T&C applies, haha)"))
	void FindPathAsync(FIntPoint StartIndex, FIntPoint TargetIndex);

	/// <summary>
	/// These are only for stress testing.
	/// </summary>

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Only")
	int NumCalls = 10;

	UFUNCTION(BlueprintCallable, Category = "Test Only",
		meta = (ToolTip = "Development ONLY."))
	void FindPathAsyncConditioning(FIntPoint StartIndex, FIntPoint TargetIndex);

	/// <summary>
	/// Gets the total cost of traversal.
	/// Calculated using each tile's movement cost in the given path.
	/// </summary>
	/// <param name="Path"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Pathfinding",
		meta = (ToolTip = "Get the cost to traverse the given path."))
	float GetTotalMovementCost(TArray<FIntPoint> Path);

public:

	UPROPERTY(EditAnywhere, Category = "Spline Path")
	UMaterialInterface* SplineMeshMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path",
		meta = (ToolTip = "The end mesh of a spline, think arrow head of an arrow."))
	UStaticMesh* SplineEndMesh;

	UStaticMeshComponent* ArrowHeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path")
	UStaticMesh* SplineBodyMesh;

	TArray<USplineMeshComponent*> SplineMeshes;

	/// <summary>
	/// Reuses previously created spline meshes
	/// for performance reasons. Required count is the length of the new path.
	/// NOTE: Even with reuse optimizations spline generation is quite slow, so don't use for
	/// real-time pathfinding.
	/// </summary>
	/// <param name="PathSpline"> Your Spline Component </param>
	/// <param name="RequiredCount"> Length of the path </param>
	void ReuseSplineMeshes(USplineComponent* PathSpline, int32 RequiredCount);

	/// <summary>
	/// Deletes or hides objects associated with 
	/// spline path. Including the spline points and spline meshes.
	/// </summary>
	/// <param name="PathSpline"></param>
	/// <param name="bHideMesh"></param>
	UFUNCTION(BlueprintCallable, Category = "Spline Path", 
		meta = (ToolTip = "Clears the existing path of a given spline."))
	void ClearSplinePath(USplineComponent* PathSpline, bool bHideMesh);
	
	/// <summary>
	/// Creates a spline path with the
	/// provided spline component and Path. 
	/// </summary>
	/// <param name="PathSpline"></param>
	/// <param name="Path"></param>
	/// <param name="HeightOffset"</param>
	/// <param name="NumSamples"></param>
	UFUNCTION(BlueprintCallable, Category = "Path Movement",
		meta = (ToolTip = "Draws Spline Path. NOTE: Quite slow, avoid high frequency calls!"))
	void DrawSplinePath(USplineComponent* PathSpline, TArray<FVector> Path, float HeightOffset, int32 NumSamples);

	/// <summary>
	/// Timer to handle delays for StartMovingOnPath()
	/// </summary>
	FTimerHandle PathMovementTimerHandle;

	/// <summary>
	/// Moves along path.
	/// For modularity sake I'm returning each iterations' FTransform to OnFollowPath delegate.
	/// This way you can use the returning transform however you'd like. But I recommend implementing
	/// your own movement function in BP.
	/// </summary>
	/// <param name="Path"> Path to follow </param>
	/// <param name="Interval"> Interval of the each timer call </param>
	/// <param name="LerpSpeed"> Speed of Movement </param>
	/// <param name="Delay"> Delay between each Tile, increase for Chess like movement</param>
	UFUNCTION(BlueprintCallable, Category = "Path Movement", 
		meta = (ToolTip = "Starts calculating interval transforms. Returned in the OnFollowPath event."))
	void StartMovingOnPath(TArray<FVector> Path, double Interval, double LerpSpeed, double Delay);

public:

	/// <summary>
	/// Heap Helpers, I honestly don't know what's going on here. 
	/// </summary>
	/// <param name="Heap"></param>
	/// <param name="Index"></param>
	
	void HeapifyUp(TArray<FTileData>& Heap, int Index);
	void HeapifyDown(TArray<FTileData>& Heap, int Index);
	void AddToHeap(TArray<FTileData>& Heap, const FTileData& Node);
	FTileData PopFromHeap(TArray<FTileData>& Heap);

private:
	mutable FCriticalSection GridDataMutex; // Mutex for thread safety
};


/*
Async Pathfinding 60fps claim: Yes, in the abstract sense (in terms of calculating numbers and distances)
you will get 60fps, with no effect on performance. Even for insanely long paths, the async PF is very quick.

However, the moment you decide to add additional stuff using the returned path, like updating HISMc, you're
bound to take a hit without proper optimizations. Like even in this plugin, I recommend not using spline paths
for real-time pathfinding. It's not that the pathfinding is slow, its what comes after it. But in terms of 
HISMc, you'll see I have used chunk optimizations, so it's pretty quick with stable 60.
*/