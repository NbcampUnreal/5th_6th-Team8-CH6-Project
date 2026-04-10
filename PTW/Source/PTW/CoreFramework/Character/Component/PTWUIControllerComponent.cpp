// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWUIControllerComponent.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "UI/PTWUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Components/WidgetComponent.h"
#include "EngineUtils.h"
#include "CoreFramework/PTWGameUserSettings.h"

#include "MiniGame/ControllerComponent/GhostChase/PTWGhostChaseControllerComponent.h"
#include "UI/PTWInGameHUD.h"
#include "UI/RankBoard/PTWRankingBoard.h"
#include "UI/ChatWidget/PTWChatList.h"
#include "UI/ChatWidget/PTWChatInput.h"
#include "UI/InGameUI/PTWDamageIndicator.h"
#include "UI/Dev/PTWDevWidget.h"
#include "UI/MiniGame/PTWGameStartTimer.h"
#include "UI/PTWPopupWidget.h"


UPTWUIControllerComponent::UPTWUIControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CachedGameState = nullptr;
}

void UPTWUIControllerComponent::InitializeUIComponent(APTWPlayerController* InPC)
{
	if (!InPC) return;

	OwnerPC = InPC;

	if (ULocalPlayer* LocalPlayer = OwnerPC->GetLocalPlayer())
	{
		UISubsystem = ULocalPlayer::GetSubsystem<UPTWUISubsystem>(LocalPlayer);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GameStateBindRetryHandle);
	}

	if (OwnerPC->GetWorld())
	{
		OwnerPC->GetWorld()->GetTimerManager().SetTimer(NameTagTimerHandle, this, &ThisClass::UpdateNameTagsVisibility, NameTagUpdateInterval, true);
	}

	BindGameStateDelegates();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(NameTagTimerHandle, this, &UPTWUIControllerComponent::UpdateNameTagsVisibility, NameTagUpdateInterval, true);
	}
}

void UPTWUIControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (!World && OwnerPC) World = OwnerPC->GetWorld();

	if (World)
	{
		FTimerManager& TM = World->GetTimerManager();
		TM.ClearTimer(NameTagTimerHandle);
		TM.ClearTimer(GameStateBindRetryHandle);
	}

	UnbindGameStateDelegates();

	Super::EndPlay(EndPlayReason);
}

void UPTWUIControllerComponent::CreateUI()
{
	if (!OwnerPC)
	{
		return;
	}

	if (!OwnerPC->IsLocalController())
	{
		return;
	}

	if (!UISubsystem)
	{
		return;
	}

	UISubsystem->ClearAllUI();
	UISubsystem->StackReset();

	if (HUDClass)
	{
		UISubsystem->ShowHUD(HUDClass);
	}

	if (RankingBoardClass)
	{
		UISubsystem->CreatePersistentWidget(RankingBoardClass, 10);
	}

	if (ChatListClass)
	{
		if (UUserWidget* ChatListWidget = UISubsystem->CreatePersistentWidget(ChatListClass, 70))
		{
			ChatListWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		UISubsystem->SetChatListClass(ChatListClass);
		bAbleChat = true;
	}

	if (ChatInputClass)
	{
		UISubsystem->SetChatInputClass(ChatInputClass);
	}

	if (DamageIndicatorClass)
	{
		UISubsystem->SetDamageIndicatorClass(DamageIndicatorClass);
	}

	if (KeyGuideWidgetClass)
	{
		UISubsystem->CreatePersistentWidget(KeyGuideWidgetClass, 15);
		
		UGameUserSettings* BaseSettings = UGameUserSettings::GetGameUserSettings();
		UPTWGameUserSettings* PTWSettings = Cast<UPTWGameUserSettings>(BaseSettings);

		if (PTWSettings)
		{
			bKeyGuideOn = PTWSettings->GetbKeyGuideOn();
			UISubsystem->SetWidgetVisibility(KeyGuideWidgetClass, bKeyGuideOn);
		}
		else
		{
			bKeyGuideOn = true;
			UISubsystem->SetWidgetVisibility(KeyGuideWidgetClass, true);
		}
	}
}

void UPTWUIControllerComponent::ReInitializeUI()
{
	if (UISubsystem)
	{
		UISubsystem->TryInitializeHUDASC();
	}
}

void UPTWUIControllerComponent::ToggleRankingBoard(bool bShow)
{
	if (!OwnerPC || !UISubsystem || !RankingBoardClass) return;

	if (!bAbleRankingBoard) return;

	if (bShow)
	{
		UISubsystem->SetWidgetVisibility(RankingBoardClass, true);
	}
	else
	{
		UISubsystem->SetWidgetVisibility(RankingBoardClass, false);
	}
}

void UPTWUIControllerComponent::TogglePauseMenu()
{
	if (!OwnerPC || !UISubsystem || !PauseMenuClass) return;

	if (!UISubsystem->IsStackEmpty())
	{
		UISubsystem->PopWidget();
	}
	else
	{
		if (PauseMenuClass)
		{
			UISubsystem->PushWidget(PauseMenuClass, EUIInputPolicy::GameAndUI);
		}
	}
}

void UPTWUIControllerComponent::ToggleChat()
{
	if (!OwnerPC || !UISubsystem || !ChatInputClass) return;

	if (bAbleChat)
	{
		UISubsystem->PushWidget(ChatInputClass, EUIInputPolicy::GameAndUI);

		if (UPTWChatList* ChatList = Cast<UPTWChatList>(UISubsystem->GetOrCreateWidget(ChatListClass)))
		{
			ChatList->SetInteractionMode(true);
		}

		bAbleChat = false;
	}
}

void UPTWUIControllerComponent::ChatInputFinished()
{
	if (!OwnerPC || !UISubsystem) return;

	if (UPTWChatList* ChatList = Cast<UPTWChatList>(UISubsystem->GetOrCreateWidget(ChatListClass)))
	{
		ChatList->SetInteractionMode(false);
	}

	UISubsystem->PopWidget();

	bAbleChat = true;
}

void UPTWUIControllerComponent::ToggleKeyGuide()
{
	UGameUserSettings* BaseSettings = UGameUserSettings::GetGameUserSettings();
	UPTWGameUserSettings* PTWSettings = Cast<UPTWGameUserSettings>(BaseSettings);

	if (!OwnerPC || !UISubsystem || !KeyGuideWidgetClass) return;

	bKeyGuideOn = !bKeyGuideOn;

	if (UISubsystem)
	{
		UISubsystem->SetWidgetVisibility(KeyGuideWidgetClass, bKeyGuideOn);
	}
	if (PTWSettings)
	{
		PTWSettings->SetbKeyGuideOn(bKeyGuideOn);
	}
}

void UPTWUIControllerComponent::ToggleDevUI()
{
	if (!OwnerPC || !DevWidgetClass) return;

	if (DevWidgetInstance && DevWidgetInstance->IsInViewport())
	{
		DevWidgetInstance->RemoveFromParent();
		DevWidgetInstance = nullptr;

		OwnerPC->SetInputMode(FInputModeGameOnly());
		OwnerPC->SetShowMouseCursor(false);
	}
	else
	{
		DevWidgetInstance = CreateWidget<UPTWDevWidget>(OwnerPC, DevWidgetClass);
		if (DevWidgetInstance)
		{
			DevWidgetInstance->AddToViewport(999);
			OwnerPC->SetInputMode(FInputModeGameAndUI());
			OwnerPC->SetShowMouseCursor(true);
		}
	}
}

void UPTWUIControllerComponent::OnMiniGameCountdownChanged(bool bStarted)
{
	if (!UISubsystem || !GameStartTimerClass) return;

	if (bStarted)
	{
		UISubsystem->ShowSystemWidget(GameStartTimerClass, 70);
	}
	else
	{
		UISubsystem->HideSystemWidget(GameStartTimerClass);
	}
}

void UPTWUIControllerComponent::HandleRoulettePhaseChanged(FPTWRouletteData RouletteData)
{
	const UEnum* EnumPtr = StaticEnum<EPTWRoulettePhase>();
	FString PhaseName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)RouletteData.CurrentPhase) : TEXT("Invalid");

	if (!UISubsystem)
	{
		return;
	}

	switch (RouletteData.CurrentPhase)
	{
	case EPTWRoulettePhase::MapRoulette:
		if (MapRouletteWidgetClass)
		{
			UISubsystem->ShowSystemWidget(MapRouletteWidgetClass, 85);
		}
		break;
	case EPTWRoulettePhase::RoundEventRoulette:
		if (MapRouletteWidgetClass)
		{
			UISubsystem->HideSystemWidget(MapRouletteWidgetClass);
		}
		break;
	case EPTWRoulettePhase::Finished:
		if (MapRouletteWidgetClass)
		{
			UISubsystem->HideSystemWidget(MapRouletteWidgetClass);
		}
		break;

	default:
		break;
	}
}

void UPTWUIControllerComponent::HandleGamePhaseChanged(EPTWGamePhase CurrentGamePhase)
{
	const UEnum* EnumPtr = StaticEnum<EPTWGamePhase>();
	FString PhaseName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)CurrentGamePhase) : TEXT("Invalid");

	if (!UISubsystem)
	{
		return;
	}

	switch (CurrentGamePhase)
	{
	case EPTWGamePhase::GameResult:
		if (RankingBoardClass)
		{
			UISubsystem->SetWidgetVisibility(RankingBoardClass, true);
			bAbleRankingBoard = false;
		}
		break;

	case EPTWGamePhase::MiniGameResult:
		if (RankingBoardClass)
		{
			UISubsystem->SetWidgetVisibility(RankingBoardClass, true);
			bAbleRankingBoard = false;
		}
		break;

	default:
		if (RankingBoardClass)
		{
			UISubsystem->SetWidgetVisibility(RankingBoardClass, false);
			bAbleRankingBoard = true;
		}
		break;
	}
}

void UPTWUIControllerComponent::HandleOpenPredictVoteUI()
{
	if (UISubsystem && PredictWinVote)
	{
		UISubsystem->PushWidget(PredictWinVote, EUIInputPolicy::UIOnly);
	}
}

void UPTWUIControllerComponent::ShowDamageIndicator(FVector DamageCauserLocation)
{
	if (!OwnerPC || !UISubsystem) return;

	UISubsystem->ShowDamageIndicator(DamageCauserLocation);
}

void UPTWUIControllerComponent::BuyVoteItem()
{
	if (GetOwner()->HasAuthority())
	{
		Client_OpenPredictVoteUI();
	}
	else
	{
		HandleOpenPredictVoteUI();
	}
}

void UPTWUIControllerComponent::Client_OpenPredictVoteUI_Implementation()
{
	HandleOpenPredictVoteUI();
}

void UPTWUIControllerComponent::Client_ShowNotification_Implementation(const FNotificationData& Data)
{
	if (!OwnerPC || !IsValid(UISubsystem)) return;

	UISubsystem->PushNotification(Data);
}

void UPTWUIControllerComponent::ShowLocalNotification(const FNotificationData& Data)
{
	if (!OwnerPC) return;

	if (UISubsystem)
	{
		UISubsystem->PushNotification(Data);
	}
}

void UPTWUIControllerComponent::SendMessage(const FText& InText, ENotificationPriority InPriority, float InDuration, bool bInterrupt)
{
	if (!OwnerPC->HasAuthority()) return;

	FNotificationData Data;
	Data.Message = InText;
	Data.Priority = InPriority;
	Data.Duration = InDuration;
	Data.bInterrupt = bInterrupt;

	Client_ShowNotification(Data);
}

void UPTWUIControllerComponent::Popup(const FText& InText)
{
	if (!PopupWidgetClass) return;

	if (!UISubsystem) return;

	UISubsystem->PushWidget(PopupWidgetClass, EUIInputPolicy::GameAndUI);

	UUserWidget* TopWidget = UISubsystem->GetTopWidget();

	if (UPTWPopupWidget* Popup = Cast<UPTWPopupWidget>(TopWidget))
	{
		Popup->SetMessage(InText);
	}
}

void UPTWUIControllerComponent::UpdateNameTagsVisibility()
{
	if (!OwnerPC) return;

	APawn* MyPawn = OwnerPC->GetPawn();
	if (!MyPawn || !OwnerPC->PlayerCameraManager) return;

	const FVector CameraLocation = OwnerPC->PlayerCameraManager->GetCameraLocation();
	const FVector CameraForward = OwnerPC->PlayerCameraManager->GetActorForwardVector();
	const float   MaxDistSq = FMath::Square(NameTagMaxDistance);

	for (TActorIterator<APTWPlayerCharacter> It(GetWorld()); It; ++It)
	{
		APTWPlayerCharacter* TargetChar = *It;
		if (!TargetChar) continue;

		// 자기 자신 / 사망 체크
		if (TargetChar == MyPawn || TargetChar->IsDead() || TargetChar->GetStealthMode())
		{
			if (UWidgetComponent* WidgetComp = TargetChar->GetNameTagWidget())
			{
				WidgetComp->SetVisibility(false);
			}
			continue;
		}

		UWidgetComponent* WidgetComp = TargetChar->GetNameTagWidget();
		if (!WidgetComp) continue;

		// 거리 체크 (DistSquared)
		const FVector TargetLocation = TargetChar->GetActorLocation();
		const float DistSq = FVector::DistSquared(CameraLocation, TargetLocation);

		if (DistSq > MaxDistSq)
		{
			WidgetComp->SetVisibility(false);
			continue;
		}

		// 시야각(FOV) 체크
		const FVector ToTarget = (TargetLocation - CameraLocation).GetSafeNormal();
		const float Dot = FVector::DotProduct(CameraForward, ToTarget);

		if (Dot < 0.3f) // 약 72도
		{
			WidgetComp->SetVisibility(false);
			continue;
		}

		// 벽 가림 체크 (Line Trace)
		const FVector TraceEnd = WidgetComp->GetComponentLocation();

		FHitResult HitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(NameTagTrace), false);
		Params.AddIgnoredActor(MyPawn);
		Params.AddIgnoredActor(TargetChar);

		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CameraLocation,
			TraceEnd,
			ECC_Visibility,
			Params
		);

		// 가시성 설정 및 스케일 조절 추가
		if (!bHit)
		{
			WidgetComp->SetVisibility(true);

			// 거리에 따른 스케일 계산 
			const float CurrentDist = FMath::Sqrt(DistSq);

			// 거리 0(스케일 1.0) ~ MaxDist(스케일 MinScale) 사이를 매핑
			float TargetScale = FMath::GetMappedRangeValueClamped(
				FVector2D(0.f, NameTagMaxDistance),
				FVector2D(1.0f, NameTagMinScale),
				CurrentDist
			);

			if (CachedGhostChaseComp)
			{
				CachedGhostChaseComp->ApplyNameTagHighlight(TargetChar, WidgetComp);
			}
		}
	}
}

void UPTWUIControllerComponent::BindGameStateDelegates()
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	APTWGameState* GS = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;
	if (!IsValid(GS))
	{
		World->GetTimerManager().SetTimer(
			GameStateBindRetryHandle,
			this,
			&UPTWUIControllerComponent::BindGameStateDelegates,
			0.2f,
			false
		);
		return;
	}
	World->GetTimerManager().ClearTimer(GameStateBindRetryHandle);

	// 중복 바인딩 방지
	UnbindGameStateDelegates();

	CachedGameState = GS;

	// 델리게이트 바인드
	CachedGameState->OnMiniGameCountdownChanged.AddDynamic(this, &ThisClass::OnMiniGameCountdownChanged);
	CachedGameState->OnRoulettePhaseChanged.AddDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
	CachedGameState->OnGamePhaseChanged.AddDynamic(this, &ThisClass::HandleGamePhaseChanged);

	// 현재상태 반영
	OnMiniGameCountdownChanged(CachedGameState->IsMiniGameCountdown());
	HandleRoulettePhaseChanged(CachedGameState->GetRouletteData());
	HandleGamePhaseChanged(CachedGameState->GetCurrentGamePhase());
}

void UPTWUIControllerComponent::UnbindGameStateDelegates()
{
	if (IsValid(CachedGameState))
	{
		CachedGameState->OnMiniGameCountdownChanged.RemoveDynamic(this, &ThisClass::OnMiniGameCountdownChanged);
		CachedGameState->OnRoulettePhaseChanged.RemoveDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
		CachedGameState->OnGamePhaseChanged.RemoveDynamic(this, &ThisClass::HandleGamePhaseChanged);
	}

	CachedGameState = nullptr;
}

void UPTWUIControllerComponent::Client_FindGhostChaseComponent_Implementation()
{
	if (AActor* OwnerActor = GetOwner())
	{
		CachedGhostChaseComp = OwnerActor->FindComponentByClass<UPTWGhostChaseControllerComponent>();
	}
}
