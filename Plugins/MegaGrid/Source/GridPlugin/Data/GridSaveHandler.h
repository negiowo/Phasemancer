// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif

#include "TimerManager.h"
#include "GridSaveHandler.generated.h"

class AGridManager;
class UGridSubsystem;
class AGridVisuals;

/// <summary>
/// Class to help load / save data.
/// Multiple levels can use the same data.
/// This actor is mainly for Editor loading but some functions
/// can be used for runtime purposes.
/// </summary>

UCLASS()
class GRIDPLUGIN_API AGridSaveHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridSaveHandler();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	bool bIsConstructed = false;

	/// <summary>
	/// Loads grid data from the given save file.
	/// </summary>
	/// <param name="SaveFileName"></param>
	UFUNCTION(BlueprintCallable, Category = "Data Functions",
		meta = (ToolTip = "Loads grid data from given file. Also loads the related visuals."))
	void LoadSavedData(FString SaveFileName);
	
	/// <summary>
	/// Saves grid data to the given save file.
	/// </summary>
	/// <param name="SaveFileName"></param>
	UFUNCTION(BlueprintCallable, Category = "Data Functions",
		meta = (ToolTip = "Saves grid data to given file."))
	void SaveData(FString SaveFileName);

#if WITH_EDITOR
	/// <summary>
	/// Auto saves grid data of the current level when switching to a new level.
	/// EDITOR ONLY. Implement a manual save system for your packaged games.
	/// </summary>
	/// <param name="World"></param>
	/// <param name="bSessionEnded"></param>
	/// <param name="bCleanupResources"></param>
	void SaveOnLevelChange(UWorld* World, bool bSessionEnded, bool bCleanupResources);
#endif

	/// <summary>
	/// Timer for delaying load data on construction. 
	/// Doing this to avoid render pipeline crashes.
	/// </summary>
	FTimerHandle DelayTimerHandle;

public:
	/// <summary>
	/// Core references.
	/// </summary>
	UGridSubsystem* GridSubsystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Dependencies")
	AGridManager* GridManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Dependencies")
	AGridVisuals* GridVisuals;

	/// <summary>
	/// Name of the .sav file (Located in Saved/SaveGames)
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Name",
		meta = (ToolTip = "Name of .sav file holding grid data. Cannot be empty."))
	FString SaveName;

	/// <summary>
	/// Copies the .sav files kin to MegaGrid into the final packaged directory.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "Data Functions",
		meta = (ToolTip = "Saves .sav from development directory to packaged directory."))
	void CopyGridSaveToPackagedFolder();
};
