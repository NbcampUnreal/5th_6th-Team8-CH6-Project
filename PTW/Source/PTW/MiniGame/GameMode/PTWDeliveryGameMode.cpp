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
#include "Gameplay/Actor/PTWResultCharacter.h"
#include "GAS/PTWDeliveryAttributeSet.h"
#include "MiniGame/Actor/Delivery/RaceTrack.h"
#include "MiniGame/ControllerComponent/Delivery/PTWDeliveryControllerComponent.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/Actor/Delivery/StartBlockActor.h"
#include "MiniGame/Data/Delivery/PTWRandomItemBoxData.h"


APTWDeliveryGameMode::APTWDeliveryGameMode()
{
}

void APTWDeliveryGameMode::StartRound()
{
	SetMiniGameRule();
	GrantDeliveryAttributeSet();
	RemoveBeginGameplayEffect();
	GetWorld()->GetTimerManager().SetTimer(RankingTimerHandle, this, &APTWDeliveryGameMode::UpdateAllPlayerRanks, 0.1f, true);
	StartBlocker->HideActor();

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
FRandomItemBoxData APTWDeliveryGameMode::GetRandomItemRowFromTable()

{
	FRandomItemBoxData OutRow;
    
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is null!"));
		return OutRow;
	}

	static const FString ContextString(TEXT("Random Item Context"));
	TArray<FRandomItemBoxData*> AllRows;
	ItemDataTable->GetAllRows<FRandomItemBoxData>(ContextString, AllRows);

	if (AllRows.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, AllRows.Num() - 1);
		if (AllRows[RandomIndex])
		{
			// 찾은 데이터를 OutRow에 복사해서 반환
			OutRow = *AllRows[RandomIndex];
		}
	}

	return OutRow;
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
	
	IPTWCombatInterface* CombatInterface = CastToPTWCombatInterface(TargetCharacter);
	if (!CombatInterface) return;
	
	if (CheckingDeadPlayer(NewPlayer))
	{
		CombatInterface->ApplyGameplayEffectToSelf(RestartPlayerEffect, 1.0f, FGameplayEffectContextHandle());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RESTART PLAYER"));
	}
}

void APTWDeliveryGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (!RaceTrackSpline)
	{
		RaceTrackSpline = Cast<ARaceTrack>(UGameplayStatics::GetActorOfClass(GetWorld(), ARaceTrack::StaticClass()));
	}
	
	if (!StartBlocker)
	{
		StartBlocker = Cast<AStartBlockActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AStartBlockActor::StaticClass()));
	}
}

void APTWDeliveryGameMode::StartCountDown()
{
	SendMessgeBeginPlay();
	Super::StartCountDown();
}

void APTWDeliveryGameMode::StartResultSequence()
{
		UWorld* World = GetWorld();
	if (!World) return;

	if (PTWGameState)
	{
		PTWGameState->SetCurrentPhase(EPTWGamePhase::MiniGameResult);
	}

	AActor* ResultCamera = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("ResultCamera"), FoundActors);
	if (FoundActors.Num() > 0) ResultCamera = FoundActors[0];

	TArray<AActor*> WinSpots;
	UGameplayStatics::GetAllActorsWithTag(World, FName("WinSpot"), WinSpots);

	TArray<AActor*> LoseSpots;
	UGameplayStatics::GetAllActorsWithTag(World, FName("LoseSpot"), LoseSpots);

	int32 CurrentWinIndex = 0;
	int32 CurrentLoseIndex = 0;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get());
		if (!PC) continue;

		APTWPlayerState* PS = PC->GetPlayerState<APTWPlayerState>();
		if (!PS) continue;

		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);

		if (APawn* OriginalPawn = PC->GetPawn())
		{
			OriginalPawn->SetActorHiddenInGame(true);
			OriginalPawn->SetActorEnableCollision(false);
		}

		if (ResultCamera)
		{
			FViewTargetTransitionParams Params;
			Params.BlendTime = 1.0f;
			Params.BlendFunction = EViewTargetBlendFunction::VTBlend_Linear;
			Params.bLockOutgoing = true;

			PC->ClientSetViewTarget(ResultCamera, Params);
		}

		if (ResultCharacterClass)
		{
			bool bIsWinner = WinnerChecking(PC); 

			FVector SpawnLoc = FVector::ZeroVector;
			FRotator SpawnRot = FRotator::ZeroRotator;
			bool bValidSpotFound = false;

			if (bIsWinner)
			{
				if (WinSpots.IsValidIndex(CurrentWinIndex))
				{
					SpawnLoc = WinSpots[CurrentWinIndex]->GetActorLocation();
					SpawnRot = WinSpots[CurrentWinIndex]->GetActorRotation();
					CurrentWinIndex++;
					bValidSpotFound = true;
				}
			}
			else
			{
				if (LoseSpots.IsValidIndex(CurrentLoseIndex))
				{
					SpawnLoc = LoseSpots[CurrentLoseIndex]->GetActorLocation();
					SpawnRot = LoseSpots[CurrentLoseIndex]->GetActorRotation();
					CurrentLoseIndex++;
					bValidSpotFound = true;
				}
			}

			if (bValidSpotFound)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APTWResultCharacter* ResultChar = World->SpawnActor<APTWResultCharacter>(ResultCharacterClass, SpawnLoc, SpawnRot, SpawnParams);

				FString PlayerName = PS->GetPlayerData().PlayerName;
				if (PlayerName.IsEmpty())
				{
					PlayerName = PS->GetPlayerName();
				}

				if (ResultChar)
				{
					ResultChar->InitializeResult(bIsWinner, PlayerName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Result] 스폰 포인트가 부족합니다! 맵에 TargetPoint를 더 배치해주세요."));
			}
		}
	}

	GetWorldTimerManager().SetTimer(ResultTimerHandle, this, &APTWDeliveryGameMode::FinishEndGameSequence, ResultSequenceDuration, false);
	
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
		SharedCheckPointStart->SetActorRotation(FRotator(0, -210.f, 0));
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
		GrantItemAbilities(ASC);
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

bool APTWDeliveryGameMode::WinnerChecking(APTWPlayerController* PC)
{
	if (!PC) return false;
	
	return GoalPlayers.ContainsByPredicate([PC](const APTWPlayerCharacter* Player)
	{
		return Player && Player->GetController() == PC;
	});
}

void APTWDeliveryGameMode::RemoveBeginGameplayEffect()
{
	for (APlayerState* AS : PTWGameState->AlivePlayers)
	{
		if (IPTWCombatInterface* CombatInterface = CastToPTWCombatInterface(Cast<APTWPlayerCharacter>(AS->GetPawn())))
		{
			CombatInterface->RemoveEffectWithTag(GameplayTags::State::Stun);
		}
	}
}

void APTWDeliveryGameMode::SendMessgeBeginPlay()
{
 	for (APlayerState* AS : PTWGameState->PlayerArray)
 	{
 		if (APTWPlayerController* PC = Cast<APTWPlayerController>(AS->GetPlayerController()))
 		{
 #define LOCTEXT_NAMESPACE "DeliveryGameMode"
 			FText BeginSendMessage = LOCTEXT("DeliveryBeginMsg", "Race to the finish line! Out of energy? Find a charging station to speed up!");
 #undef LOCTEXT_NAMESPACE
 			PC->SendMessage(BeginSendMessage, ENotificationPriority::Normal, 10);
 		}
 	}
}

void APTWDeliveryGameMode::ApplyBeginPlayEffect(APTWPlayerController* PC)
{
	IPTWCombatInterface* PTWInter = CastToPTWCombatInterface(Cast<APTWPlayerCharacter>(PC->GetPawn()));
	if (!PTWInter) return;
				
	PTWInter->ApplyGameplayEffectToSelf(BeginApplyEffect, 1.0f, FGameplayEffectContextHandle());
}

void APTWDeliveryGameMode::GrantItemAbilities(UAbilitySystemComponent* ASC) 
{
	static const FString ContextString(TEXT("Granting Default Items"));
	TArray<FRandomItemBoxData*> AllRows;
	ItemDataTable->GetAllRows<FRandomItemBoxData>(ContextString, AllRows);
	
	for (const FRandomItemBoxData* Row : AllRows)
	{
		if (Row && Row->RandomItemGA)
		{
			if (!ASC->FindAbilitySpecFromClass(Row->RandomItemGA))
			{
				FGameplayAbilitySpec ItemSpec(Row->RandomItemGA, 1);
				ASC->GiveAbility(ItemSpec);
			}
		}
	}
}

void APTWDeliveryGameMode::InitializeRaceRankingUI()
{
	for (int32 i = 0; i < RankPCList.Num(); ++i)
	{
		UPTWDeliveryControllerComponent* DeliveryControllerComp =  Cast<UPTWDeliveryControllerComponent>(RankPCList[i]->GetControllerComponent());
		if (!DeliveryControllerComp) return;
		DeliveryControllerComp->RaceRankUpdate();
	}
}
