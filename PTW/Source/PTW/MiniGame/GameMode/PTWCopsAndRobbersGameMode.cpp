// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCopsAndRobbersGameMode.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"


APTWCopsAndRobbersGameMode::APTWCopsAndRobbersGameMode()
{
	MiniGameRule.TimeRule.bUseTimer = true;
	MiniGameRule.TimeRule.Round = 1;
	MiniGameRule.TimeRule.Timer = 150.0f;
	
	MiniGameRule.TimeRule.bUseCountDown = true;
	MiniGameRule.TimeRule.CountDown = 10.0f;
	
	MiniGameRule.SpawnRule.bUseRespawn = false;
	
	MiniGameRule.CombatRule.bAllowGun = true;
	MiniGameRule.CombatRule.bAllowMelee = true;
	
	MiniGameRule.WinConditionRule.WinType = EPTWWinType::Survival;
	MiniGameRule.WinConditionRule.OvertimeRule = EPTWOvertimeRule::None;
	
	MiniGameRule.TeamRule.bUseTeam = true;
	MiniGameRule.TeamRule.NumTeams = 2;
	
}

void APTWCopsAndRobbersGameMode::StartGame()
{
	Super::StartGame();
	
	
}

void APTWCopsAndRobbersGameMode::WaitingToStartRound()
{
	Super::WaitingToStartRound();

	for (APlayerState* Player : PTWGameState->AlivePlayers)
	{
		if (APTWPlayerState* PTWPlayer = Cast<APTWPlayerState>(Player))
		{
			if (UAbilitySystemComponent* PlayerASC = PTWPlayer->GetAbilitySystemComponent())
			{
				FGameplayAbilitySpec AbilitySpec(GA_Blind, 1, INDEX_NONE, this);
				PlayerASC->GiveAbility(AbilitySpec);
				
				PlayerASC->TryActivateAbilityByClass(GA_Blind);
				UE_LOG(LogTemp, Display, TEXT("WaitingToStartRound: %s"), *PTWPlayer->GetPlayerName());
			}
		}
	}
}


