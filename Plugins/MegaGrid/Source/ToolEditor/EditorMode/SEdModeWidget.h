// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#pragma once

#include "Framework/Application/SlateApplication.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SComboBox.h"
#include "GridPlugin/Public/GridSubsystem.h"

/// <summary>
/// Widget class for Tile Editor mode.
/// </summary>
class SEdModeWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SEdModeWidget) {}
    SLATE_END_ARGS();

    /// <summary>
    /// Tick but for widgets.
    /// </summary>
    /// <param name="AllottedGeometry"></param>
    /// <param name="InCurrentTime"></param>
    /// <param name="InDeltaTime"></param>
    void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, 
        const float InDeltaTime) override;

    /// <summary>
    /// Where widgets are initialized.
    /// </summary>
    /// <param name="InArgs"></param>
    void Construct(const FArguments& InArgs);
    
    /// <summary>
    /// Gets the editor mode.
    /// </summary>
    /// <returns></returns>
    class FCustomEdMode* GetEdMode() const;
    UGridSubsystem* GridSubsystem;

    /// <summary>
    /// Current selected tile type.
    /// </summary>
    TSharedPtr<FString> SelectedType;

    /// <summary>
    /// Type dropdown.
    /// </summary>
    TArray<TSharedPtr<FString>> TypeDropdownOptions;
    
    /// <summary>
    /// Generator for type dropdown.
    /// </summary>
    /// <returns></returns>
    TSharedRef<SWidget> GenerateTypeDropdownWidget(TSharedPtr<FString> InItem);
    
    /// <summary>
    /// Dropdown selection handler.
    /// </summary>
    /// <returns></returns>
    void OnDropdownSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
    
    /// <summary>
    /// Dynamically populates types from tile-type data table.
    /// </summary>
    /// <returns></returns>
    void PopulateTypeDropdown(UDataTable* OptionsDataTable);
    
    /// <summary>
    /// Brush size display text.
    /// </summary>
    TSharedPtr<STextBlock> BrushSizeText;

    /// <summary>
    /// Current checked button - Add or Remove?
    /// </summary>
    FName CurrentCheckedTypeButton;
    TSharedPtr<SCheckBox> AddTypeCheckBox;
    TSharedPtr<SCheckBox> RemoveTypeCheckBox;

    /// <summary>
    /// Called when clear all types checkbox is changed.
    /// </summary>
    void OnClearAllCheckBoxChanged(ECheckBoxState NewState);
    
    /// <summary>
    /// Called when fill only empty checkbox is changed.
    /// </summary>
    void OnFillOnlyEmptyCheckBoxChanged(ECheckBoxState NewState);
    
    /// <summary>
    /// Clear button handler.
    /// </summary>
    FReply OnClearButtonClicked();

    /// <summary>
    /// Gets the current state of clear all types.
    /// </summary>
    ECheckBoxState GetCurrentClearAllTypes();

    /// <summary>
    /// Gets the current state of fill only empty.
    /// </summary>
    ECheckBoxState GetCurrentFillOnlyEmptyState();

    /// <summary>
    /// Gets the current state Add or Remove checkbox.
    /// </summary>
    ECheckBoxState GetCurrentAddOrRemoveType(bool bAdd);

    /// <summary>
    /// Called when either Add or Remove types' states change.
    /// </summary>
    void OnCheckStateChanged(ECheckBoxState NewState, FName ButtonName);

    /// <summary>
    /// Gets relevant checkbox by its name.
    /// </summary>
    TSharedPtr<SCheckBox> GetCheckBoxByName(FName ButtonName);

    TSharedPtr<SSlider> BrushSizeSlider = SNew(SSlider);

    /// <summary>
    /// Called when brush size slider changes.
    /// </summary>
    void OnBrushSliderValueChanged(float NewValue);

    /// <summary>
    /// Below are getters for some essential
    /// grid information.
    /// </summary>
    FText GetTileIndex() const;
    FText GetTileLocation() const;
    FText GetTileType() const;
};

#endif