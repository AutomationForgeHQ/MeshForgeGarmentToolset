#include "MeshForgeGarmentToolsetModule.h"

#include "MeshForgeGarmentToolset.h"
#include "ToolsetRegistry/UToolsetRegistry.h"

DEFINE_LOG_CATEGORY(LogMeshForgeGarmentToolset);

void FMeshForgeGarmentToolsetModule::StartupModule()
{
	if (!UToolsetRegistry::IsAvailable())
	{
		// Expected outside the editor, and whenever the experimental plugins are off. Not an error.
		UE_LOG(LogMeshForgeGarmentToolset, Log, TEXT("Toolset registry unavailable - Garment Studio tools not registered."));
		return;
	}

	if (UToolsetRegistry::IsToolsetClassRegistered(UMeshForgeGarmentToolset::StaticClass()))
	{
		bRegistered = true;
		return;
	}

	UToolsetRegistry::RegisterToolsetClass(UMeshForgeGarmentToolset::StaticClass());
	bRegistered = UToolsetRegistry::IsToolsetClassRegistered(UMeshForgeGarmentToolset::StaticClass());

	// Worth logging either way. A toolset nobody registers is invisible and says nothing about it.
	UE_LOG(LogMeshForgeGarmentToolset, Log, TEXT("Garment Studio toolset %s."), bRegistered ? TEXT("registered") : TEXT("failed to register"));
}

void FMeshForgeGarmentToolsetModule::ShutdownModule()
{
	if (bRegistered && UToolsetRegistry::IsAvailable())
	{
		UToolsetRegistry::UnregisterToolsetClass(UMeshForgeGarmentToolset::StaticClass());
		bRegistered = false;
	}
}

IMPLEMENT_MODULE(FMeshForgeGarmentToolsetModule, MeshForgeGarmentToolset)
