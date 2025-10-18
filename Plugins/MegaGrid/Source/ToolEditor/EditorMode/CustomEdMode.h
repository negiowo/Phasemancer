// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#pragma once

#include "GridUtils.h"
#include "GridManager.h"
#include "GridPlugin/Visuals/GridVisuals.h"
#include "EditorModes.h"
#include "EdMode.h"

class UGridSubsystem;

/// <summary>
/// Struct used for converting cursor position
/// to world location.
/// </summary>
struct FTwoVectors_Struct
{
    FVector vLocation;
    FVector vRotation; 
};

/// <summary>
/// This class handles the TileEditor mode.
/// </summary>
class FCustomEdMode : public FEdMode
{
public:
    const static FEditorModeID EM_TileEditorMode;

    // FEdMode interface
    virtual void Initialize() override;
    virtual void Enter() override;
    virtual void Exit() override;
    
    /// <summary>
    /// Handles input in the custom editor mode.
    /// </summary>
    /// <param name="InViewportClient"></param>
    /// <param name="InViewport"></param>
    /// <param name="InMouseX"></param>
    /// <param name="InMouseY"></param>
    /// <returns></returns>
    bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
    
    /// <summary>
    /// Called every time mouse is moved in custom editor viewport.
    /// </summary>
    virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;

    /// <summary>
    /// Handles mouse drag.
    /// </summary>
    /// <param name="ViewportClient"></param>
    void HandleDrag(FEditorViewportClient* ViewportClient);

    /// <summary>
    /// Converts mouse position to world location.
    /// </summary>
    /// <param name="ViewportClient"></param>
    /// <returns></returns>
    FTwoVectors_Struct ConvertMouseToWorld(FEditorViewportClient* ViewportClient);

public:

    UWorld* EditorWorld;
    
    UPROPERTY()
    AGridManager* GridManager = nullptr;

    UPROPERTY()
    UDataTable* TileTypeMapping;

    /// <summary>
    /// Current screen-space mouse position.
    /// </summary>
    FIntPoint CurrentMousePosition;
   
    bool bIsHex;

    /// <summary>
    /// Flag that tells if the user is 
    /// click-dragging mouse.
    /// </summary>
    bool bIsDragging = false;

    /// <summary>
    /// Flag that tells mouse handler
    /// to add tile types.
    /// </summary>
    bool bAddType = false;

    /// <summary>
    /// Flag that tells mouse handler
    /// to remove tile types.
    /// </summary>
    bool bRemoveType = false;

    /// <summary>
    /// Current tile type.
    /// </summary>
    FName TileType = "Default";

    /// <summary>
    /// Movement cost of type.
    /// </summary>
    float TileCost = 0;

    /// <summary>
    /// Flag to clear all types.
    /// If false only selected type is affected.
    /// </summary>
    bool bClearAllTypes = true;

    /// <summary>
    /// If true, only Default valued
    /// tiles are affected.
    /// </summary>
    bool bOnlyFillEmpty = false;

    /// <summary>
    /// Below are variables for 
    /// editor mode widget.
    /// </summary>
    FIntPoint ClickedIndex;
    FVector ClickedLocation;
    FName ClickedType;
    TSharedPtr<FString> SelectedType;

public:

    /// <summary>
    /// Number of tiles to affect under cursor.
    /// Think of this as the radius from center.
    /// </summary>
    int32 BrushSize = 0;

    /// <summary>
    /// Last tile under cursor. 
    /// Used for avoiding redundant calls.
    /// </summary>
    FIntPoint PreviousIndex = FIntPoint(-999,-99);

    /// <summary>
    /// Gets affected indices under brush area.
    /// </summary>
    /// <param name="CursorIndex"></param>
    /// <returns></returns>
    TArray<FIntPoint> GetAffectedIndices(FIntPoint CursorIndex);

    /// <summary>
    /// Modifies tiles under cursor.
    /// Adds or removes types.
    /// </summary>
    /// <param name="CursorIndex"></param>
    void ModifyTilesUnderCursor(FIntPoint CursorIndex);

    /// <summary>
    /// Clears types.
    /// </summary>
    void ClearTypes();

public:

    AGridVisuals* GridVisuals = nullptr;
    UGridSubsystem* GridSubsystem;

public:
    /// <summary>
    /// Used by the widget to assign 
    /// selected type.
    /// </summary>
    /// <param name="Type"></param>
    void SetTileType(TSharedPtr<FString> Type);

    /// <summary>
    /// Called when grid shape is changed.
    /// </summary>
    void OnGridShapeChanged();
};

#endif