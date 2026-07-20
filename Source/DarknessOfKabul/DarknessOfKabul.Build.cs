using UnrealBuildTool;

public class DarknessOfKabul : ModuleRules
{
	public DarknessOfKabul(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG"
		});

		// Add private dependencies only when an implementation needs them.
	}
}
