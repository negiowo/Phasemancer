// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#pragma once
#include "Toolkits/BaseToolkit.h"
#include "CustomEdMode.h"
#include "SEdModeWidget.h"

class FEdModeToolkit : public FModeToolkit
{
public:
    FEdModeToolkit()
    {
        SAssignNew(EdModeWidget, SEdModeWidget);
    }

    /** IToolkit interface */
    virtual FName GetToolkitFName() const override { return FName("TileEditorMode"); }
    virtual FText GetBaseToolkitName() const override { return NSLOCTEXT("BuilderModeToolkit", "DisplayName", "Builder"); }
    virtual class FEdMode* GetEditorMode() const override { return GLevelEditorModeTools().GetActiveMode(FCustomEdMode::EM_TileEditorMode); }
    virtual TSharedPtr<class SWidget> GetInlineContent() const override { return EdModeWidget; }

private:
    TSharedPtr<SEdModeWidget> EdModeWidget;
};

#endif