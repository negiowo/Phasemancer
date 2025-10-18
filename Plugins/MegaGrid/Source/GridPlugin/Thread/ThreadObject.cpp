// Copyright Two Bit Studios 2025. All Rights Reserved.

#include "ThreadObject.h"

FPathfindingThread::FPathfindingThread(FIntPoint Start, FIntPoint Target, UPathFindingComponent* Pathfinder)
{
	// Constructor simply sets the variables.

	CurrentStart = Start;
	CurrentTarget = Target;
	CurrentPathFinder = Pathfinder;
}

bool FPathfindingThread::Init()
{
	// Initialize flags.
	bStopThread = false;
	CurrentPathFinder->bStopPathFinding = false;
	return true;
}

uint32 FPathfindingThread::Run()
{
	// Call FindPath from the current pathfinding component.
	FPathStruct Path = CurrentPathFinder->FindPath(CurrentStart, CurrentTarget);

	TWeakObjectPtr<UPathFindingComponent> WeakPathFinder = CurrentPathFinder; // Use WeakObject to avoid corrupt pointers.

	// Switch back to the game thread to broadcast the result
	AsyncTask(ENamedThreads::GameThread, [WeakPathFinder, Path]()
		{
			// When path is complete.
			if (WeakPathFinder.IsValid() && WeakPathFinder->OnPathComplete.IsBound())
			{
				// Safely broadcast the delegate
				WeakPathFinder->OnPathComplete.Broadcast(Path);

			}
		});

	return 0;
}

void FPathfindingThread::Stop()
{
	// Reset required flags.
	bStopThread = true;
	CurrentPathFinder->bStopPathFinding = true;
}
