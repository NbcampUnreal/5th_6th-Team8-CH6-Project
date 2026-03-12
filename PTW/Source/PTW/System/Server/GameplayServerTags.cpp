#include "GameplayServerTags.h"

namespace GameplayServerTags
{
	namespace GameSessionsAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ListFleets, "GameplayServerTags.GameSessionsAPI.ListFleets", "플릿 리스트");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(FindOrCreateGameSession, "GameplayServerTags.GameSessionsAPI.FindOrCreateGameSession", "탐색 또는 세션 생성");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreateGameSession, "GameplayServerTags.GameSessionsAPI.CreateGameSession", "세션 생성");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreatePlayerSession, "GameplayServerTags.GameSessionsAPI.CreatePlayerSession", "세션 생성");
	}
}
