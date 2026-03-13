// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCopsAndRobbersGameMode.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Algo/RandomShuffle.h"
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
	
	for (int32 i = PTWGameState->GetTeams().Num(); i < MiniGameRule.TeamRule.NumTeams; ++i)
	{
		FPTWTeamInfo NewTeam;
		NewTeam.TeamID = i;
		PTWGameState->GetTeams().Add(NewTeam);
	}
	
	FPTWTeamInfo& RobbersTeam = PTWGameState->GetTeams()[ROBBERS];
	FPTWTeamInfo& CopsTeam = PTWGameState->GetTeams()[COPS];

	RobbersTeam.Members.Empty();
	CopsTeam.Members.Empty();
	
	TArray<APlayerState*> ValidPlayers;
	for (APlayerState* PS : PTWGameState->AlivePlayers)
	{
		if (IsValid(PS)) 
		{
			ValidPlayers.Add(PS);
		}
	}

	if (ValidPlayers.Num() == 0) return;
	Algo::RandomShuffle(ValidPlayers);

	const int32 MaxCopsSize = FMath::CeilToInt(ValidPlayers.Num() / 4.0f);
	
	CopsTeam.Members.Reserve(MaxCopsSize);
	RobbersTeam.Members.Reserve(ValidPlayers.Num() - MaxCopsSize);
	
	for (int32 Index = 0; Index < ValidPlayers.Num(); ++Index)
	{
		APlayerState* PS = ValidPlayers[Index];

		const bool bAssignToCops = (Index < MaxCopsSize);
   
		FPTWTeamInfo& TargetTeam = bAssignToCops ? CopsTeam : RobbersTeam;
		const int32 TargetTeamId = bAssignToCops ? COPS : ROBBERS;
    
		TargetTeam.Members.Add(PS);
    
		if (IPTWPlayerRoundDataInterface* RoundData = Cast<IPTWPlayerRoundDataInterface>(PS))
		{
			RoundData->SetTeamId(TargetTeamId);
		}
	}

	
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


