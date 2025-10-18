// Copyright Two Bit Studios 2025. All Rights Reserved.

#include "GridSubsystem.h"
#include "Kismet/GameplayStatics.h"

UGridSubsystem::UGridSubsystem()
{
}

FGridState* UGridSubsystem::GetActiveState()
{
	return bIsRuntime ? &RuntimeState : &EditorState;
}

void UGridSubsystem::SetSaveFileName(FString SaveName)
{
	CurrentSaveName = SaveName;
}

void UGridSubsystem::CreateSaveData(FString SaveName)
{
	// Creates a new save game with default values.

	UGridPluginSave* PluginSaveInstance = Cast<UGridPluginSave>(UGameplayStatics::CreateSaveGameObject(UGridPluginSave::StaticClass()));
	if (PluginSaveInstance)
	{
		// Initialize default values for the save data
		PluginSaveInstance->SavedGridData = TMap<FIntPoint, FTileData>();  // Default to an empty map
		PluginSaveInstance->GridSize = FVector2D(200,200);
		PluginSaveInstance->LastGridSize = FVector2D(200, 200);
		PluginSaveInstance->GridOffset = FVector2D(0);
		PluginSaveInstance->bIsHex = bIsHex;
		PluginSaveInstance->TileCount = FIntPoint(50, 50);
		PluginSaveInstance->StartIndex = FIntPoint();
		PluginSaveInstance->EndIndex = FIntPoint();

		PluginSaveInstance->LineWidth = 30;
		PluginSaveInstance->LineColor = FColor::Black;
		PluginSaveInstance->LineOpacity = 1;
		PluginSaveInstance->TileOpacity = 0; 
		PluginSaveInstance->TileColor = FColor::Green;

		UE_LOG(LogTemp, Warning, TEXT("UGridSubsystem::CreateSaveData -> Save Name: %s"), *SaveName);

		UGameplayStatics::SaveGameToSlot(PluginSaveInstance, SaveName, 0);
	}
}

void UGridSubsystem::SaveGridData(FString SaveName)
{
	// Saves all the subsystem variables to the given save game
	
	UGridPluginSave* PluginSaveInstance = Cast<UGridPluginSave>(UGameplayStatics::LoadGameFromSlot(SaveName, 0));

	UE_LOG(LogTemp, Warning, TEXT("UGridSubsystem::SaveGridData -> Save Name: %s"), *SaveName);
	
	// Check if the save file already exists, if yes then proceed to save.
	if (PluginSaveInstance)
	{
		PluginSaveInstance->SavedGridData = EditorState.GridData;
		PluginSaveInstance->GridSize = GridSize;
		PluginSaveInstance->LastGridSize = LastGridSize;
		PluginSaveInstance->GridOffset = GridOffset;
		PluginSaveInstance->bIsHex = bIsHex;
		PluginSaveInstance->TileCount = TileCount;
		PluginSaveInstance->StartIndex = StartIndex;
		PluginSaveInstance->EndIndex = EndIndex;

		PluginSaveInstance->LineWidth = LineWidth;
		PluginSaveInstance->LineColor = LineColor;
		PluginSaveInstance->LineOpacity = LineOpacity;
		PluginSaveInstance->TileOpacity = TileOpacity;
		PluginSaveInstance->TileColor = TileColor;

		UGameplayStatics::SaveGameToSlot(PluginSaveInstance, SaveName, 0);
	}

	// Else, create a new one with default values.
	else
	{			
		CreateSaveData(SaveName);
	}

}

void UGridSubsystem::LoadGridData(FString SaveName)
{
	// Loads all the save game's variables to the subsystem.
	
	UGridPluginSave* PluginSaveInstance = Cast<UGridPluginSave>(UGameplayStatics::LoadGameFromSlot(SaveName, 0));

	// Check if the save file already exists, if yes then proceed to load.
	if (PluginSaveInstance) {

		UE_LOG(LogTemp, Warning, TEXT("Loading GridData from File: %s"), *SaveName);

		EditorState.GridData = PluginSaveInstance->SavedGridData;
		GridSize = PluginSaveInstance->GridSize;
		LastGridSize = PluginSaveInstance->LastGridSize;
		GridOffset = PluginSaveInstance->GridOffset;
		bIsHex = PluginSaveInstance->bIsHex;
		TileCount = PluginSaveInstance->TileCount;
		StartIndex = PluginSaveInstance->StartIndex;
		EndIndex = PluginSaveInstance->EndIndex;

		LineWidth = PluginSaveInstance->LineWidth;
		LineColor = PluginSaveInstance->LineColor;
		LineOpacity = PluginSaveInstance->LineOpacity;
		TileOpacity = PluginSaveInstance->TileOpacity;
		TileColor = PluginSaveInstance->TileColor;

		RuntimeState = EditorState; // Load all the Editor data into runtime
	}

	// Else create a new save and then load it.
	else if (!UGameplayStatics::DoesSaveGameExist(SaveName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("No Save Data Found for Map: %s, Creating New Save!"), *SaveName);

		CreateSaveData(SaveName);
		LoadGridData(SaveName);
	}
}

void UGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Initialize default values.
	bIsInitialized = false;

	GridSize = FVector2D(100, 100);
	LastGridSize = FVector2D(100, 100);
	GridOffset = FVector2D(0, 0);
	bIsHex = false;
	TileCount = FIntPoint(100, 100);

	LineWidth = 10;
	LineColor = FColor::Black;
	LineOpacity = 1;
	TileOpacity = 0.5;
	TileColor = FColor::Emerald;

	UE_LOG(LogTemp, Warning, TEXT("GridSubsystem Initialized!"));
}

void UGridSubsystem::Deinitialize()
{
	Super::Deinitialize();

	// When subsystem is deinitialized save the current file (eg. like closing editor).
	bIsInitialized = false;
	SaveGridData(CurrentSaveName);
}

// Getters

FTileData UGridSubsystem::GetTileData(FIntPoint TileIndex)
{	
	QUICK_SCOPE_CYCLE_COUNTER(STAT_GetTileData);

	// Gets the active state (Editor or Runtime).
	FGridState* ActiveState = GetActiveState();
	FTileData* TileData;
	
	if (ActiveState->GridData.Contains(TileIndex))
	{
		// If tile exists, then return it.
		TileData = ActiveState->GridData.Find(TileIndex);

		return *TileData;
	}

	// else return a constructor.
	return FTileData();
}

TMap<FIntPoint, FTileData>& UGridSubsystem::GetGridData()
{
	FGridState* ActiveState = GetActiveState();

	return ActiveState->GridData;
}

FVector2D UGridSubsystem::GetGridSize()
{
	return GridSize;
}

FVector2D UGridSubsystem::GetGridOffset()
{
	return GridOffset;
}

FIntPoint UGridSubsystem::GetTileCount()
{
	return TileCount;
}

bool UGridSubsystem::GetIsHex()
{
	return bIsHex;
}

float UGridSubsystem::GetLineWidth()
{
	return LineWidth;
}

float UGridSubsystem::GetOpacity(bool bIsLine)
{
	return bIsLine ? LineOpacity : TileOpacity;
}

FLinearColor UGridSubsystem::GetColor(bool bIsLine)
{
	return bIsLine ? LineColor : TileColor;
}

FVector2D UGridSubsystem::SetGridSize(FVector2D _GridSize)
{
	LastGridSize = GridSize;
	GridSize = _GridSize;
	return GridSize;
}

void UGridSubsystem::ClearGridData()
{
	Mutex.Lock(); // Mutex for safe async access.

	FGridState* ActiveState = GetActiveState();
	ActiveState->GridData.Empty();

	Mutex.Unlock();
}

void UGridSubsystem::SetGridData(TMap<FIntPoint, FTileData> GridData)
{
	Mutex.Lock(); // Mutex for safe async access.

	FGridState* ActiveState = GetActiveState();
	ActiveState->GridData = GridData;

	Mutex.Unlock();
}

void UGridSubsystem::SetTileData(FTileData TileData)
{
	Mutex.Lock(); // Mutex for safe async access.

	FGridState* ActiveState = GetActiveState();
	ActiveState->GridData.Add(TileData.Index, TileData);

	Mutex.Unlock();
}

FVector2D UGridSubsystem::SetGridOffset(FVector2D _GridOffset)
{
	GridOffset = _GridOffset;
	return GridOffset;
}

FIntPoint UGridSubsystem::SetTileCount(FIntPoint _TileCount)
{
	TileCount = _TileCount;
	return TileCount;
}

void UGridSubsystem::AddTileToGrid(FIntPoint TileIndex, FTileData TileData)
{	
	Mutex.Lock(); // Mutex for safe async access.

	GetActiveState()->GridData.Add(TileIndex, TileData);

	Mutex.Unlock();
}

void UGridSubsystem::RemoveTileFromGrid(FIntPoint TileIndex)
{
	Mutex.Lock(); // Mutex for safe async access.

	if (GetActiveState()->GridData.Contains(TileIndex))
	{
		GetActiveState()->GridData.FindAndRemoveChecked(TileIndex);
	}

	Mutex.Unlock();
}

bool UGridSubsystem::SetIsHex(bool _bIsHex)
{
	bIsHex = _bIsHex;
	return bIsHex;
}

float UGridSubsystem::SetLineWidth(float Width)
{
	LineWidth = Width;	
	return LineWidth;
}

float UGridSubsystem::SetOpacity(float Opacity, bool bIsLine)
{
	bIsLine ? LineOpacity = Opacity : TileOpacity = Opacity;

	return GetOpacity(bIsLine);
}

FLinearColor UGridSubsystem::SetColor(FLinearColor Color, bool bIsLine)
{	
	bIsLine ? LineColor = Color : TileColor = Color;
	return GetColor(bIsLine);
}

bool UGridSubsystem::GetIsRuntime()
{
	return bIsRuntime;
}

void UGridSubsystem::SetRuntimeMode(bool bEnable)
{	
	// Enables / disables runtime mode.

	bIsRuntime = bEnable;

	if (bIsRuntime) 
	{
		RuntimeState = EditorState;
	}
}
	