// How to fit a garment by looking at it: what no tool signature can say.

#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/AgentSkill.h"
#include "GarmentStudioSkill.generated.h"

/**
 * The loop an agent with vision follows in the Garment Studio, and what to look for in each picture.
 *
 * Native, so it is discovered automatically - no registration. Says nothing a tool description already says.
 */
UCLASS()
class MESHFORGEGARMENTTOOLSET_API UGarmentStudioSkill : public UAgentSkill
{
	GENERATED_BODY()

public:

	UGarmentStudioSkill();
};
