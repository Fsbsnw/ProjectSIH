#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/002_Systems/003_Combat/Interfaces/PFBattleLayoutProvider.h"
#include "Project_SIH/002_Systems/010_Map/PFMapContext.h"
#include "PFBattleMapContext.generated.h"

class ACameraActor;
class ATargetPoint;

UCLASS(Blueprintable)
class PROJECT_SIH_API APFBattleMapContext
	: public APFMapContext
	, public IPFBattleLayoutProvider
{
	GENERATED_BODY()

public:
	APFBattleMapContext();

	virtual bool TryGetBattleSpawnLayout(
		FPFBattleSpawnLayout& OutLayout) const override;

	bool TryGetBattleCamera(
		ACameraActor*& OutCamera) const override;

private:
	bool TryGetAllySpawnTransforms(
		TArray<FTransform>& OutTransforms,
		TSet<const ATargetPoint*>& OutUsedSpawnPoints) const;

	bool TryResolveSpawnTransform(
		const ATargetPoint* SpawnPoint,
		const FString& SpawnPointLabel,
		TSet<const ATargetPoint*>& InOutUsedSpawnPoints,
		FTransform& OutTransform) const;

private:
	UPROPERTY(
		EditInstanceOnly,
		Category = "Map Context|Battle",
		meta = (
			DisplayName = "Ally Spawn Points",
			EditFixedSize))
	TArray<TObjectPtr<ATargetPoint>> m_AllySpawnPoints;

	UPROPERTY(
		EditInstanceOnly,
		Category = "Map Context|Battle",
		meta = (DisplayName = "Boss Spawn Point"))
	TObjectPtr<ATargetPoint> m_BossSpawnPoint;

	UPROPERTY(
		EditInstanceOnly,
		Category = "Map Context|Battle",
		meta = (DisplayName = "Battle Camera"))
	TObjectPtr<ACameraActor> m_BattleCamera;
};
