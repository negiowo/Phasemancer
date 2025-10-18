// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GridDataStructures.generated.h"

/// <summary>
/// Context specifier for grid visuals.
/// Editor and runtime visuals are separated to maintain
/// integrity.
/// </summary>
UENUM(BlueprintType, 
    meta = (ToolTip = "Context specifier for grid visuals."))
enum class EGridVisualContext : uint8
{   
    Auto UMETA(DisplayName = "Auto"), // Automatically resolved using ResolveGridVisualContext(Context)
    Editor  UMETA(DisplayName = "Editor"), // Editor use
    Runtime UMETA(DisplayName = "Runtime") // Runtime use, on or after BeginPlay()
};

/// <summary>
/// Struct that hold visuals related data for each tile.
/// Contains the HISMC the tile belongs to and it's instance index.
/// </summary>
USTRUCT(BlueprintType,
    meta = (ToolTip = "Struct that hold visuals related data for each tile."))
struct FTileInstanceInfo
{
    GENERATED_BODY()
    
    UHierarchicalInstancedStaticMeshComponent* HISMC; // HierarchicalInstancedStaticMeshComponent the tile belongs to.
   
    int32 InstanceIndex; // The instance index within the HISMC
   
    FTileInstanceInfo()
        : HISMC(nullptr), InstanceIndex(INDEX_NONE) {}

    FTileInstanceInfo(UHierarchicalInstancedStaticMeshComponent* InHISMC, int32 InIndex)
        : HISMC(InHISMC), InstanceIndex(InIndex) {}
};

/// <summary>
/// Struct for tile-type data table.
/// </summary>
USTRUCT(BlueprintType,
    meta = (ToolTip = "Struct for tile-type data table"))
struct FTileTypeMapping : public FTableRowBase
{
    GENERATED_BODY()

    // Tile type name, has to be same as row.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type Mapping")
    FName TileTypeName;

    // Movement cost or tile cost.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type Mapping")
    double MovementCost;

    // Color for the type.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type Mapping")
    FColor TypeColor;

};

/// <summary>
/// Struct for tile-state data table.
/// </summary>
USTRUCT(BlueprintType,
    meta = (ToolTip = "Struct for tile-state data table"))
struct FTileStateMapping : public FTableRowBase 
{
    GENERATED_BODY()

    // Tile state name, has to be same as row.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile State Mapping")
    FName TileStateName;

    // Color for the state.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile State Mapping")
    FColor StateColor;
};

/// <summary>
/// Holds data about each tile in grid.
/// </summary>
USTRUCT(BlueprintType,
    meta = (ToolTip = "Holds data about each tile in grid."))
struct FTileData
{
    GENERATED_BODY()

    // Default constructor
    FTileData()
        : Index(FIntPoint(-9999, -9999)), GCost(0), HCost(0), FCost(0), MovementCost(1),
        ParentIndex(FIntPoint(-9999, -9999)), Neighbors({}),
        TileTransform(FTransform()), TileType(FName("Default")),
        TileState({ FName("None") }), bIsWalkable(true) {}

    FTileData(FIntPoint InIndex, float InGCost, float InHCost, float InFCost, float InMovementCost,
        FIntPoint InParent, TArray<FIntPoint> InNeighbors, FTransform InTileTransform,
        FName InTileType, TArray<FName> InTileState, bool InIsWalkable)
        : Index(InIndex), GCost(InGCost), HCost(InHCost), FCost(InFCost), MovementCost(InMovementCost),
        ParentIndex(InParent), Neighbors(InNeighbors), TileTransform(InTileTransform),
        TileType(InTileType), TileState(InTileState), bIsWalkable(InIsWalkable) {}


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    FIntPoint Index;  // This tile's index (X, Y)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, 
        Category = "Tile Data", meta = (ToolTip = " Cost from start tile."))
    float GCost;             // Cost from start tile, only calculated during pathfinding

    UPROPERTY(EditAnywhere, BlueprintReadWrite, 
        Category = "Tile Data", meta = (ToolTip = "Cost from target tile."))
    float HCost;             // Cost from target tile, only calculated during pathfinding

    UPROPERTY(EditAnywhere, BlueprintReadWrite, 
        Category = "Tile Data", meta = (ToolTip = "Total Cost (HCost + GCost)."))
    float FCost;            // Total Cost, only calculated during pathfinding

    UPROPERTY(EditAnywhere, BlueprintReadWrite, 
        Category = "Tile Data", meta = (ToolTip = "Cost for entering tile."))
    float MovementCost;    // Cost for entering tile.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    FIntPoint ParentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    TArray<FIntPoint> Neighbors; // Precomputed neighbors. Not used in this implementation. NOT SAFE FOR MULTITHREADING/ASYNC.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    FTransform TileTransform; // Transform of the tile in world space.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    FName TileType; // Type of tile

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    TArray<FName> TileState; // States of tile

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    bool bIsWalkable; // Is the tile walkable? Only calculated and used in Pathfinding.

    // Equality override for arrays

    bool operator==(const FTileData& Other) const
    {
        return Index == Other.Index && GCost == Other.GCost && HCost == Other.HCost &&
            FCost == Other.FCost && ParentIndex == Other.ParentIndex && TileTransform.Equals(Other.TileTransform, KINDA_SMALL_NUMBER)
            && TileType == Other.TileType && bIsWalkable == Other.bIsWalkable;
    }
};