#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMeshForgeGarmentToolset, Log, All);

/** Registers the Garment Studio's tools with the toolset registry. Skills are found on their own. */
class FMeshForgeGarmentToolsetModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	bool bRegistered = false;
};
