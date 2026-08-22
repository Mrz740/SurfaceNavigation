// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "Modules/ModuleManager.h"

SURFACENAVIGATIONEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogSurfaceBake, Display, All)

class FSurfaceNavigationEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
