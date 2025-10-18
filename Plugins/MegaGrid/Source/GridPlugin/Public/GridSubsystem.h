// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "GridDataStructures.h"
#include "GridPluginSave.h"
#include "GridSubsystem.generated.h"

class AGridData;
class AGridVisuals;

/// <summary>
/// State to differentiate Editor and Runtime.
/// When accessing grid data, it's always state specific.
/// </summary>
USTRUCT()
struct FGridState
{
    GENERATED_BODY()

    // Store grid data
    TMap<FIntPoint, FTileData> GridData;
};

/// <summary>
/// GridSubsytem acts as the core for the grid.
/// Managing and storing save specific grid data and more.
/// </summary>
UCLASS()
class GRIDPLUGIN_API UGridSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
private:

    /// <summary>
    /// Returns the currently active grid state
    /// </summary>
    /// <returns></returns>
    FGridState* GetActiveState();

    /// <summary>
    /// Editor-specific grid state
    /// </summary>
    FGridState EditorState;

    /// <summary>
    /// Runtime-specific grid state
    /// </summary>
    FGridState RuntimeState;

    /// <summary>
    /// Determines if the system is in runtime mode
    /// </summary>
    bool bIsRuntime;



public:

    UGridSubsystem();

    /// <summary>
    /// For async handling
    /// </summary>
    FCriticalSection Mutex;

    FCriticalSection HeapMutex;

    /// <summary>
    /// Which save file is the subsystem currently using.
    /// </summary>
    FString CurrentSaveName;
    bool bIsInitialized;

    /// <summary>
    /// Start index for the hex grid.
    /// </summary>
    UPROPERTY()
    FIntPoint StartIndex;

    /// <summary>
    /// End index for the hex grid.
    /// </summary>
    UPROPERTY()
    FIntPoint EndIndex;

    /// <summary>
    /// Previous grid size
    /// </summary>
    UPROPERTY()
    FVector2D LastGridSize = FVector2D(100, 100);

    /// <summary>
    /// Current grid size
    /// </summary>
    UPROPERTY()
    FVector2D GridSize = FVector2D(100, 100);

    /// <summary>
    /// Last saved offset
    /// </summary>
    UPROPERTY()
    FVector2D GridOffset;

    /// <summary>
    /// Current tile count
    /// </summary>
    UPROPERTY()
    FIntPoint TileCount;

    /// <summary>
    /// Is the grid a hex?
    /// </summary>
    UPROPERTY()
    bool bIsHex;

    UPROPERTY()
    float LineWidth;

    UPROPERTY()
    float LineOpacity;

    UPROPERTY()
    float TileOpacity;

    UPROPERTY()
    FLinearColor TileColor;

    UPROPERTY()
    FLinearColor LineColor;

    /// <summary>
    /// Trace channels to be used throughout the plugin
    /// </summary>
    ECollisionChannel GridSurfaceChannel = ECC_GameTraceChannel1;
    ECollisionChannel AutoMapChannel = ECC_GameTraceChannel2;

    // Getters

    /// <summary>
    /// Returns the tile data of the given tile index.
    /// </summary>
    /// <param name="Index"></param>
    /// <returns></returns>
    FTileData GetTileData(FIntPoint TileIndex);

    /// <summary>
    /// Returns the grid data of the active state.
    /// </summary>
    /// <returns></returns>
    TMap<FIntPoint, FTileData>& GetGridData();

    /// <summary>
    /// Returns the current grid size.
    /// </summary>
    /// <returns></returns>
    FVector2D GetGridSize();

    /// <summary>
    /// Returns the current grid offset.
    /// </summary>
    /// <returns></returns>
    FVector2D GetGridOffset();

    /// <summary>
    /// Returns the current tile count.
    /// </summary>
    /// <returns></returns>
    FIntPoint GetTileCount();

    /// <summary>
    /// Returns bIsHex.
    /// </summary>
    /// <returns></returns>
    bool GetIsHex();

    float GetLineWidth();
    float GetOpacity(bool bIsLine);
    FLinearColor GetColor(bool bIsLine);

    // Setters

    /// <summary>
    /// Sets / overwrites grid data of the active state.
    /// </summary>
    /// <param name="GridData"></param>
    void SetGridData(TMap<FIntPoint, FTileData> GridData);

    /// <summary>
    /// Adds or updates the given tile index with the new TileData. 
    /// </summary>
    /// <param name="Index"></param>
    /// <param name="TileData"></param>
    void AddTileToGrid(FIntPoint TileIndex, FTileData TileData);

    /// <summary>
    /// Removes the given tile from grid data of the active state.
    /// </summary>
    /// <param name="Index"></param>
    void RemoveTileFromGrid(FIntPoint TileIndex);

    /// <summary>
    /// Clears grid data of the active state.
    /// </summary>
    void ClearGridData();

    /// <summary>
    /// Sets grid size.
    /// </summary>
    /// <param name="_GridSize"></param>
    /// <returns></returns>
    FVector2D SetGridSize(FVector2D _GridSize);

    /// <summary>
    /// Sets grid offset.
    /// </summary>
    /// <param name="_GridOffset"></param>
    /// <returns></returns>
    FVector2D SetGridOffset(FVector2D _GridOffset); 

    /// <summary>
    /// Sets tile count.
    /// </summary>
    /// <param name="_TileCount"></param>
    /// <returns></returns>
    FIntPoint SetTileCount(FIntPoint _TileCount);

    /// <summary>
    /// Sets bIsHex.
    /// </summary>
    /// <param name="_bIsHex"></param>
    /// <returns></returns>
    bool SetIsHex(bool _bIsHex);

    float SetLineWidth(float Width);
    float SetOpacity(float Opacity, bool bIsLine);
    FLinearColor SetColor(FLinearColor Color, bool bIsLine);
   
    /// <summary>
    /// Sets / overwrites tile data at the containing tile index.
    /// </summary>
    /// <param name="TileData"></param>
    void SetTileData(FTileData TileData);

    /// <summary>
    /// Sets the current save name.
    /// </summary>
    /// <param name="LevelName"></param>
    void SetSaveFileName(FString SaveName);
   
    /// <summary>
    /// Returns bIsRuntime.
    /// </summary>
    /// <returns></returns>
    UFUNCTION()
    bool GetIsRuntime();

    /// <summary>
    /// Sets bIsRuntime.
    /// </summary>
    /// <param name="bEnable"></param>
    UFUNCTION()
    void SetRuntimeMode(bool bEnable);

    // Save and Load

    /// <summary>
    /// Creates a new .sav file of the given name.
    /// </summary>
    /// <param name="SaveName"></param>
    UFUNCTION()
    void CreateSaveData(FString SaveName);

    /// <summary>
    /// Saves all grid related data to the given save file.
    /// </summary>
    /// <param name="SaveName"></param>
    UFUNCTION()
    void SaveGridData(FString SaveName);

    /// <summary>
    /// Loads all grid related data from the given save file.
    /// </summary>
    /// <param name="SaveName"></param>
    UFUNCTION()
    void LoadGridData(FString SaveName);

    /// <summary>
    /// Called when the subsystem is initialized
    /// </summary>
    /// <param name="Collection"></param>
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /// <summary>
    /// Called when the subsystem is deinitialized
    /// </summary>
    virtual void Deinitialize() override;

 };
