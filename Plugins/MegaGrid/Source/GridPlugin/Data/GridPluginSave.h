// Copyright Two Bit Studios 2025. All Rights Reserved.

/*
* This is the SaveGame class that stores
* all the grid related data to a .sav file
* in Saved/SaveGames. Make sure this file is copied
* into your final packaged game folder.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GridDataStructures.h"
#include "GridPluginSave.generated.h"

class AGridVisuals;

UCLASS()
class GRIDPLUGIN_API UGridPluginSave : public USaveGame
{
	GENERATED_BODY()
	

public:

	UGridPluginSave();

	UPROPERTY()
	TMap<FIntPoint, FTileData> SavedGridData;

	UPROPERTY()
	FIntPoint StartIndex;

	UPROPERTY()
	FIntPoint EndIndex;

	UPROPERTY()
	FVector2D LastGridSize;

	UPROPERTY()
	FVector2D GridSize;

	UPROPERTY()
	FVector2D GridOffset;

	UPROPERTY()
	bool bIsHex;

	UPROPERTY()
	FIntPoint TileCount;

	UPROPERTY()
	float LineWidth;

	UPROPERTY()
	float LineOpacity;

	UPROPERTY()
	float TileOpacity;

	UPROPERTY()
	FLinearColor TileColor;

	UPROPERTY()
	FLinearColor LineColor;
};