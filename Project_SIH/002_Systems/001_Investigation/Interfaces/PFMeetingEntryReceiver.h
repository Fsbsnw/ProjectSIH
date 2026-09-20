// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Project_SIH/000_Core/001_Contracts/000_Flow/PFGameFlowTypes.h"
#include "UObject/Interface.h"
#include "PFMeetingEntryReceiver.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPFMeetingEntryReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class PROJECT_SIH_API IPFMeetingEntryReceiver
{
	GENERATED_BODY()

public:
	virtual EPFPhaseStartResult StartMeeting(const FPFMeetingEntryContext& Context) = 0;
};
