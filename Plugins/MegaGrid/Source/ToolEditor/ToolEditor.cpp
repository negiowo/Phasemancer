// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#include "ToolEditor.h"
#include "ICustomModuleInterface.h"
#include "EditorMode/EdModeTool.h"

#define LOCTEXT_NAMESPACE "FToolEditorModule"

// Menu related...
TSharedRef<FWorkspaceItem> FToolEditor::MenuRoot = FWorkspaceItem::NewGroup(FText::FromString("Menu Root"));


void FToolEditor::StartupModule()
{
	ICustomModuleInterface::StartupModule();
}

void FToolEditor::ShutdownModule()
{
	ICustomModuleInterface::ShutdownModule();
}

void FToolEditor::AddModuleListeners()
{
    ModuleListeners.Add(MakeShareable(new EdModeTool));
}

// Menu related...

void FToolEditor::AddMenuExtension(const FMenuExtensionDelegate& extensionDelegate, FName extensionHook, const TSharedPtr<FUICommandList>& CommandList, EExtensionHook::Position position)
{
    MenuExtender->AddMenuExtension(extensionHook, position, CommandList, extensionDelegate);
}

void FToolEditor::MakePulldownMenu(FMenuBarBuilder& menuBuilder)
{
    menuBuilder.AddPullDownMenu(
        FText::FromString("Grid Menu Dummy"),
        FText::FromString("Open Grid menu"),
        FNewMenuDelegate::CreateRaw(this, &FToolEditor::FillPulldownMenu),
        "Example",
        FName(TEXT("ExampleMenu"))
    );
}

void FToolEditor::FillPulldownMenu(FMenuBuilder& menuBuilder)
{
    // just a frame for tools to fill in
    menuBuilder.BeginSection("Grid Menu_1", FText::FromString("Section 1"));
    menuBuilder.AddMenuSeparator(FName("Section_1"));
    menuBuilder.EndSection();

    menuBuilder.BeginSection("Grid Menu_1", FText::FromString("Section 2"));
    menuBuilder.AddMenuSeparator(FName("Section_2"));
    menuBuilder.EndSection();
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToolEditor, ToolEditor)

#endif