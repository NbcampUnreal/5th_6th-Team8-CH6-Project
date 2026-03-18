// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryGameMode.h"


#include "AbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameFramework/PlayerStart.h"
#include "GAS/PTWDeliveryAttributeSet.h"
#include "MiniGame/Actor/Delivery/RaceTrack.h"
#include "MiniGame/ControllerComponent/Delivery/PTWDeliveryControllerComponent.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Debug/PTWLogCategorys.h"
#include "Kismet/GameplayStatics.h"

APTWDeliveryGameMode::APTWDeliveryGameMode()
{
}

void APTWDeliveryGameMode::StartRound()
{
	SetMiniGameRule();
	GrantDeliveryAttributeSet();
	GetWorld()->GetTimerManager().SetTimer(RankingTimerHandle, this, &APTWDeliveryGameMode::UpdateAllPlayerRanks, 0.1f, true);
	Super::StartRound();
}

void APTWDeliveryGameMode::GiveDeliveryItems(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply)
{
	if (!TargetCharacter) return;
	if (DeliveredCharacters.Contains(TargetCharacter->GetController())) return;
	
	ApplyGameEffect(TargetCharacter, EffectToApply);
	GivingDefaultWeapon(TargetCharacter);
	DeliveryUISetting(TargetCharacter);
	DeliveredCharacters.AddUnique(TargetCharacter->GetController());
}

void APTWDeliveryGameMode::GoalPlayer(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply)
{
	if (GoalPlayers.Num() == 0)
	{
		StartEndCountDown();
	}
	GoalPlayers.Add(TargetCharacter);
	
	ApplyGameEffect(TargetCharacter, EffectToApply);
}

void APTWDeliveryGameMode::SetPlayerSpawnLocation(APTWPlayerController* PC, FVector NewLocation)
{
	PlayerSpawnPoints.FindOrAdd(PC) = NewLocation;
}

FTransform APTWDeliveryGameMode::GetPlayerSpawnTransform(APTWPlayerController* PC)
{
	if (FVector* SpawnLoc = PlayerSpawnPoints.Find(PC))
	{
		return FTransform(FRotator::ZeroRotator, *SpawnLoc);
	}
	
	return FTransform();
}

void APTWDeliveryGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	APTWPlayerCharacter* TargetCharacter = Cast<APTWPlayerCharacter>(KillActor);
	ApplyGameEffect(TargetCharacter, KillBonusEffect);
	
	APTWPlayerCharacter* DeadCharacter = Cast<APTWPlayerCharacter>(DeadActor);
	
	IPTWCombatInterface* DeadCombatInterface = CastToPTWCombatInterface(DeadCharacter);
	if (!DeadCombatInterface) return;
	
	DeadCombatInterface->RemoveEffectWithTag(GameplayTags::MiniGame::Delivery);
	
	Super::HandlePlayerDeath(DeadActor, KillActor);
}

void APTWDeliveryGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	
	APTWPlayerCharacter* TargetCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn());
	if (!TargetCharacter) return;
	
	if (CheckingDeadPlayer(NewPlayer))
	{
		IPTWCombatInterface* CombatInterface = CastToPTWCombatInterface(TargetCharacter);
		if (!CombatInterface) return;
		CombatInterface->ApplyGameplayEffectToSelf(RestartPlayerEffect, 1.0f, FGameplayEffectContextHandle());
	}
}

void APTWDeliveryGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (!RaceTrackSpline)
	{
		RaceTrackSpline = Cast<ARaceTrack>(UGameplayStatics::GetActorOfClass(GetWorld(), ARaceTrack::StaticClass()));
	}
}

void APTWDeliveryGameMode::ApplyGameEffect(APTWPlayerCharacter* Target, TSubclassOf<UGameplayEffect> TargetGameplayEffect)
{
	IPTWCombatInterface* CombatInterface = CastToPTWCombatInterface(Target);
	if (!CombatInterface) return;
	CombatInterface->ApplyGameplayEffectToSelf(TargetGameplayEffect, 1.0f, FGameplayEffectContextHandle());
}

void APTWDeliveryGameMode::StartEndCountDown()
{
	for (APTWPlayerController* PC : RankPCList)
	{
		UPTWDeliveryControllerComponent* DeliveryComp = Cast<UPTWDeliveryControllerComponent>(PC->GetControllerComponent());
		if (DeliveryComp)
		{
			DeliveryComp->ShowCountDownWidget();
		}
	}

	UpdateCountDown();
	GetWorld()->GetTimerManager().SetTimer(CountDownTimerHandle, this, &APTWDeliveryGameMode::UpdateCountDown,1.0f,true);
}

void APTWDeliveryGameMode::UpdateCountDown()
{
	if (FinalCount == 0)
	{
		GiveRoundScore();
		StopCountDown();
	}
	
	for (APTWPlayerController* PC : RankPCList)
	{
		UPTWDeliveryControllerComponent* DeliveryComp = Cast<UPTWDeliveryControllerComponent>(PC->GetControllerComponent());
		if (DeliveryComp)
		{
			DeliveryComp->SetCountDownText(FinalCount);
		}
	}
	
	FinalCount--;
}

void APTWDeliveryGameMode::StopCountDown()
{
	GetWorld()->GetTimerManager().ClearTimer(CountDownTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(RankingTimerHandle);
	
	FTimerHandle EndTimerWaitHandle;
	GetWorld()->GetTimerManager().SetTimer(EndTimerWaitHandle, this, &APTWDeliveryGameMode::EndTimer, 3.0f, false);
}

bool APTWDeliveryGameMode::CheckingDeadPlayer(AController* NewPlayer)
{
	APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	check(PTWPlayerState);
	
	return PTWPlayerState->GetPlayerRoundData().DeathCount != 0;
}

void APTWDeliveryGameMode::GiveRoundScore()
{
	int32 FirstScore = 6;
	for (int32 i = 0; i < RankPCList.Num(); i++)
	{
		UPTWDeliveryControllerComponent* DeliveryComp = Cast<UPTWDeliveryControllerComponent>(RankPCList[i]->GetControllerComponent());
		if (!DeliveryComp) return;
		
		AddRoundScore(RankPCList[i]->GetPlayerState<APlayerState>(), FirstScore);
		
		if (FirstScore > 2)
		{
			FirstScore--;
		}
	}
}

IPTWCombatInterface* APTWDeliveryGameMode::CastToPTWCombatInterface(APTWPlayerCharacter* PlayerCharacter)
{
	IPTWCombatInterface* PTWCombatInterface = Cast<IPTWCombatInterface>(PlayerCharacter);
	return PTWCombatInterface;
}

AActor* APTWDeliveryGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	APTWPlayerController* PC = Cast<APTWPlayerController>(Player);
	
	if (PC && PlayerSpawnPoints.Contains(PC))
	{
		if (!SharedCheckPointStart)
		{
			SharedCheckPointStart = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass());
			
			if (SharedCheckPointStart && SharedCheckPointStart->GetRootComponent())
			{
				SharedCheckPointStart->GetRootComponent()->SetMobility(EComponentMobility::Movable);
			}
		}
		
		SharedCheckPointStart->SetActorLocation(PlayerSpawnPoints[PC]);
		return SharedCheckPointStart;
	}
	
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

float APTWDeliveryGameMode::GetDistanceForActor(AActor* TargetActor)
{
	if (RaceTrackSpline && RaceTrackSpline->GetSplineComponent())
	{
		USplineComponent* Spline = RaceTrackSpline->GetSplineComponent();
		FVector ActorLoc = TargetActor->GetActorLocation();
		
		float Key = Spline->FindInputKeyClosestToWorldLocation(ActorLoc);
		float Distance = Spline->GetDistanceAlongSplineAtSplineInputKey(Key);
		
		return Distance;
	}
	
	return 0.0f;
}

void APTWDeliveryGameMode::UpdateAllPlayerRanks()
{
	RankPCList.Empty();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			UPTWDeliveryControllerComponent* DeliveryControllerComp =  Cast<UPTWDeliveryControllerComponent>(PC->GetControllerComponent());
			if (!DeliveryControllerComp) return;
			
			if (PC->GetPawn())
			{
				DeliveryControllerComp->SetTraveledDistance(GetDistanceForActor(PC->GetPawn()));
			}
			RankPCList.Add(PC);
		}
	}
	
	RankPCList.Sort([](const APTWPlayerController& A, const APTWPlayerController& B) 
	{
		UPTWDeliveryControllerComponent* DeliveryCompA = Cast<UPTWDeliveryControllerComponent>(A.GetControllerComponent());
		UPTWDeliveryControllerComponent* DeliveryCompB = Cast<UPTWDeliveryControllerComponent>(B.GetControllerComponent());
		return DeliveryCompA->GetTraveledDistance() < DeliveryCompB->GetTraveledDistance();
	});

	// 3. 등수 부여
	for (int32 i = 0; i < RankPCList.Num(); ++i)
	{
		UPTWDeliveryControllerComponent* DeliveryControllerComp =  Cast<UPTWDeliveryControllerComponent>(RankPCList[i]->GetControllerComponent());
		if (!DeliveryControllerComp) return;
		DeliveryControllerComp->MyCurrentRank = i + 1;
	}
	
}

void APTWDeliveryGameMode::GivingDefaultWeapon(APTWPlayerCharacter* TargetCharacter)
{
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;
	
	SpawnManager->SpawnSingleItem(TargetCharacter->GetPlayerState<APTWPlayerState>(), DeliveryDefaultWeapon);
}

void APTWDeliveryGameMode::SetMiniGameRule()
{
	MiniGameRule.TimeRule.Timer = 180;
	MiniGameRule.KillRule.KillScore = 0;
	MiniGameRule.SpawnRule.RespawnDelay = 5.0f;
}

void APTWDeliveryGameMode::GrantDeliveryAttributeSet()
{
	for (APlayerState* AS : PTWGameState->AlivePlayers)
	{
		APTWPlayerState* PS = Cast<APTWPlayerState>(AS);
		if (!PS) return;
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		if (!ASC) return;
		
		if (ASC->GetAttributeSet(UPTWDeliveryAttributeSet::StaticClass())) return;
		
		UPTWDeliveryAttributeSet* NewSet = NewObject<UPTWDeliveryAttributeSet>(AS->GetPawn());
		ASC->AddSpawnedAttribute(NewSet);
		InitializeAttributeSet(ASC);
	}
}

void APTWDeliveryGameMode::InitializeAttributeSet(UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC) return;
	TargetASC->SetNumericAttributeBase(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute(), 1.0f);
	TargetASC->SetNumericAttributeBase(UPTWDeliveryAttributeSet::GetChargeSpeedAttribute(), 1.0f);
	float MaxValue = TargetASC->GetNumericAttribute(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute());
	TargetASC->SetNumericAttributeBase(UPTWDeliveryAttributeSet::GetBatteryLevelAttribute(), MaxValue);
}

void APTWDeliveryGameMode::DeliveryUISetting(APTWPlayerCharacter* TargetCharacter)
{
	if (APTWPlayerController* PlayerController = Cast<APTWPlayerController>(TargetCharacter->GetController()))
	{
		UPTWDeliveryControllerComponent* DeliveryComp = Cast<UPTWDeliveryControllerComponent>(PlayerController->GetControllerComponent());
		
		if (DeliveryComp)
		{
			DeliveryComp->AddBatteryUI();
		}
	}
}
