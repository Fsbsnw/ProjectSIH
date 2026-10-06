// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Project_SIH/000_Core/001_Contracts/003_Battle/PFCombatProfileTypes.h"
#include "PFCharacterDefinition.generated.h"

class UCurveTable;
class UTexture2D;

/**
 *
 */
UCLASS()
class PROJECT_SIH_API UPFCharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetId MakePrimaryAssetID(const FGameplayTag& CharacterID);

	const FPFCombatProfile& GetDefaultCombatProfile() const;

	const UCurveTable* GetLevelBonusTable() const;

	/** UI에 표시할 캐릭터 이름을 반환합니다. */
	const FText& GetDisplayName() const { return m_DisplayName; }

	/** UI에서 필요할 때 로드할 캐릭터 아이콘 참조를 반환합니다. */
	const TSoftObjectPtr<UTexture2D>& GetIcon() const { return m_Icon; }

#if WITH_EDITOR
	bool SetDefinitionTagForEditorImport(const FGameplayTag& InDefinitionTag);

	void SetPresentationDataForEditorImport(
		const FText& InDisplayName,
		const TSoftObjectPtr<UTexture2D>& InIcon);
#endif

private:
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Character ID",
			Categories = "ID.Character",
			ToolTip = "캐릭터를 식별하는 고유 GameplayTag입니다. ID.Character.*의 형태입니다"))
	FGameplayTag m_DefinitionTag;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Definition|Battle",
		meta = (
			DisplayName = "Default Combat Profile",
			ToolTip = "플레이어블 전투 캐릭터의 기본 전투 구성을 정의합니다."))
	FPFCombatProfile m_DefaultCombatProfile;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Definition|Stat",
		meta = (
			DisplayName = "Level Bonus Table",
			ToolTip = "현재 레벨에 적용할 누적 능력치 보너스를 정의합니다"))
	TObjectPtr<UCurveTable> m_LevelBonusTable;

	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition|Presentation",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Display Name",
			ToolTip = "캐릭터의 표시 이름입니다"))
	FText m_DisplayName;

	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Definition|Presentation",
		meta = (AllowPrivateAccess = "true",
			DisplayName = "Icon",
			ToolTip = "캐릭터 UI에 사용하는 아이콘입니다"))
	TSoftObjectPtr<UTexture2D> m_Icon;
};
