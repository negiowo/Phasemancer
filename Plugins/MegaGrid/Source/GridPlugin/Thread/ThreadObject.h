// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GridDataStructures.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HAL/Runnable.h"
#include "Delegates/Delegate.h"

#include "PathFindingComponent.h"

class FRunnableThread;
class UPathFindingComponent;

/// <summary>
/// Thread class for async pathfinding.
/// </summary>
class GRIDPLUGIN_API FPathfindingThread : public FRunnable
{
public:

	/// <summary>
	/// Flag to stop async pathfinding or thread.
	/// </summary>
	bool bStopThread;

	/// <summary>
	/// Constructor.
	/// </summary>
	/// <param name="Start"></param>
	/// <param name="Target"></param>
	/// <param name="Pathfinder"></param>
	FPathfindingThread(FIntPoint Start, FIntPoint Target, UPathFindingComponent* Pathfinder);

	virtual bool Init();

	virtual uint32 Run();

	virtual void Stop();

private:

	/// <summary>
	/// Calling component.
	/// </summary>
	UPathFindingComponent* CurrentPathFinder;

	/// <summary>
	/// Start Tile.
	/// </summary>
	FIntPoint CurrentStart;

	/// <summary>
	/// End Tile.
	/// </summary>
	FIntPoint CurrentTarget;
};
