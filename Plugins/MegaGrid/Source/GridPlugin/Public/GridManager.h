// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#include "GridUtils.h"
#include "GridPlugin/Visuals/GridVisuals.h"
#include "GridManager.generated.h"

class UGridSubsystem;

/// <summary>
/// Delegate invoked when the grid shape is modified.
/// It notifies relevant classes to update accordingly.
/// </summary>
DECLARE_MULTICAST_DELEGATE(FOnShapeChanged);

/// <summary>
/// Delegate invoked when a data table is assigned.
/// Notifies relevant systems to handle the new data table.
/// </summary>
DECLARE_MULTICAST_DELEGATE(FOnDataTableAssigned);

/// <summary>
/// Struct used for batch processing.
/// </summary>
struct FProcessedTileData
{
	FIntPoint TileIndex;
	FVector Location;
	FName Type;
	double MovementCost;
	bool bIsWalkable;
	float ZScale;
};

UCLASS()
class GRIDPLUGIN_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AGridManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndReason) override;
	virtual void OnConstruction(const FTransform& Transform) override; // For Getting Subsystem when compiling BP

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY()
	UGridSubsystem* GridSubsystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Dependencies", 
		meta = (RequiredAsset))
	AGridVisuals* GridVisuals = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Dependencies", 
		meta = (RequiredAsset), meta = (ToolTip = "DataTable containing Tile Name, Type, Color and Movement Costs."))
	UDataTable* TileTypeMapping;

	/// <summary>
	/// Types to be mapped during AutoMapObstacles()
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Dependencies", meta = (RequiredAsset),
		meta = (ToolTip = "Types to automatically map during grid generation."))
	TArray<FName> TypesToAutoMap;

	/// <summary>
	/// Multiplier for scaling the hexagonal trace size during AutoMapObstacles().
	/// Adjusts the size of the sphere trace, used only when bUseShapeTrace is enabled.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generation",
		meta = (ToolTip = "Multiplier used for Sphere Trace, only used if bUseShapeTrace is active."));
	float HexAutoMapExtent = 0.8;

	/// <summary>
	/// Multiplier for scaling the box trace size during AutoMapObstacles().
	/// Adjusts the size of the sphere trace, used only when bUseShapeTrace is enabled.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generation",
		meta = (ToolTip = "Multiplier used for Box Trace, only used if bUseShapeTrace is active."));
	float SquareAutoMapExtent = 0.8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generation",
		meta = (ToolTip = "Chances of obstacles spawning. [0,1]"));
	float ObstacleSpawnRate = 0.1;

	/// <summary>
	/// Determines whether shape tracing is used for automatically mapping tiles during grid generation.
	/// Enables more precise detection of tile boundaries and obstacles.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generation",
		meta = (ToolTip = "Use shaped trace for auto mapping tiles."))
	bool bUseShapeTrace = false;

	/// <summary>
	/// Movement Costs from Data Table are loaded
	/// here for faster retrieval. 
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid Generation",
		meta = (ToolTip = "Precomputed Type to MovementCost map."))
	TMap<FName, float> TypeMovementCostMap;


private:

	bool IsCoreDependenciesValid();

public:

	/// <summary>
	/// Stores grid data from the previous state, saved during the execution of 
	/// RemapExistingTiles(EGridVisualContext Context).
	/// Used to preserve tile information during remapping processes.
	/// </summary>
	TMap<FIntPoint, FTileData> RemappedGridData;

	/// <summary>
	/// Retrieves a copy of the current grid data from the subsystem.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Tile Data", 
		meta = (ToolTip = "Returns a copy of the grid data."))
	TMap<FIntPoint, FTileData> GetGridData();
	
	/// <summary>
	/// Retrieves a copy of the tile data associated with the tile index.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Returns a copy of the tile's data."))
	FTileData GetTileData(FIntPoint TileIndex);

	/// <summary>
	/// Gets all tiles of given type.
	/// </summary>
	/// <param name="Type"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Get all tiles of given type."))
	TArray<FIntPoint> GetAllTilesOfType(FName Type);

	/// <summary>
	/// Gets all tiles of given state.
	/// </summary>
	/// <param name="State"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Get all tiles of given state."))
	TArray<FIntPoint> GetAllTilesOfState(FName State);

	/// <summary>
	/// Replaces the entire grid data in the GridSubsystem with the provided data.
	/// <b>Warning:</b> This operation is not recommended for high-frequency calls. For frequent updates, consider using SetTileData instead.
	/// </summary>
	/// <param name="NewGridData"></param>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Overwrites existing grid data."))
	void SetGridData(TMap<FIntPoint, FTileData> NewGridData);

	/// <summary>
	/// Replaces the data of a specific tile in the grid with the provided tile data.
	/// <b>Recommendation:</b> This method is best used after modifying a copy of the tile data (retrieved via GetTileData()).
	/// For frequent tile modifications, consider using a pointer method such as AddStateToTile() for better performance and safety.
	/// </summary>
	/// <param name="NewTileData"></param>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Overwrites the tile data at the new data's index."))
	void SetTileData(FTileData NewTileData);

	/// <summary>
	/// Sets grid size.
	/// NOTE: Wipes existing tile visuals. Try remapping if you intend to use the existing data.
	/// </summary>
	/// <param name="GridSize"></param>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Set grid size."))
	void SetGridSize(FVector2D GridSize);

	/// <summary>
	/// Sets tile count.
	/// NOTE: Wipes existing tile visuals. Try remapping if you intend to use the existing data.
	/// </summary>
	/// <param name="TileCount"></param>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Set tile count."))
	void SetTileCount(FVector2D TileCount);

	/// <summary>
	/// Sets the channel to be used for grid surface operations, such as surface hits, across the plugin.
	/// </summary>
	/// <param name="Channel"></param>
	UFUNCTION(BlueprintCallable, Category = "Tile Data", 
		meta = (ToolTip = "Sets surface trace channel on GridSubsystem."))
	void SetSurfaceTraceChannel(ECollisionChannel Channel);

	/// <summary>
	/// Sets the channel to be used for auto mapping tiles, influencing how tiles are detected in the grid.
	/// </summary>
	/// <param name="Channel"></param>
	UFUNCTION(BlueprintCallable, Category = "Tile Data",
		meta = (ToolTip = "Sets auto map trace channel on GridSubsystem."))
	void SetAutoMapTraceChannel(ECollisionChannel Channel);

	// Line Setters (Visuals)

	UFUNCTION(BlueprintCallable, Category = "Grid Settings")
	void SetLineWidth(float Width);

	UFUNCTION(BlueprintCallable, Category = "Grid Settings")
	void SetLineColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Grid Settings")
	void SetLineOpacity(float Opacity);

	// Tile Setters (Visuals)

	UFUNCTION(BlueprintCallable, Category = "Grid Settings")
	void SetGridColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Grid Settings")
	void SetGridOpacity(float Opacity);

	UFUNCTION(BlueprintCallable, Category = "Grid Settings")
	void SetGridShape(EGridShape Shape);

	/// <summary>
	/// StartTile and EndTile are precomputed for Hex Grids to optimize bounds calculation.
	/// These values are crucial for determining the grid's boundaries and ensuring accurate tile indexing.
	/// </summary>
	FIntPoint StartTile, EndTile;

	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Updates the transform of the given TileIndex both visually and in data."))
	void UpdateTileTransform(EGridVisualContext Context, FIntPoint TileIndex, FTransform InTransform);
	
public:
	
	/// <summary>
	/// Invoked when shape is changed.
	/// </summary>
	FOnShapeChanged OnShapeChanged;

	UFUNCTION(BlueprintCallable, Category = "Grid Setters")
	void NotifyGridChanged();


public:

	/// <summary>
	/// Adds / updates tile visual.
	/// Call this after modifying tile data if visuals aren't automatically updated.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "Adds/updates the given tile's visual."))
	void AddTileVisualToGrid(FIntPoint TileIndex, EGridVisualContext Context);

	/// <summary>
	/// Adds a state to the tile.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="State"></param>
	/// <param name="bReloadTile"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "Adds the given state to the given tile. States are added uniquely. ScopeLock for async, see documentation."))
	void AddStateToTile(FIntPoint TileIndex, FName State, bool bReloadTile, bool bScopeLock);

	/// <summary>
	/// Removes the state from the tile.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="State"></param>
	/// <param name="bReloadTile"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles", 
		meta = (ToolTip = "Removes the given state from tile if it exists. ScopeLock for async, see documentation."))
	void RemoveStateFromTile(FIntPoint TileIndex, FName State, bool bReloadTile, bool bScopeLock);

	/// <summary>
	/// Sets the type for the tile, overriding the existing tile type.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="Type"></param>
	/// <param name="bReloadTile"></param>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "Sets the given type to the given tile. Types are overriden."))
	void SetTileType(FIntPoint TileIndex, FName Type, 
		EGridVisualContext Context, bool bReloadTile);

	/// <summary>
	/// Removes the type from the tile. Resets to Default.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="Type"></param>
	/// <param name="bReloadTile"></param>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "The given type is removed from the given tile. Resets type to Default."))
	void RemoveTileType(FIntPoint TileIndex, FName Type, 
		EGridVisualContext Context, bool bReloadTile);

	/// <summary>
	/// Clears all tiles of given state.
	/// </summary>
	/// <param name="State"></param>
	/// <param name="bReloadTile"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles", 
		meta = (ToolTip = "Clears all tiles in the grid of given state."))
	void ClearAllTilesOfState(FName State, bool bReloadTiles);

	/// <summary>
	/// Clears all states of the entire grid.
	/// </summary>
	/// <param name="bReloadTile"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles", 
		meta = (ToolTip = "Clears all tiles in the grid of all states."))
	void ClearAllStates(bool bReloadTiles);

	/// <summary>
	/// Clears all types of the entire grid. Resets to Default.
	/// </summary>
	/// <param name="bReloadTile"></param>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "Clears all tiles in the grid of all types. Resets to Default."))
	void ClearAllTypes(EGridVisualContext Context, bool bReloadTiles);

	/// <summary>
	/// Clears all tiles of the given type. Resets to Default.
	/// </summary>
	/// <param name="Type"></param>
	/// <param name="bReloadTile"></param>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "Clears all tiles in the grid of the given type. Resets to Default."))
	void ClearAllTilesOfType(FName Type, EGridVisualContext Context, bool bReloadTiles);

	/// <summary>
	/// Hides or unhides the given state.
	/// </summary>
	/// <param name="Visibility"></param>
	/// <param name="TileIndex"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "Hides/Unhides the given state."))
	void SetStateVisibility(bool Visibility, FName State);

	/// <summary>
	/// Hides or unhides the given type.
	/// </summary>
	/// <param name="Context"></param>
	/// <param name="Visibility"></param>
	/// <param name="TileType"></param>
	UFUNCTION(BlueprintCallable, Category = "Add and Remove Tiles",
		meta = (ToolTip = "Hides/Unhides the given type."))
	void SetTypeVisibility(EGridVisualContext Context, bool Visibility, FName TileType);

public:

	UWorld* World;

	FOnDataTableAssigned OnDataTableAssigned;

	/// <summary>
	/// Notifies the Editor and delegates when the TileMapping and StateMapping
	/// tables are assigned. This triggers updates to relevant systems or components
	/// that depend on these mappings for proper functionality.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Delegate to notify when the Data Tables are assigned."))
	void NotifyOnDataTableAssigned();

	/// <summary>
	/// Generates grid data. Creates both visuals and data.
	/// </summary>
	/// <param name="TileCount"></param>
	/// <param name="bAutoMapObstacles"></param>
	/// <param name="bRandomObstacles"></param>
	/// <param name="bRemap"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Generates grid data and visuals."))
	void GenerateGrid(FIntPoint TileCount, bool bAutoMapTiles,
		bool bRandomObstacles, bool bRemap);

	/// <summary>
	/// Minimum Z Scale for HISMC instances.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generation",
		meta = (ToolTip = "Min scale multiplier for tile instance."))
	float TileMinScale = 2;

	/// <summary>
	/// Maximum Z Scale for HISMC instances.
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Generation",
		meta = (ToolTip = "Max scale multiplier for tile instance."))
	float TileMaxScale = 8;

	/// <summary>
	/// Generates grid locations based on the provided parameters. This function does not create 
	/// or modify the grid data, making it suitable for procedural placement scenarios.
	/// </summary>
	/// <param name="TileCount"></param>
	/// <param name="bUseSurface"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Generates tile locations of the grid."))
	TArray<FVector> GenerateGridLocations(FIntPoint TileCount, bool bUseSurface);

	/// <summary>
	/// Recalculates tile locations in existing grid. Useful for terrain adjustments.
	/// </summary>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Recalculates all instances' locations from a pre-existing grid data. Useful for terrain changes."))
	void RecalculateTileLocations(EGridVisualContext Context);

	/// <summary>
	/// Destroys grid completely, including visuals and data.
	/// </summary>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Destroys grid data and visuals."))
	void DestroyGrid(EGridVisualContext Context);

	/// <summary>
	/// Reloads tiles with existing grid data.
	/// </summary>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Reloads the grid visuals."))
	void ForceReloadTiles(EGridVisualContext Context);

	/// <summary>
	/// Precomputes movement costs from the data table to a TMap.
	/// Only called during begin play, so if you update the table during runtime,
	/// make sure to call it in relevant places.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Pre-Loads movement costs for each tile into TypeMovementCostMap."))
	void PrecomputeMovementCosts();

	/// <summary>
	/// Gets movement cost from precomputed map. 
	/// WARNING: NOT USABLE FOR ASYNC/MULTITHREADING.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Gets movement cost from precomputed map. NOT USABLE FOR ASYNC/MULTITHREADING."))
	float GetMovementCost(FIntPoint TileIndex);

	/// <summary>
	/// Remaps previous grid data into newly created grid.
	/// NOTE: May result in some data loss.
	/// </summary>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Generation",
		meta = (ToolTip = "Remaps existing tile data into new grid. Useful after grid size or tile count changes."))
	void RemapExistingTiles(EGridVisualContext Context);

	TArray<FTransform> PendingTileLocations;
	bool bPreviousBatchComplete = true;
	FTimerHandle BatchTimerHandle;

	/// <summary>
	/// Used in conjunction with GenerateGrid(), processes tiles in batches
	/// to avoid crashes.
	/// </summary>
	/// <param name="bAutoMapTiles"></param>
	/// <param name="bRandomObstacles"></param>
	/// <param name="bRemap"></param>
	void ProcessTileBatch(bool bAutoMapTiles, bool bRandomObstacles, bool bRemap);
};
