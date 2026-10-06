#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

#if UE_BUILD_SHIPPING || UE_BUILD_TEST

#define PF_LOG_EX(Key, bShowOnScreen, Format, ...) \
	do \
	{ \
		const FString PFLogBody = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG( \
			LogTemp, \
			Warning, \
			TEXT("[%s:%d] %s"), \
			ANSI_TO_TCHAR(__FUNCTION__), \
			__LINE__, \
			*PFLogBody); \
	} while (false)

#else

#define PF_LOG_EX(Key, bShowOnScreen, Format, ...) \
	do \
	{ \
		const FString PFLogBody = FString::Printf(Format, ##__VA_ARGS__); \
		const FString PFLogMessage = FString::Printf( \
			TEXT("[%s:%d] %s"), \
			ANSI_TO_TCHAR(__FUNCTION__), \
			__LINE__, \
			*PFLogBody); \
		UE_LOG(LogTemp, Warning, TEXT("%s"), *PFLogMessage); \
		const bool bPFShowOnScreen = (bShowOnScreen); \
		if (bPFShowOnScreen && GEngine) \
		{ \
			const int32 PFLogKey = (Key); \
			GEngine->AddOnScreenDebugMessage(PFLogKey, 5.0f, FColor::Cyan, PFLogMessage); \
		} \
	} while (false)

#endif

#define PF_LOG(Format, ...) \
	PF_LOG_EX(INDEX_NONE, true, Format, ##__VA_ARGS__)
