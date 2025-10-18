// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#pragma once

#include "ToolEditor.h"
#include "ICustomModuleInterface.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
/**
 * 
 */
class FTabToolBase : public ICustomModuleListenerInterface, public TSharedFromThis< FTabToolBase >
{
public:

    virtual void OnStartupModule() override
    {
        Initialize();
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, FOnSpawnTab::CreateRaw(this, &FTabToolBase::SpawnTab))
            .SetGroup(FToolEditor::Get().GetMenuRoot())
            .SetDisplayName(TabDisplayName)
            .SetTooltipText(ToolTipText);
    };

    virtual void OnShutdownModule() override
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
    };

    // In this function set TabName/TabDisplayName/ToolTipText
    virtual void Initialize() {};
    virtual TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs) { return SNew(SDockTab); };

    virtual void MakeMenuEntry(FMenuBuilder& menuBuilder)
    {
        FGlobalTabmanager::Get()->PopulateTabSpawnerMenu(menuBuilder, TabName);
    };

protected:
    FName TabName;
    FText TabDisplayName;
    FText ToolTipText;
};

#endif