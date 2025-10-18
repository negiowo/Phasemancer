// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#pragma once

#include "UnrealEd.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "Editor/PropertyEditor/Public/PropertyEditing.h"
#include "IAssetTypeActions.h"
#include "ICustomModuleInterface.h"

class FToolEditor : public ICustomModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    virtual void AddModuleListeners() override;

    static inline FToolEditor& Get()
    {
        return FModuleManager::LoadModuleChecked< FToolEditor >("ToolEditor");
    }

    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("ToolEditor");
    }

// Toolbar Menu Related...

public:
    void AddMenuExtension(const FMenuExtensionDelegate& extensionDelegate, FName extensionHook, const TSharedPtr<FUICommandList>& CommandList = NULL, EExtensionHook::Position position = EExtensionHook::Before);
    TSharedRef<FWorkspaceItem> GetMenuRoot() { return MenuRoot; };

protected:
    TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;
    TSharedPtr<FExtender> MenuExtender;

    static TSharedRef<FWorkspaceItem> MenuRoot;

    void MakePulldownMenu(FMenuBarBuilder& menuBuilder);
    void FillPulldownMenu(FMenuBuilder& menuBuilder);
};

#endif