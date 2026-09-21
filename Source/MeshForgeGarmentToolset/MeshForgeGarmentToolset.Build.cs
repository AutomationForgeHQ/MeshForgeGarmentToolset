using UnrealBuildTool;

public class MeshForgeGarmentToolset : ModuleRules
{
	public MeshForgeGarmentToolset(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"ToolsetRegistry",     // UToolsetDefinition and UAgentSkill are public base classes
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"MeshForge",           // the definition the fit step lives on
				"MeshForgeGarment",    // the capability this exposes: FGarmentStudio
				"Projects",            // IPluginManager, so GetToolsetVersion() reads the descriptor
				"UnrealEd",            // RaiseScriptError comes from KismetSystemLibrary
			}
			);

		// As with every other toolset in this family: no dependency on ModelContextProtocol. Tools
		// register with ToolsetRegistry and MCP picks them up from there.
	}
}
