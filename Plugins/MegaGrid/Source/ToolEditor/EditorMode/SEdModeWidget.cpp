// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#include "SEdModeWidget.h"
#include "ToolEditor/ToolEditor.h"
#include "CustomEdMode.h"
#include "Misc/DefaultValueHelper.h"

void SEdModeWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    // Get the tile under cursor every tick.
    GetTileIndex();
}

void SEdModeWidget::Construct(const FArguments& InArgs)
{
    // Get all the icons loaded.
    const FSlateBrush* RemoveIconBrush = FSlateStyleRegistry::FindSlateStyle("MegaGridStyle")->GetBrush("MegaGrid.RemoveTile.Small");
    const FSlateBrush* AddIconBrush = FSlateStyleRegistry::FindSlateStyle("MegaGridStyle")->GetBrush("MegaGrid.AddTile.Small");
    const FSlateBrush* ClearIconBrush = FSlateStyleRegistry::FindSlateStyle("MegaGridStyle")->GetBrush("MegaGrid.ClearAll.Small");

    GridSubsystem = GEngine->GetEngineSubsystem<UGridSubsystem>();
    
    if (GetEdMode()->GridManager)
    {
        UDataTable* TileTypeMapping = GetEdMode()->GridManager->TileTypeMapping;
        if (TileTypeMapping)
        {
            PopulateTypeDropdown(TileTypeMapping); // Dynamically populate type dropdown menu.
        }
    }

    // Set the initial selection to be the last selected type.
    SelectedType = GetEdMode()->SelectedType;

    // Draw widgets.
    ChildSlot
        [
            SNew(SScrollBox)
                + SScrollBox::Slot()
                .VAlign(VAlign_Top)
                .Padding(5.f)
                [
                    SNew(SVerticalBox)

                        // Welcome text block
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(15.f)
                        [ 
                            // Tile Editor mode info.
                            SNew(STextBlock)
                                .Text(FText::FromString(TEXT("Welcome to Tile Editor! \nHere you can set Tile Types for your grid."
                                    "\nEach tile can have its own color and movement cost,"
                                    "\nassign them in the GridManager's Type and State Table and re-enter this mode!")))
                                .AutoWrapText(true)
                        ]

                        // Dark grey area for the buttons
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.f)
                        [
                            SNew(SBorder)
                                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                                .BorderBackgroundColor(FLinearColor::Black) // Dark grey background
                                .Padding(10.f)
                                [
                                    SNew(SVerticalBox)

                                        // Add Type Button
                                        + SVerticalBox::Slot()
                                        .FillHeight(40)
                                        .Padding(0, 0, 0, 2)
                                        [
                                            SNew(SHorizontalBox)
                                                + SHorizontalBox::Slot()
                                                .AutoWidth()                                       
                                                [

                                                    // Creating Add Type button.
                                                    SAssignNew(AddTypeCheckBox, SCheckBox)
                                                        .Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
                                                        .OnCheckStateChanged(this, &SEdModeWidget::OnCheckStateChanged, FName("Add Type"))
                                                        .IsChecked(GetCurrentAddOrRemoveType(true))                          
                                                        .Content()
                                                        [

                                                            // Add icon
                                                            SNew(SImage)
                                                                .Image(AddIconBrush)
                                                                .DesiredSizeOverride(FVector2D(34.f, 5.f))
                                                                .ToolTipText(FText::FromString("Add Type"))
                                                        ]
                                                ]

                                                + SHorizontalBox::Slot()
                                                .AutoWidth()
                                                [
                                                    // Creating Remove Type button.
                                                    SAssignNew(RemoveTypeCheckBox, SCheckBox)
                                                        .CheckBoxContentUsesAutoWidth(true)
                                                        .Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
                                                        .OnCheckStateChanged(this, &SEdModeWidget::OnCheckStateChanged, FName("Remove Type"))
                                                        .IsChecked(GetCurrentAddOrRemoveType(false))
                                                        .Content()
                                                        
                                                        [         
                                                            // Add icon
                                                            SNew(SImage)
                                                                .Image(RemoveIconBrush)
                                                                .DesiredSizeOverride(FVector2D(34.f, 5.f))
                                                                .ToolTipText(FText::FromString("Remove Type"))
     
                                                                
                                                        ]
                                                ]

                                                + SHorizontalBox::Slot()
                                                .AutoWidth() // Allow the button to size automatically
                                                [

                                                    // Creating Clear Type button.
                                                    SNew(SButton)                                                       
                                                        .OnClicked(this, &SEdModeWidget::OnClearButtonClicked) // Define the click event handler
                                                        .DesiredSizeScale(FVector2D(1.01f, 1.01f))
                                                        .Content()
                                                        [
                                                            // Add icon.
                                                            SNew(SImage)
                                                                .Image(ClearIconBrush)
                                                                .DesiredSizeOverride(FVector2D(30.f, 30.f))
                                                                .ToolTipText(FText::FromString("Clear Types"))
                                                        ]
                                                ] 
                                                + SHorizontalBox::Slot()
                                                .AutoWidth() // Ensure the checkbox sizes properly
                                                .Padding(10.f, 10.f, 10.f, 10.f) // Add some spacing between the button and the checkbox
                                                [
                                                    // Clear All Types checkbox.
                                                    SNew(SCheckBox)
                                                        .OnCheckStateChanged(this, &SEdModeWidget::OnClearAllCheckBoxChanged) // Define the state change handler
                                                        .IsChecked(GetCurrentClearAllTypes()) // Default state
                                                        .ToolTipText(FText::FromString("Disabling this allows you to clear specific types"))
                                                        .Content()
                                                        [
                                                            SNew(STextBlock)
                                                                .Text(FText::FromString("Clear All Types"))
                                                        ]
                                                ]

                                        ]
                                ]
                        ]

                        // Dropdown menu
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.f, 5.f)
                        [
                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .VAlign(VAlign_Center)
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(FText::FromString(TEXT("Select Type: ")))
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                [
                                    // Creating dropdown
                                    SNew(SComboBox<TSharedPtr<FString>>)
                                        .OptionsSource(&TypeDropdownOptions)     
                                        .OnGenerateWidget(this, &SEdModeWidget::GenerateTypeDropdownWidget)
                                        .OnSelectionChanged(this, &SEdModeWidget::OnDropdownSelectionChanged)                                        
                                        [
                                            // Updating text (only for display).
                                            SNew(STextBlock)
                                                .Text_Lambda([this]() -> FText {                                                                      
                                                return FText::FromString(SelectedType.IsValid() ? *SelectedType : TEXT("Default"));
                                                    })
                                        ]
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .VAlign(VAlign_Center)
                                .Padding(5.f, 0.f)
                                [
                                    // Fill Only Empty checkbox.
                                    SNew(SCheckBox)
                                        .OnCheckStateChanged(this, &SEdModeWidget::OnFillOnlyEmptyCheckBoxChanged) // Define the state change handler
                                        .IsChecked(GetCurrentFillOnlyEmptyState()) // Default state
                                        .ToolTipText(FText::FromString("Disabling this allows you to clear specific types"))
                                        .Content()
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString("Only Fill Empty"))
                                        ]
                                ]
                        ]
                        // Brush Size
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.f, 5.f)
                        [   
                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(FText::FromString(TEXT("Brush Size: ")))
                                ]
                                
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                
                                [
                                    // Creating BrushSizeText.
                                    SAssignNew(BrushSizeText, STextBlock)
                                        .Text(FText::AsNumber(GetEdMode()->BrushSize)) // Inital size
                                        .Justification(ETextJustify::Center)
                                ]
                                
                                + SHorizontalBox::Slot()
                                .FillWidth(1.0f)
                                .Padding(5.f, 0.f)
                                [
                                    // Creating BrushSizeSlider.
                                    SAssignNew(BrushSizeSlider, SSlider)
                                        .Value(GetEdMode()->BrushSize) 
                                        .MinValue(0.0f)
                                        .MaxValue(8.0f)
                                        .StepSize(1.0f)
                                        .MouseUsesStep(true)
                                        .OnValueChanged(this, &SEdModeWidget::OnBrushSliderValueChanged) // Callback when the slider value changes
                                ]
                        ]

                        // Tile Info
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.f, 5.f)
                        [
                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(FText::FromString(TEXT("Tile Index: ")))
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(this, &SEdModeWidget::GetTileIndex)
                                ]                             
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.f, 5.f)
                        [

                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(FText::FromString(TEXT("Tile Location: ")))
                                ]

                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(this, &SEdModeWidget::GetTileLocation)
                                ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(5.f, 5.f)
                        [
                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(FText::FromString(TEXT("Tile Type: ")))
                                ]

                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5.f, 0.f)
                                [
                                    SNew(STextBlock)
                                        .Text(this, &SEdModeWidget::GetTileType)
                                ]
                        ]
                     ]
                ];
}

FCustomEdMode* SEdModeWidget::GetEdMode() const
{
    // Returns editor mode pointer.
    return (FCustomEdMode*)GLevelEditorModeTools().GetActiveMode(FCustomEdMode::EM_TileEditorMode);
}

TSharedRef<SWidget> SEdModeWidget::GenerateTypeDropdownWidget(TSharedPtr<FString> InItem)
{
    return SNew(STextBlock)
        .Text(FText::FromString(InItem.IsValid() ? *InItem : TEXT("Invalid Item"))); 
}

void SEdModeWidget::OnDropdownSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    // Only process the event if the user selected an option (not programmatic or focus change events)
    if (SelectInfo == ESelectInfo::OnMouseClick || SelectInfo == ESelectInfo::OnKeyPress)
    {
        SelectedType = NewSelection;

        if (NewSelection.IsValid())
        {
            GetEdMode()->SelectedType = NewSelection;
            GetEdMode()->SetTileType(NewSelection);
        }
    }
}

void SEdModeWidget::PopulateTypeDropdown(UDataTable* OptionsDataTable)
{
    if (!OptionsDataTable)
    {
        TypeDropdownOptions.Add(MakeShared<FString>(TEXT("Default")));
        TypeDropdownOptions.Add(MakeShared<FString>(TEXT("Double Cost")));
        TypeDropdownOptions.Add(MakeShared<FString>(TEXT("Triple Cost")));
        TypeDropdownOptions.Add(MakeShared<FString>(TEXT("Obstacle")));
        TypeDropdownOptions.Add(MakeShared<FString>(TEXT("Water")));

        return;
    }

    // Get all rows from the data table
    static const FString ContextString(TEXT("Dropdown Options Context"));
    TArray<FName> RowNames = OptionsDataTable->GetRowNames();

    TypeDropdownOptions.Empty(); // Clear existing options

    for (const FName& RowName : RowNames)
    {
        const FTileTypeMapping* Row = OptionsDataTable->FindRow<FTileTypeMapping>(RowName, ContextString, true);
        if (!Row)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find row for name: %s"), *RowName.ToString());
            continue;
        }

        // Add the valid row to DropdownOptions
        TypeDropdownOptions.Add(MakeShared<FString>(Row->TileTypeName.ToString()));
    }
}

FReply SEdModeWidget::OnClearButtonClicked()
{
    GetEdMode()->ClearTypes();
    return FReply::Handled();
}


void SEdModeWidget::OnClearAllCheckBoxChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        GetEdMode()->bClearAllTypes = true; // Update in editor mode
    }

    else
    {
        GetEdMode()->bClearAllTypes = false; // Update in editor mode
    }
}

void SEdModeWidget::OnFillOnlyEmptyCheckBoxChanged(ECheckBoxState NewState)
{
    if (NewState == ECheckBoxState::Checked)
    {
        GetEdMode()->bOnlyFillEmpty = true; // Update in editor mode
    }

    else
    {
        GetEdMode()->bOnlyFillEmpty = false; // Update in editor mode
    }
}

ECheckBoxState SEdModeWidget::GetCurrentClearAllTypes()
{
    bool Checked = GetEdMode()->bClearAllTypes;
    ECheckBoxState State = Checked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;

    return State;
}

ECheckBoxState SEdModeWidget::GetCurrentFillOnlyEmptyState()
{
    bool Checked = GetEdMode()->bOnlyFillEmpty;
    ECheckBoxState State = Checked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    
    return State;
}

ECheckBoxState SEdModeWidget::GetCurrentAddOrRemoveType(bool bAdd)
{
    bool Checked;
    ECheckBoxState State;

    if (bAdd)
    {
        Checked = GetEdMode()->bAddType; 
        State = Checked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        return State;
    }
    else
    {
        Checked = GetEdMode()->bRemoveType;
        State = Checked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        return State;
    }
}

void SEdModeWidget::OnCheckStateChanged(ECheckBoxState NewState, FName ButtonName)
{
    if (NewState == ECheckBoxState::Checked)
    {   
        // Change conditions in Editor Mode
        if (ButtonName == FName("Add Type")) 
        {
            // Update in editor mode
            GetEdMode()->bAddType = true; 
            GetEdMode()->bRemoveType = false;

            RemoveTypeCheckBox.Get()->SetIsChecked(ECheckBoxState::Unchecked);
        }

        else if (ButtonName == FName("Remove Type")) 
        {
            // Update in editor mode
            GetEdMode()->bAddType = false;
            GetEdMode()->bRemoveType = true;

            AddTypeCheckBox.Get()->SetIsChecked(ECheckBoxState::Unchecked);

        }

        if (CurrentCheckedTypeButton != NAME_None && CurrentCheckedTypeButton != ButtonName)
        {
            // Set the current checkbox back to unchecked
            GetCheckBoxByName(CurrentCheckedTypeButton)->SetIsChecked(ECheckBoxState::Unchecked);
        }

        // Update the current checked button
        CurrentCheckedTypeButton = ButtonName;
    }
    else
    {

        // Set both conditions to false
        GetEdMode()->bAddType = false;
        GetEdMode()->bRemoveType = false;
        
        // If unchecked, reset the current checked button if it's the same as this one
        if (CurrentCheckedTypeButton == ButtonName)
        {
            CurrentCheckedTypeButton = NAME_None;

            AddTypeCheckBox.Get()->SetIsChecked(ECheckBoxState::Unchecked);
            RemoveTypeCheckBox.Get()->SetIsChecked(ECheckBoxState::Unchecked);
        }
    }

}

TSharedPtr<SCheckBox> SEdModeWidget::GetCheckBoxByName(FName ButtonName)
{
    // Make sure the name matches with the widgets.
    if (ButtonName == FName("Add Type"))
    {
        return AddTypeCheckBox;
    }
    else if (ButtonName == FName("Remove Type"))
    {
        return RemoveTypeCheckBox;
    }

    return nullptr;
}

void SEdModeWidget::OnBrushSliderValueChanged(float NewValue)
{
    GetEdMode()->BrushSize = FMath::Floor(NewValue); // flooring to convert to int.
    GetEdMode()->BrushSize = NewValue;

    if (BrushSizeText.IsValid())
    {
        BrushSizeText->SetText(FText::AsNumber(NewValue));
    }
}

FText SEdModeWidget::GetTileIndex() const
{
    FIntPoint& HoveredIndex = GetEdMode()->ClickedIndex;
    return FText::FromString(*HoveredIndex.ToString());
}

FText SEdModeWidget::GetTileLocation() const
{
    FVector& ClickedLocation = GetEdMode()->ClickedLocation;
    return FText::FromString(*ClickedLocation.ToString());
}

FText SEdModeWidget::GetTileType() const
{
    FName ClickedType = GetEdMode()->ClickedType;
    return FText::FromName(ClickedType);
}

#endif