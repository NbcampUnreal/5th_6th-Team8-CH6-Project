// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCopsAndRobbersGameMode.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/Actor/CopsAndRobbers/PTWCitizenSpawner.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "System/PTWItemSpawnManager.h"


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

void APTWCopsAndRobbersGameMode::EndGame()
{
	Super::EndGame();
}

void APTWCopsAndRobbersGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
	// if (APlayerController* PC = Cast<APlayerController>(C))
	// {
	// 	PlayerReadyToPlay(PC);
	// }
}

void APTWCopsAndRobbersGameMode::AssignTeam()
{
	Super::AssignTeam();
	// ROBBERS : COPS is 3 : 1 (75% : 25%)
	
}

void APTWCopsAndRobbersGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	Super::HandlePlayerDeath(DeadActor, KillActor);
	if (!IsValid(DeadActor) || !IsValid(KillActor)) return;
	
	IAbilitySystemInterface* VictimAS = Cast<IAbilitySystemInterface>(DeadActor);
	if (!VictimAS) return;
	UAbilitySystemComponent* VictimASC = VictimAS->GetAbilitySystemComponent();
	if (!IsValid(VictimASC)) return;
	
	IAbilitySystemInterface* AttackerAS = Cast<IAbilitySystemInterface>(KillActor);
	if (!AttackerAS) return;
	UAbilitySystemComponent* AttackerASC = AttackerAS->GetAbilitySystemComponent();
	if (!IsValid(AttackerASC)) return;
	
	if (VictimASC->HasMatchingGameplayTag(GameplayTags::Role::Citizen))
	{
		// 공격자 경찰에게 리바운드 피해 GE 재생
		check(ReboundGameplayEffect);

		FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
		Context.AddSourceObject(AttackerASC->GetAvatarActor());
		
		FGameplayEffectSpecHandle ReboundSpecHandle =
			AttackerASC->MakeOutgoingSpec(ReboundGameplayEffect, 1.0f, Context);
		if (ReboundSpecHandle.IsValid())
		{
			AttackerASC->ApplyGameplayEffectSpecToSelf(*ReboundSpecHandle.Data.Get());
		}
	}
}

void APTWCopsAndRobbersGameMode::WaitingToStartRound()
{
	Super::WaitingToStartRound();
	
	TArray<APlayerState*> Players = PTWGameState->PlayerArray;
	FPTWTeamInfo& RobbersTeam = PTWGameState->GetTeams()[ROBBERS];
	FPTWTeamInfo& CopsTeam = PTWGameState->GetTeams()[COPS];
	
	TArray<APlayerState*> AllPlayers;
	AllPlayers.Append(RobbersTeam.Members);
	AllPlayers.Append(CopsTeam.Members);
	
	RobbersTeam.Members.Empty();
	CopsTeam.Members.Empty();
	const int32 TotalPlayers = AllPlayers.Num();
	if (TotalPlayers > 0)
	{
		for (int32 i = TotalPlayers - 1; i > 0; --i)
		{
			int32 RandomIndex = FMath::RandRange(0, i);
			AllPlayers.Swap(i, RandomIndex);
		}
	}
	
	int32 MaxCopsSize = FMath::CeilToInt(TotalPlayers / 4.0f);
	for (int32 i = 0; i < TotalPlayers; ++i)
	{
		APlayerState* PS = AllPlayers[i];
		if (IsValid(PS)) 
		{
			IPTWPlayerRoundDataInterface* RoundData = Cast<IPTWPlayerRoundDataInterface>(PS);

			if (i < MaxCopsSize)
			{
				CopsTeam.Members.Add(PS);
				if (RoundData)
				{
					RoundData->SetTeamId(COPS);
				}
			}
			else
			{
				RobbersTeam.Members.Add(PS);
				if (RoundData)
				{
					RoundData->SetTeamId(ROBBERS);
				}
			}
		}
	}
	
	// FPTWTeamInfo& RobbersTeam = PTWGameState->GetTeams()[ROBBERS];
	// FPTWTeamInfo& CopsTeam = PTWGameState->GetTeams()[COPS];
	
	for (APlayerState* Robber : RobbersTeam.Members)
	{
		UAbilitySystemComponent* ASC = CastChecked<IAbilitySystemInterface>(Robber)->GetAbilitySystemComponent();
		check(ASC);
		
		ASC->AddLooseGameplayTag(GameplayTags::Role::Robber, 1, EGameplayTagReplicationState::TagAndCountToAll);
		UE_LOG(LogTemp, Display, TEXT("Robber: %s"), *Robber->GetPlayerName());
	}
	
	for (APlayerState* Cop : CopsTeam.Members)
	{
		UAbilitySystemComponent* ASC = CastChecked<IAbilitySystemInterface>(Cop)->GetAbilitySystemComponent();
		check(ASC);
		
		ASC->AddLooseGameplayTag(GameplayTags::Role::Cop, 1, EGameplayTagReplicationState::TagAndCountToAll);
		UE_LOG(LogTemp, Display, TEXT("Cop: %s"), *Cop->GetPlayerName());
		
		check(BlindGameplayEffect);
		
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(ASC);
		
		FGameplayEffectSpecHandle BlindSpecHandle =
			ASC->MakeOutgoingSpec(BlindGameplayEffect, 1.0f, Context);
		if (BlindSpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*BlindSpecHandle.Data.Get());
		}
		
		check(CopsWeaponDefinition);
		UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
		check(SpawnManager);
		
		APTWPlayerState* PTWPS = CastChecked<APTWPlayerState>(Cop);
		SpawnManager->SpawnSingleItem(PTWPS, CopsWeaponDefinition);
	}
}


