#pragma once
#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace GameplayServerTags
{
	namespace GameSessionsAPI
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ListFleets);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(FindOrCreateGameSession);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(CreateGameSession);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(CreatePlayerSession);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(DescribeGameSession);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SearchGameSessions);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(CheckSessionStatus);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(UpdateGameSession);
	}
}
