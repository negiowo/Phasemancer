// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#pragma once
#include "ToolEditor/TabToolBase.h"

class EdModeTool : public FTabToolBase
{
public:
    virtual void OnStartupModule() override;
    virtual void OnShutdownModule() override;

    virtual ~EdModeTool() {}
private:
    static TSharedPtr< class FSlateStyleSet > StyleSet;

    /// <summary>
    /// Register style set like icons, etc.
    /// </summary>
    void RegisterStyleSet();

    /// <summary>
    /// Unregister style set like icons, etc.
    /// </summary>
    void UnregisterStyleSet();
    
    /// <summary>
    /// Register Editor Mode to be used with ToolEditor.
    /// </summary>
    void RegisterEditorMode();

    void UnregisterEditorMode();
};

#endif