#include "GameplayServerTags.h"

namespace GameplayServerTags
{
	namespace GameSessionsAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ListFleets, "GameplayServerTags.GameSessionsAPI.ListFleets", "플릿 리스트");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(FindOrCreateGameSession, "GameplayServerTags.GameSessionsAPI.FindOrCreateGameSession", "탐색 또는 세션 생성");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreateGameSession, "GameplayServerTags.GameSessionsAPI.CreateGameSession", "게임 세션 생성");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreatePlayerSession, "GameplayServerTags.GameSessionsAPI.CreatePlayerSession", "플레이어 세션 생성");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(DescribeGameSession, "GameplayServerTags.GameSessionsAPI.DescribeGameSession", "세션 정보");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SearchGameSessions, "GameplayServerTags.GameSessionsAPI.SearchGameSessions", "세션 검색");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CheckSessionStatus, "GameplayServerTags.GameSessionsAPI.CheckSessionStatus", "세션 상태 탐색");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(UpdateGameSession, "GameplayServerTags.GameSessionsAPI.UpdateGameSession", "스팀Id 업데이트");
	}
}
