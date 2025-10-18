// Copyright Two Bit Studios 2025. All Rights Reserved.

#if WITH_EDITOR

#pragma once

#include "Modules/ModuleManager.h"
#include "Containers/Array.h"

class ICustomModuleListenerInterface
{
public:
	virtual void OnStartupModule() {};
	virtual void OnShutdownModule() {};
};

class ICustomModuleInterface : public IModuleInterface
{
public:
	void StartupModule() override
	{
		if (!IsRunningCommandlet())
		{
			AddModuleListeners();
			for (int32 i = 0; i < ModuleListeners.Num(); ++i)
			{
				ModuleListeners[i]->OnStartupModule();
			}
		}
	}

	void ShutdownModule() override
	{
		for (int32 i = 0; i < ModuleListeners.Num(); ++i)
		{
			ModuleListeners[i]->OnShutdownModule();
		}
	}

	virtual void AddModuleListeners() {};

protected:
	TArray<TSharedRef<ICustomModuleListenerInterface>> ModuleListeners;
};

#endif