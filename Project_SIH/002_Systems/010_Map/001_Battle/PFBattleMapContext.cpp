#include "PFBattleMapContext.h"

#include "Camera/CameraActor.h"
#include "Engine/TargetPoint.h"
#include "Project_SIH/000_Core/001_Contracts/002_Party/PFPartyTypes.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFBattleTypes.h"
#include "Project_SIH/000_Core/001_Diagnostics/PFDebugMacros.h"

APFBattleMapContext::APFBattleMapContext()
{
	m_AllySpawnPoints.SetNum(
		FPFPartyData::RequiredMemberCount);
}

bool APFBattleMapContext::TryGetBattleSpawnLayout(
	FPFBattleSpawnLayout& OutLayout) const
{
	OutLayout = FPFBattleSpawnLayout();

	FPFBattleSpawnLayout SpawnLayout;
	TSet<const ATargetPoint*> UsedSpawnPoints;

	if (!TryGetAllySpawnTransforms(
		SpawnLayout.m_AllyTransforms,
		UsedSpawnPoints))
	{
		return false;
	}

	if (!TryResolveSpawnTransform(
		m_BossSpawnPoint.Get(),
		TEXT("Boss"),
		UsedSpawnPoints,
		SpawnLayout.m_BossTransform))
	{
		return false;
	}

	if (!SpawnLayout.IsValid())
	{
		PF_LOG(TEXT("Battle spawn layout is invalid"));
		return false;
	}

	OutLayout = MoveTemp(SpawnLayout);
	return true;
}

bool APFBattleMapContext::TryGetBattleCamera(
	ACameraActor*& OutCamera) const
{
	OutCamera = m_BattleCamera.Get();

	if (!IsValid(OutCamera))
	{
		PF_LOG(TEXT("Battle camera is not assigned"));
		return false;
	}

	return true;
}

bool APFBattleMapContext::TryGetAllySpawnTransforms(
	TArray<FTransform>& OutTransforms,
	TSet<const ATargetPoint*>& OutUsedSpawnPoints) const
{
	OutTransforms.Reset();
	OutUsedSpawnPoints.Reset();

	if (m_AllySpawnPoints.Num()
		!= FPFPartyData::RequiredMemberCount)
	{
		PF_LOG(
			TEXT(
				"Ally spawn point count is invalid. "
				"Expected=%d, Actual=%d"),
			FPFPartyData::RequiredMemberCount,
			m_AllySpawnPoints.Num());
		return false;
	}

	for (int32 Index = 0;
		Index < m_AllySpawnPoints.Num();
		++Index)
	{
		FTransform SpawnTransform;

		if (!TryResolveSpawnTransform(
			m_AllySpawnPoints[Index].Get(),
			FString::Printf(
				TEXT("Ally[%d]"),
				Index),
			OutUsedSpawnPoints,
			SpawnTransform))
		{
			return false;
		}

		OutTransforms.Add(SpawnTransform);
	}

	return true;
}

bool APFBattleMapContext::TryResolveSpawnTransform(
	const ATargetPoint* SpawnPoint,
	const FString& SpawnPointLabel,
	TSet<const ATargetPoint*>& InOutUsedSpawnPoints,
	FTransform& OutTransform) const
{
	OutTransform = FTransform::Identity;

	if (!IsValid(SpawnPoint))
	{
		PF_LOG(
			TEXT("%s spawn point is invalid"),
			*SpawnPointLabel);
		return false;
	}

	if (InOutUsedSpawnPoints.Contains(SpawnPoint))
	{
		PF_LOG(
			TEXT(
				"%s spawn point is duplicated. "
				"SpawnPoint=%s"),
			*SpawnPointLabel,
			*GetNameSafe(SpawnPoint));
		return false;
	}

	const FTransform SpawnTransform =
		SpawnPoint->GetActorTransform();

	if (!SpawnTransform.IsValid())
	{
		PF_LOG(
			TEXT(
				"%s spawn transform is invalid. "
				"SpawnPoint=%s"),
			*SpawnPointLabel,
			*GetNameSafe(SpawnPoint));
		return false;
	}

	InOutUsedSpawnPoints.Add(SpawnPoint);
	OutTransform = SpawnTransform;

	return true;
}
