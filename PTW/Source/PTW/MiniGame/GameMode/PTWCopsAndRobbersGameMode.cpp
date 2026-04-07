// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCopsAndRobbersGameMode.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Algo/RandomShuffle.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/ControllerComponent/CopsAndRobbers/PTWCARControllerComponent.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "System/PTWItemSpawnManager.h"

#define LOCTEXT_NAMESPACE "COPSANDROBBERSGAMEMODE"
APTWCopsAndRobbersGameMode::APTWCopsAndRobbersGameMode()
{
	MiniGameRule.TimeRule.bUseTimer = true;
	MiniGameRule.TimeRule.Round = 1;
	MiniGameRule.TimeRule.Timer = 120.0f;
	
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

void APTWCopsAndRobbersGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APTWCopsAndRobbersGameMode::StartGame()
{
	Super::StartGame();
}

void APTWCopsAndRobbersGameMode::EndGame()
{
	Super::EndGame();
}

void APTWCopsAndRobbersGameMode::EndTimer()
{
	if (bIsGameEnded) return;
	
	// 타이머가 종료되면 도둑팀이 승리
	PTWGameState->SetWinTeamId(ROBBERS);
	
	Super::EndTimer();
}

void APTWCopsAndRobbersGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
}

void APTWCopsAndRobbersGameMode::AssignTeam()
{
	Super::AssignTeam();
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
		
		if (APlayerController* PC = Robber->GetPlayerController())
		{
			if (APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC))
			{
				PTWPC->SendMessage(LOCTEXT("CAR_RobbersStart", "당신은 도둑입니다. 경찰을 피해 시민들 사이에 섞여 숨으세요."),ENotificationPriority::High,10.f);
			}
		}
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
		
		APTWPlayerCharacter* PlayerCharacter = Cop->GetPawn<APTWPlayerCharacter>();
		check(PlayerCharacter);
		SpawnManager->SpawnWeaponActor(PlayerCharacter, CopsWeaponDefinition, CopsWeaponDefinition->WeaponTag);;
		
		if (APlayerController* PC = Cop->GetPlayerController())
		{
			if (APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC))
			{
				PTWPC->SendMessage(LOCTEXT("CAR_CopsStart", "당신은 경찰입니다. 시민 사이에 숨은 도둑들을 검거하세요."),ENotificationPriority::High,10.f);
			}
		}
	}
	
	// 경찰은 도둑들의 PlayerNameTagWidget을 볼 수 없도록 Widget을 파괴.
	// 경찰은 도둑과 AI를 구분할 수 없도록 하기 위함.
	for (APlayerState* Cop : CopsTeam.Members)
	{
		if (APlayerController* PC = Cop->GetPlayerController())
		{
			if (APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC))
			{
				if (UPTWCARControllerComponent* ControllerComponent = Cast<UPTWCARControllerComponent>(PTWPC->GetControllerComponent()))
				{
					for (APlayerState* Robber : RobbersTeam.Members)
					{
						ControllerComponent->ClientRPC_TargetDestroyNameTag(Robber);
					}
				}
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE
