// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "Modules/ModuleManager.h"

class FSurfaceNavigationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
