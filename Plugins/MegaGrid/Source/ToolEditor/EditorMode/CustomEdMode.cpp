// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#include "CustomEdMode.h"

#include "ToolEditor/ToolEditor.h"
#include "Toolkits/ToolkitManager.h"
#include "ScopedTransaction.h"
#include "EdModeToolkit.h"
#include "GridSubsystem.h"
#include "DrawDebugHelpers.h"

const FEditorModeID FCustomEdMode::EM_TileEditorMode(TEXT("EM_TileEditorMode"));

void FCustomEdMode::Initialize()
{
    EditorWorld = GEditor->GetEditorWorldContext().World();
}

void FCustomEdMode::Enter()
{
    // Called when Editor Mode is entered.

    FEdMode::Enter(); 
    
    // Get Editor specific world.
    EditorWorld = GEditor->GetEditorWorldContext().World(); 
    GridSubsystem = UGridUtils::GetGridSubsystem();

    // Get references to manager and visuals.
    GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(EditorWorld, AGridManager::StaticClass()));
    GridVisuals = Cast<AGridVisuals>(UGameplayStatics::GetActorOfClass(EditorWorld, AGridVisuals::StaticClass()));
    
    // Pre-load colors from data tables.
    if(GridVisuals)
    { 
        GridVisuals->PrecomputeColors();
    }

    if (GridSubsystem)
    {
        bIsHex = GridSubsystem->GetIsHex(); // Get current grid shape.
    }

    if (GridManager)
    {
        GridManager->OnShapeChanged.AddLambda([this]()
            {
                OnGridShapeChanged(); // Bind function to grid shape changes.
            });

        GridManager->OnDataTableAssigned.AddLambda([this]()
            {
                TileTypeMapping = GridManager->TileTypeMapping; // Bind function to data table changes.   
            });

        TileTypeMapping = GridManager->TileTypeMapping;
    }

    if (GridManager && IsValid(TileTypeMapping))
    {
        static const FString ContextString(TEXT("Tile Mapping EdMode."));
        FTileTypeMapping* FoundRow = TileTypeMapping->FindRow<FTileTypeMapping>(TileType, ContextString, true);

        if (!FoundRow)
        {
            TileType = FName("Default");
            SelectedType = MakeShared<FString>(TileType.ToString());
        }
    }

    else
    {
        TileType = FName("Default");
        SelectedType = MakeShared<FString>(TileType.ToString());
    }

    if (!Toolkit.IsValid())
    {
        Toolkit = MakeShareable(new FEdModeToolkit);
        Toolkit->Init(Owner->GetToolkitHost());
    }

    UE_LOG(LogTemp, Warning, TEXT("Tile Editor Mode Entered!"));
}

void FCustomEdMode::Exit()
{
    FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
    Toolkit.Reset();

    if (GridManager)
    {
        // Clear all binds

        GridManager->OnShapeChanged.Clear();
        GridManager->OnDataTableAssigned.Clear();
    }

    UE_LOG(LogTemp, Warning, TEXT("Tile Editor Mode Exitted!"));

    FEdMode::Exit();
}

bool FCustomEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
    // Called whenever a input is detected.

    if (Key == EKeys::LeftMouseButton)
    {   
        if (Event == IE_Pressed)
        {
            // Only handle drag is mouse is pressed.
            bIsDragging = true;
            HandleDrag(ViewportClient);

            return true;

        }
        else if (Event == IE_Released)
        {
            bIsDragging = false;
            return true;
        }
    }

    return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

bool FCustomEdMode::CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
    // Called whenever the mouse moves.
    
    if (bIsDragging)
    {
        HandleDrag(InViewportClient);   
    }

    return FEdMode::CapturedMouseMove(InViewportClient, InViewport, InMouseX, InMouseY);
}

void FCustomEdMode::HandleDrag(FEditorViewportClient* ViewportClient)
{
    // Convert mouse screen-space position to world location.
    FVector WorldPosition = ConvertMouseToWorld(ViewportClient).vLocation; 

    // Get tile index for the world position.
    FIntPoint TileIndex = UGridUtils::GetIndexFromLocation(WorldPosition, bIsHex);

    // Snap to grid.
    FVector TilePosition = UGridUtils::GetLocationFromIndex(TileIndex, bIsHex);  
    FVector2D GridSize = UGridUtils::GetGridSize();

    FColor DebugColor = FColor::Emerald;

    // Surface tested tile position.
    bool bOutHit;
    TilePosition = UGridUtils::GetSurfaceLocation(GetWorld(), TilePosition, nullptr, 
        bOutHit);
    
    // Storing current tile index for widget use.
    ClickedIndex = TileIndex;

    if (GridSubsystem)
    {
        FTileData TileData = GridSubsystem->GetTileData(TileIndex);
        
        // Storing current tile position and type for widget use.
        ClickedLocation = TilePosition;
        ClickedType = TileData.TileType;
        
    }

    // If the tile is new, modify it.
    if (TileIndex != PreviousIndex) 
    {
        ModifyTilesUnderCursor(TileIndex);
        PreviousIndex = TileIndex;   
    }
    
    // Draw a debug sphere for visualization.
    DrawDebugSphere(GetWorld(), TilePosition, GridSize.X / 2, 12, DebugColor, false, 0.2, 0, 10);
}

FTwoVectors_Struct FCustomEdMode::ConvertMouseToWorld(FEditorViewportClient* ViewportClient)
{   
    // Converts mouse screen-space position to
    // world position.

    FTwoVectors_Struct Result;
    FVector2D GridSize = UGridUtils::GetGridSize();
    FVector2D GridOffset = UGridUtils::GetGridOffset();

    FEditorViewportClient* EClient = ViewportClient;

    // Get mouse position.
    CurrentMousePosition.X = EClient->GetCachedMouseX();
    CurrentMousePosition.Y = EClient->GetCachedMouseY();

    FSceneViewFamily ViewFamily = FSceneViewFamily::ConstructionValues(EClient->Viewport, EClient->GetScene(), EClient->EngineShowFlags);
    FSceneView* CulScene = EClient->CalcSceneView(&ViewFamily);

    if (!CulScene)
    {
        return Result;
    }

    // De-project the mouse position to world coordinates
    CulScene->DeprojectFVector2D(CurrentMousePosition, Result.vLocation, Result.vRotation);

    FVector Start = CulScene->ViewLocation; // Start from the camera location
    FVector End = Start + (Result.vRotation * 9999999); // Extend in the direction of the rotation

    // Line trace parameters
    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;

    // Perform the line trace
    if (GWorld->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams))
    {        
        Result.vLocation = HitResult.Location; 
    }

    return Result;
}

void FCustomEdMode::ClearTypes()
{
    // Clears tile types.
    // If cleared, type is set to Default.
    
    if (!GridVisuals || !GridSubsystem)
    {
        return;
    }

    if (bClearAllTypes)
    {
        GridManager->ClearAllTypes(EGridVisualContext::Editor, true);
    }
    else
    {
        GridManager->ClearAllTilesOfType(TileType, EGridVisualContext::Editor, true);
    }
}

TArray<FIntPoint> FCustomEdMode::GetAffectedIndices(FIntPoint CursorIndex)
{
    // Get the affected indices under brush area.

    TArray<FIntPoint> AffectedIndices;

    if (!bIsHex)
    {
        for (int32 X = -BrushSize; X <= BrushSize; ++X)
        {
            for (int32 Y = -BrushSize; Y <= BrushSize; ++Y)
            {
                FIntPoint NewIndex = CursorIndex + FIntPoint(X, Y);
                AffectedIndices.Add(NewIndex);
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
                
                AffectedIndices.Add(HexCoordinate);
            }
        }
    }

    return AffectedIndices;
}

void FCustomEdMode::ModifyTilesUnderCursor(FIntPoint CursorIndex)
{ 
   // Get affected indices.
    TArray<FIntPoint> AffectedIndices = UGridUtils::GetIndicesUnderBrush(CursorIndex, BrushSize, FIntPoint(0));

    if (!GridVisuals) {
        UE_LOG(LogTemp, Error, TEXT("GridVisuals Actor is not valid!"));
        return;
    }

    for (const FIntPoint& Index : AffectedIndices)
    {   
        FIntPoint InIndex = Index;

        const FString ContextString(TEXT("Tile Mapping Context"));

        FTileTypeMapping* FoundRow = TileTypeMapping->FindRow<FTileTypeMapping>(TileType, ContextString, true);

        if (FoundRow)
        {
            // Get movement cost from data table.
            TileCost = FoundRow->MovementCost;
        }

        FTileData* Node = GridSubsystem->GetGridData().Find(InIndex);

        if (bAddType)
        {
            if (Node)
            {
                if (bOnlyFillEmpty)
                {
                    // Only add if the type is Default
                    if (Node->TileType == FName("Default"))
                    {
                        Node->TileType = TileType;
                        Node->MovementCost = TileCost;
                    }
                }
                else 
                {
                    Node->TileType = TileType;
                    Node->MovementCost = TileCost;
                }
            }

            // Update visuals.
            GridVisuals->AddTileVisualEditor(Index);

        }

        else if (bRemoveType)
        {

            if (Node)
            {
                if (!bClearAllTypes)
                {
                    // Only clear if the type matches.
                    if (Node->TileType == TileType)
                    {
                        Node->TileType = FName("Default");
                        Node->MovementCost = TileCost;
                    }
                }
                
                else 
                {
                    Node->TileType = FName("Default");
                    Node->MovementCost = TileCost;
                }
            }

            // Update visuals.
            GridVisuals->AddTileVisualEditor(Index);
        }
    }
}

void FCustomEdMode::SetTileType(TSharedPtr<FString> Type)
{
    const FString& SelectedValue = *Type;

    TileType = FName(SelectedValue);
}

void FCustomEdMode::OnGridShapeChanged()
{
    bIsHex = UGridUtils::GetGridSubsystem()->GetIsHex();
}

#endif
