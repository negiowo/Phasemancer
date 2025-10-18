// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "GridPlugin/Data/GridDataStructures.h"

#include "GridVisuals.generated.h"

class AGridManager;
class UGridSubsystem;

/// <summary>
/// Shape enum.
/// </summary>
UENUM(BlueprintType)
enum class EGridShape : uint8
{
	Hex  UMETA(DisplayName = "Hex"),
	Square UMETA(DisplayName = "Square")
};

UCLASS()
class GRIDPLUGIN_API AGridVisuals : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridVisuals();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void PostLoad() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UGridSubsystem* GridSubsystem;
	UWorld* World;

	bool bIsPlaced = false;
	bool IsCoreValid();

public:
	
	/// <summary>
	/// Default ZeroAliasingGrid paths.
	/// </summary>
	FString SquareGridMaterialPath = "MaterialInstanceConstant'/MegaGrid/ZeroAliasingGrid/Square/WorldPosition/Single/MI_ZGPostProcess.MI_ZGPostProcess'";
	FString HexGridMaterialPath = "MaterialInstanceConstant'/MegaGrid/ZeroAliasingGrid/Hex/WorldPosition/Single/MI_ZGHexPost.MI_ZGHexPost'";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Dependencies", 
		meta = (RequiredAsset))
	UDataTable* TileTypeMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Dependencies", meta = (RequiredAsset))
	UDataTable* TileStateMapping;

	/// <summary>
	/// Post-Process component that handles grid material.
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPostProcessComponent* GridPostProcess = nullptr;

	/// <summary>
	/// Preloads colors from TileStateMapping and TileTypeMapping for faster lookups.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals")
	void PrecomputeColors();

	bool bIsInitialized;

	/// <summary>
	/// Scale multipliers for Highlight Materials.
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid Material", 
		meta = (ToolTip = "Scale multiplier for square highlight material"))
	float SQHighlightScaleMultiplier = 1.02;

	UPROPERTY(EditAnywhere, Category = "Grid Material",
		meta = (ToolTip = "Scale multiplier for hex highlight material"))
	float HexHighlightScaleMultiplier = 1.08;

public:
	
	/// <summary>
	/// Whether to use a custom grid material. Must be Post-Process.
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid Material",
		meta = (ToolTip = "Whether to use a custom grid material. Assign to HexGridMaterial or SquareGridMaterial."))
	bool bCustomGridMaterial;

	/// <summary>
	/// Whether to use a custom highlight material. 
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid Material",
		meta = (ToolTip = "Whether to use a custom highlight material. Assign to HexHighlightMaterial or SquareHighlightMaterial."))
	bool bCustomHighlightMaterial;

	/// <summary>
	/// Parent materials used to create grid DMIs. Can be Material Instances.
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid Material",
		meta = (ToolTip = "Parent hex grid material."))
	UMaterialInterface* HexGridMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Grid Material",
		meta = (ToolTip = "Parent square grid material."))
	UMaterialInterface* SquareGridMaterial = nullptr;

	/// <summary>
	/// Dynamically created material instances for grid Post-Process.
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = "Grid Material")
	UMaterialInstanceDynamic* SquareDMI;

	UPROPERTY(VisibleAnywhere, Category = "Grid Material")
	UMaterialInstanceDynamic* HexDMI;
	
	/// <summary>
	/// Default paths to highlight materials.
	/// </summary>
	FString HighlightPathHex = "MaterialInstanceConstant'/MegaGrid/Materials/MI_HighlightHex.MI_HighlightHex'";
	FString HighlightPathSq = "MaterialInstanceConstant'/MegaGrid/Materials/MI_HighlightSq.MI_HighlightSq'";

	/// <summary>
	/// Parent materials used to create highlight DMIs. Can be Material Instances.
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid Material")
	UMaterialInterface* HexHighlightMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Grid Material")
	UMaterialInterface* SquareHighlightMaterial = nullptr;

	/// <summary>
	/// Dynamically created material instances for tile highlights or tile colors.
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Material")
	UMaterialInstanceDynamic* SquareHighlightDMI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Material")
	UMaterialInstanceDynamic* HexHighlightDMI;

	/// <summary>
	/// Prepares all dynamic materials.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Grid Material",
		meta = (ToolTip = "Prepares all the dynamic grid materials."))
	void SetupDynamicMaterials();
	
	/// <summary>
	/// Changes the shape of both grid and highlight materials.
	/// </summary>
	/// <param name="GridShape"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Material",
		meta = (ToolTip = "Switches DMIs to use the given grid shape."))
	void SwitchGridShape(EGridShape GridShape);

	/// <summary>
	/// Loads all visuals required for grid.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Loads all visuals from existing grid data."))
	void LoadAllVisuals(EGridVisualContext Context);

	/// <summary>
	/// Hides/unhides the HISMCs of the given context.
	/// </summary>
	/// <param name="Context"></param>
	/// <param name="Visibility"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Hide/Unhide the visuals of the given context."))
	void SetVisualsVisibility(EGridVisualContext Context, bool Visibility);

	TMap<FIntPoint, FTileInstanceInfo> TileToInstanceMapRuntime = TMap<FIntPoint, FTileInstanceInfo>();
	TMap<FIntPoint, UHierarchicalInstancedStaticMeshComponent*> ChunkToISMCMapRuntime;
	
	/// <summary>
	/// Maps a tile index to its corresponding HISMC and instance index (FTileInstanceInfo).
	/// UPROPERTY() ensures compatibility and safety when switching between editor and runtime visuals.
	/// </summary>
	UPROPERTY()
	TMap<FIntPoint, FTileInstanceInfo> TileToInstanceMapEditor = TMap<FIntPoint, FTileInstanceInfo>();

	/// <summary>
	/// Maps a chunk index to its associated HISMC (e.g., multiple chunks like 1,2 and 4,4 may share the same HISMC).
	/// UPROPERTY() ensures compatibility and safety when switching between editor and runtime visuals.
	/// </summary>
	UPROPERTY()
	TMap<FIntPoint, UHierarchicalInstancedStaticMeshComponent*> ChunkToISMCMapEditor;

	/// <summary>
	/// Mesh used for tiles in the HISMCs
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid",
		meta = (ToolTip = "Mesh used for HISMC (Tiles)."))
	UStaticMesh* TileMesh;

	/// <summary>
	/// Defines the number of tiles grouped into a single HISMC. For example, a value of 50 will group tiles 
	/// from (0,0) to (49,49) into one chunk (i.e., 50 x 50). Actual grouping may vary slightly.
	/// Higher values reduce the number of HISMCs, resulting in fewer components but increased instance update times.
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid",
		meta = (ToolTip = "How many tiles belong to one HISMC? (e.g. 50 = 50x50 tiles per HISM)."))
	int32 ChunkSize = 50;
	
	/// <summary>
	/// Scaling factor for square tile's instance transform.
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Grid",
		meta = (ToolTip = "Scaling factor for square tile instance (HISMC)."))
	FVector SquareTileScaleFactor = FVector(200, 200, 2);

	/// <summary>
	/// Types and states to exclude from GetColorFromState & GetColorFromType calls.
	/// Returns transparent color.
	/// </summary>
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid",
		meta = (ToolTip = "Types that are hidden will be temporarily stored here."))
	TArray<FName> TypesToNotRender;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid",
		meta = (ToolTip = "States that are hidden will be temporarily stored here."))
	TArray<FName> StateToNotRender;

	/// <summary>
	/// Dynamically get/create a HISMC for the given ChunkIndex.
	/// </summary>
	/// <param name="ChunkPosition"></param>
	/// <param name="Context"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals", 
		meta = (ToolTip = "Dynamically create or get an HISMC for the given ChunkIndex."))
	UHierarchicalInstancedStaticMeshComponent* GetOrCreateISMCForChunk(FIntPoint ChunkIndex,
		EGridVisualContext Context);
	
	/// <summary>
	/// Sets culling distance for the HISMCs of the given context. 
	/// Good for reducing overdraw at eye level perspective.
	/// </summary>
	/// <param name="Context"></param>
	/// <param name="StartCullDistance"></param>
	/// <param name="EndCullDistance"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals", 
		meta = (ToolTip = "Dynamically set distance culling for the HISMC of the given context."))
	void SetInstanceCulling(EGridVisualContext Context, float StartCullDistance,
		float EndCullDistance);

	/// <summary>
	/// Get TileInstanceInfo for a given tile index.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="OutInfo"></param>
	/// <param name="InTileToInstanceMap"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Gets the tile instance info for the given tile index."))
	bool GetInstanceInfoForTile(FIntPoint TileIndex, FTileInstanceInfo& OutInfo,
		TMap<FIntPoint, FTileInstanceInfo>& InTileToInstanceMap);
	
	/// <summary>
	/// Procedurally create a Chunk Index.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Procedurally calculate which ChunkIndex the given tile index belongs to."))
	FIntPoint GetChunkFromTileIndex(const FIntPoint& TileIndex);

#if WITH_EDITOR
	/// <summary>
	/// Create or update an HISMC instance for the given tile index.
	/// For Editor
	/// </summary>
	/// <param name="Index"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Adds/Updates the HISMC instance corresponding to the tile index. Editor USE ONLY."))
	void AddTileVisualEditor(const FIntPoint& TileIndex);
#endif
	/// <summary>
	/// Create or Update an HISMC instance for the given tile index.
	/// For Runtime
	/// </summary>
	/// <param name="Index"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Adds/Updates the HISMC instance corresponding to the tile index."))
	void AddTileVisualRuntime(const FIntPoint& TileIndex);

	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Updates the HISM instance transform of the given TileIndex."))
	void UpdateVisualTransform(const FIntPoint& TileIndex, FTransform InTransform, EGridVisualContext Context);

	/// <summary>
	/// Destroys all HISMCs and instances of the given context.
	/// Empties the relevant maps as well.
	/// </summary>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals", 
		meta = (ToolTip = "Destroys all the HISMCs of the given context, resulting in an empty grid."))
	void DestroyVisuals(EGridVisualContext Context);

	/// <summary>
	/// Starts visual processing of the grid.
	/// Tiles are updated or created depending on use case.
	/// </summary>
	/// <param name="Context"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Process visuals for the entire grid data asynchronously."))
	void ProcessVisualsAsync(EGridVisualContext Context);

public:

	/// <summary>
	/// Returns the color corresponding to the given tile type
	/// </summary>
	/// <param name="TileType"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Returns the type color."))
	FVector4 GetColorFromType(FName TileType);

	/// <summary>
	/// Returns the color from either the tile state or type.
	/// States take priority, if none is found, type color is returned.
	/// </summary>
	/// <param name="TileData"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Visuals",
		meta = (ToolTip = "Returns color best suited for the tile. States take priority over Types."))
	FVector4 GetColorFromState(FTileData TileData);
	
	/// <summary>
	/// Arrange this array in order of state priority. States
	/// at the start of the array gets priority.
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Tiles",
		meta = (ToolTip = "Priority at which tile states are considered. First item gets highest priority."))
	TArray<FName> StatePriority { "Hovered",
		"Path", "Selected", "None" };

	/// <summary>
	/// Colors from data tables are preloaded into maps for faster lookup.
	/// </summary>
	TMap<FName, FVector4> TypeColorMap;
	TMap<FName, FVector4> StateColorMap;


public:
	/// <summary>
	/// Parameter setters for grid material.
	/// </summary>
	void SetGridSize(FVector2D GridSize);
	void SetTileCount(FVector2D TileCount);
	void SetGridOffset(float Count, FVector2D TileCount, bool bIsX);
	void SetLineWidth(float Width);
	void SetColor(FLinearColor Color, bool bIsLine);
	void SetOpacity(float Value, bool bIsLine);
};
