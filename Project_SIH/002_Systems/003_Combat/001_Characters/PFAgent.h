#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/002_Systems/003_Combat/001_Characters/PFBattleCharacterBase.h"
#include "PFAgent.generated.h"

UCLASS(Abstract, Blueprintable)
class PROJECT_SIH_API APFAgent : public APFBattleCharacterBase
{
	GENERATED_BODY()

public:
	virtual EPFBattleSide GetBattleSide() const override;
};
