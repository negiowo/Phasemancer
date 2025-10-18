// Copyright Two Bit Studios 2025. All Rights Reserved.


#include "GridSaveHandler.h"
#include "GridManager.h"
#include "GridSubsystem.h"
#include "GridVisuals.h"

#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "Engine/World.h"
#include "Engine/Engine.h"

#include "UObject/ObjectSaveContext.h"

// Sets default values
AGridSaveHandler::AGridSaveHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AGridSaveHandler::BeginPlay()
{
	
	Super::BeginPlay();

	GridSubsystem = UGridUtils::GetGridSubsystem();

	/// NOTE: For editor performance, try manually
	/// saving before begin play, takes 3 seconds for 320^2.
	/// SaveData is only called in the editor. Ensure to manually save the data before packaging the game.
	
#if WITH_EDITOR
	SaveData(SaveName); 
#endif

	/// NOTE: LoadData is only called in packaged builds.
	/// Ensure that the SaveName is specified in the editor properties before shipping.

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT || UE_BUILD_SHIPPING || UE_BUILD_TEST
	GridSubsystem->LoadGridData(SaveName);
#endif

}

void AGridSaveHandler::BeginDestroy()
{
	bIsConstructed = false;

	Super::BeginDestroy();
}

void AGridSaveHandler::OnConstruction(const FTransform& Transform)
{
	// Only loading data once during construction.

#if WITH_EDITOR 

	if (!bIsConstructed)
	{
		// NOTE: The below is an auto-save delegate for when a new level is opened.
		// This is to save the current level's data. But I'm not using it because it's very
		// unpredictable when used in conjunction with OnConstruction LoadGridData(). 
		
		//FWorldDelegates::OnWorldCleanup.AddUObject(this, &AGridSaveHandler::SaveOnLevelChange);
		UE_LOG(LogTemp, Warning, TEXT("GridSaveHandler Constructed"));

		GridSubsystem = UGridUtils::GetGridSubsystem();

		if (GIsEditor && GetWorld() && !GetWorld()->IsGameWorld())
		{
			LoadSavedData(SaveName);
		}

	}

	bIsConstructed = true;

#endif
	Super::OnConstruction(Transform);
}

// Called every frame
void AGridSaveHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGridSaveHandler::LoadSavedData(FString SaveFileName)
{
	// Load data from given file. Ensure such a file exists in the packaged game
	// folder structure. Saves/SaveGames

	if (SaveFileName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Save Name is not valid, please enter a save name"), *SaveFileName);
		return;
	}
	
	// Actual loading of the data via GridSubsystem
	if (GridSubsystem)
	{
		GridSubsystem->LoadGridData(SaveFileName);
	}

	// Completely load the grid visuals  if needed.
	if (GridVisuals)
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimer(
			DelayTimerHandle,
			[this]()
			{
				GridVisuals->LoadAllVisuals(EGridVisualContext::Auto);
			},
			0.12f,  // Add a slight delay to avoid render thread crashes.
			false   
		);
		
	}

	UE_LOG(LogTemp, Warning, TEXT("GridSaveHandler - LoadSavedGridData : %s"), *SaveFileName);
}

void AGridSaveHandler::SaveData(FString SaveFileName)
{
	if (SaveFileName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Save Name is not valid, please enter a save name"), *SaveFileName);
		return;
	}

	// Actual saving of data via GridSubsystem.
	if (GridSubsystem)
	{
		GridSubsystem->SaveGridData(SaveFileName);
	}

	UE_LOG(LogTemp, Warning, TEXT("GridSaveHandler - SavedData to : %s"), *SaveFileName);
}

#if WITH_EDITOR
void AGridSaveHandler::SaveOnLevelChange(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	// Auto-save GridData on level change, EDITOR ONLY!

	UGridUtils::GetGridSubsystem()->SaveGridData(SaveName);
	UE_LOG(LogTemp, Warning, TEXT("Auto Saved: %s"), *SaveName);
}
#endif

void AGridSaveHandler::CopyGridSaveToPackagedFolder()
{
	FString SourceFolder = FPaths::ProjectDir() + TEXT("GridSave/");
	FString DestinationFolder = FPaths::ProjectSavedDir() + TEXT("SaveGames/");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Check if the source directory exists
	if (PlatformFile.DirectoryExists(*SourceFolder))
	{
		// Ensure the destination directory exists; if not, create it
		if (!PlatformFile.DirectoryExists(*DestinationFolder))
		{
			PlatformFile.CreateDirectoryTree(*DestinationFolder);
		}

		// Get a list of files in the source folder
		TArray<FString> SourceFiles;
		PlatformFile.FindFiles(SourceFiles, *SourceFolder, nullptr); // nullptr means all file types

		for (const FString& SourceFile : SourceFiles)
		{
			FString FileName = FPaths::GetCleanFilename(SourceFile); // Extract filename
			FString DestFile = DestinationFolder + FileName; // Target file path

			// Check if the file is missing in the target location
			if (!PlatformFile.FileExists(*DestFile))
			{
				// Copy only the missing file
				PlatformFile.CopyFile(*DestFile, *SourceFile);
			}
		}
	}
}

