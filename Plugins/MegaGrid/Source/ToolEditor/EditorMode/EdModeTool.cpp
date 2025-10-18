// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#include "EdModeTool.h"
#include "ToolEditor/ToolEditor.h"
#include "CustomEdMode.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

TSharedPtr< FSlateStyleSet > EdModeTool::StyleSet = nullptr;

void EdModeTool::OnStartupModule()
{
    RegisterStyleSet();
    RegisterEditorMode();
}

void EdModeTool::OnShutdownModule()
{
    UnregisterStyleSet();
    UnregisterEditorMode();
}

void EdModeTool::RegisterStyleSet()
{
    // Default icon sizes
    const FVector2D Icon20x20(20.0f, 20.0f);
    const FVector2D Icon40x40(40.0f, 40.0f);

    if (StyleSet.IsValid())
    {
        return;
    }

    StyleSet = MakeShareable(new FSlateStyleSet("MegaGridStyle"));

    // Set the content root to your plugin's Resources folder
    FString PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("MegaGrid"))->GetBaseDir();
    StyleSet->SetContentRoot(PluginBaseDir / TEXT("Resources/Editor"));
    StyleSet->SetCoreContentRoot(PluginBaseDir / TEXT("Resources/Editor"));

    // Register the icons
    StyleSet->Set("MegaGrid.TileMode", new IMAGE_BRUSH(TEXT("TileModeIcon"), Icon40x40));  // 40x40 icon
    StyleSet->Set("MegaGrid.TileMode.Small", new IMAGE_BRUSH(TEXT("TileModeIcon"), Icon20x20));  // 20x20 icon

    StyleSet->Set("MegaGrid.AddTile", new IMAGE_BRUSH(TEXT("AddTileIcon"), Icon40x40));  // 40x40 icon
    StyleSet->Set("MegaGrid.AddTile.Small", new IMAGE_BRUSH(TEXT("AddTileIcon"), FVector2D(10, 10)));  // 20x20 icon

    StyleSet->Set("MegaGrid.RemoveTile", new IMAGE_BRUSH(TEXT("RemoveTileIcon"), Icon40x40));  // 40x40 icon
    StyleSet->Set("MegaGrid.RemoveTile.Small", new IMAGE_BRUSH(TEXT("RemoveTileIcon"), FVector2D(10, 10)));  // 20x20 icon

    StyleSet->Set("MegaGrid.ClearAll", new IMAGE_BRUSH(TEXT("ClearAllIcon"), Icon40x40));  // 40x40 icon
    StyleSet->Set("MegaGrid.ClearAll.Small", new IMAGE_BRUSH(TEXT("ClearAllIcon"), Icon20x20));  // 20x20 icon

    FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void EdModeTool::UnregisterStyleSet()
{
    if (StyleSet.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
        ensure(StyleSet.IsUnique());
        StyleSet.Reset();
    }
}

void EdModeTool::RegisterEditorMode()
{
    // Register the editor mode.
    FEditorModeRegistry::Get().RegisterMode<FCustomEdMode>(
        FCustomEdMode::EM_TileEditorMode,
        FText::FromString("Tile Editor"),
        FSlateIcon(StyleSet->GetStyleSetName(), "MegaGrid.TileMode", "MegaGrid.TileMode.Small"),
        true, 500 // Add icon
    );
}

void EdModeTool::UnregisterEditorMode()
{
    FEditorModeRegistry::Get().UnregisterMode(FCustomEdMode::EM_TileEditorMode);
}

#undef IMAGE_BRUSH

#endif