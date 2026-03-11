// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PTWUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"           // PlayerState 접근을 위해 필요
#include "AbilitySystemInterface.h"              // IAbilitySystemInterface 정의
#include "AbilitySystemComponent.h"              // UAbilitySystemComponent 반환 타입을 위해 필요

#include "UI/InGameUI/PTWDamageIndicator.h"
#include "UI/ChatWidget/PTWChatInput.h"
#include "UI/ChatWidget/PTWChatList.h"
#include "PTWInGameHUD.h"
#include "RankBoard/PTWRankingBoard.h"

void UPTWUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPTWUISubsystem::Deinitialize()
{
	// 화면에서 모두 제거 (Destroy는 안 함)
	for (FUIStackEntry& Entry : WidgetStack)
	{
		if (Entry.Widget)
		{
			Entry.Widget->RemoveFromParent();
		}
	}

	WidgetStack.Empty();
	CachedWidgets.Empty();
	HUDWidget = nullptr;

	Super::Deinitialize();
}

UAbilitySystemComponent* UPTWUISubsystem::GetLocalPlayerASC() const
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("GetLocalPlayerASC : !PC"));

		return nullptr;
	}

	// PlayerState에서 가져오기
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(PC->PlayerState))
	{
		return ASCInterface->GetAbilitySystemComponent();
	}
	UE_LOG(LogTemp, Error, TEXT("GetLocalPlayerASC : !PS"));

	return nullptr;
}

void UPTWUISubsystem::PushWidget(TSubclassOf<UUserWidget> WidgetClass, EUIInputPolicy InputPolicy)
{
	UUserWidget* Widget = GetOrCreateWidget(WidgetClass);
	if (!Widget)
		return;

	// 이미 떠 있으면 중복 방지
	if (Widget->IsInViewport())
		return;

	// 기본 Policy
	FUIStackEntry Entry;
	Entry.Widget = Widget;
	Entry.ZOrder = 99; // Window Layer
	Entry.InputPolicy = InputPolicy;

	Widget->AddToViewport(Entry.ZOrder);
	WidgetStack.Add(Entry);

	ApplyInputPolicy(Entry.InputPolicy);

	if (InputPolicy != EUIInputPolicy::GameOnly)
	{
		Widget->SetKeyboardFocus();
	}
}

void UPTWUISubsystem::PopWidget()
{
	if (WidgetStack.IsEmpty())
		return;

	// 스택 최상단 제거
	FUIStackEntry TopEntry = WidgetStack.Pop();

	if (TopEntry.Widget)
	{
		TopEntry.Widget->RemoveFromParent();
	}

	// 다음 입력 정책 결정
	if (WidgetStack.IsEmpty())
	{
		ApplyInputPolicy(DefaultInputPolicy);
	}
	else
	{
		ApplyInputPolicy(WidgetStack.Last().InputPolicy);
	}
}

bool UPTWUISubsystem::IsWidgetInStack(TSubclassOf<UUserWidget> WidgetClass) const
{
	for (const FUIStackEntry& Entry : WidgetStack)
	{
		if (Entry.Widget && Entry.Widget->GetClass() == WidgetClass)
		{
			return true;
		}
	}
	return false;
}

bool UPTWUISubsystem::IsStackEmpty() const
{
	return WidgetStack.Num() == 0;
}

void UPTWUISubsystem::StackReset()
{
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	WidgetStack.Reset();
	PC->SetShowMouseCursor(false);
	PC->SetInputMode(FInputModeGameOnly());
}

UUserWidget* UPTWUISubsystem::GetTopWidget() const
{
	if (WidgetStack.Num() > 0)
	{
		return WidgetStack.Last().Widget;
	}
	return nullptr;
}

void UPTWUISubsystem::ShowHUD(TSubclassOf<UUserWidget> HUDClass)
{
	HUDWidget = GetOrCreateWidget(HUDClass);
	if (!HUDWidget) return;

	if (!HUDWidget->IsInViewport())
	{
		HUDWidget->AddToViewport(0);
	}

	// ASC 체크
	UAbilitySystemComponent* ASC = GetLocalPlayerASC();

	if (ASC)
	{
		if (UPTWInGameHUD* InGameHUD = Cast<UPTWInGameHUD>(HUDWidget))
		{
			InGameHUD->InitializeUI(ASC);
		}
	}
	else
	{
		// ASC 준비될 때까지 타이머
		GetWorld()->GetTimerManager().SetTimer(
			ASCInitTimerHandle,
			this,
			&UPTWUISubsystem::TryInitializeHUDASC,
			0.1f,
			true
		);
	}
}

UUserWidget* UPTWUISubsystem::CreatePersistentWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	UUserWidget* Widget = GetOrCreateWidget(WidgetClass);

	if (Widget && !Widget->IsInViewport())
	{
		Widget->AddToViewport(ZOrder);
		Widget->SetVisibility(ESlateVisibility::Hidden); // 처음엔 숨김 처리
	}
	return Widget;
}

UUserWidget* UPTWUISubsystem::ShowSystemWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!WidgetClass)
		return nullptr;

	APlayerController* PC = GetPlayerController();
	if (!PC)
		return nullptr;

	// 이미 떠 있으면 재사용
	if (TObjectPtr<UUserWidget>* Found = CachedWidgets.Find(WidgetClass))
	{
		UUserWidget* Widget = Found->Get();
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(ZOrder);
		}
		Widget->SetVisibility(ESlateVisibility::Visible);
		return Widget;
	}

	// 새로 생성
	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!Widget)
		return nullptr;

	Widget->AddToViewport(ZOrder);
	CachedWidgets.Add(WidgetClass, Widget);
	return Widget;
}

void UPTWUISubsystem::HideSystemWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!IsValid(this) || !GetWorld() || GetWorld()->bIsTearingDown)
	{
		return;
	}

	if (!WidgetClass) return;

	if (TObjectPtr<UUserWidget>* FoundWidgetPtr = CachedWidgets.Find(WidgetClass))
	{
		UUserWidget* Widget = FoundWidgetPtr->Get();
		if (IsValid(Widget))
		{
			Widget->RemoveFromParent();
		}
	}
}

void UPTWUISubsystem::SetWidgetVisibility(TSubclassOf<UUserWidget> WidgetClass, bool bVisible)
{
	if (TObjectPtr<UUserWidget>* Found = CachedWidgets.Find(WidgetClass))
	{
		UUserWidget* Widget = Found->Get();

		ESlateVisibility Visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
		Widget->SetVisibility(Visibility);

		if (bVisible)
		{
			if (UPTWRankingBoard* RankBoard = Cast<UPTWRankingBoard>(Widget))
			{
				RankBoard->UpdateRanking(); // 최신 정보로 갱신
			}
		}
	}
}

void UPTWUISubsystem::ShowDamageIndicator(const FVector& DamageCauserLocation)
{
	if (!DamageIndicatorClass) return;

	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	UPTWDamageIndicator* Indicator = CreateWidget<UPTWDamageIndicator>(PC, DamageIndicatorClass);

	if (!Indicator) return;

	Indicator->AddToViewport(50); 
	Indicator->Init(DamageCauserLocation);
}

UUserWidget* UPTWUISubsystem::GetOrCreateWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
		return nullptr;

	// 이미 캐싱되어 있다면 재사용
	if (TObjectPtr<UUserWidget>* Found = CachedWidgets.Find(WidgetClass))
	{
		return Found->Get();
	}

	APlayerController* PC = GetPlayerController();
	if (!PC)
		return nullptr;

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!NewWidget)
		return nullptr;

	CachedWidgets.Add(WidgetClass, NewWidget);
	return NewWidget;
}

void UPTWUISubsystem::PushNotification(const FNotificationData& Data)
{
	if (!HUDWidget) return;

	if (UPTWInGameHUD* InGameHUD = Cast<UPTWInGameHUD>(HUDWidget))
	{
		InGameHUD->ShowNotification(Data);
	}
}

APlayerController* UPTWUISubsystem::GetPlayerController() const
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		return LocalPlayer->GetPlayerController(GetWorld());
	}
	return nullptr;
}

void UPTWUISubsystem::ApplyInputPolicy(EUIInputPolicy Policy)
{
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	switch (Policy)
	{
	case EUIInputPolicy::GameOnly:
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
		break;
	}
	case EUIInputPolicy::UIOnly:
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());
		break;
	}
	case EUIInputPolicy::GameAndUI:
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
		break;
	}
	default:
		break;
	}
}

void UPTWUISubsystem::TryInitializeHUDASC()
{
	UAbilitySystemComponent* ASC = GetLocalPlayerASC();

	if (!ASC || !HUDWidget)
	{
		return;
	}

	if (UPTWInGameHUD* InGameHUD = Cast<UPTWInGameHUD>(HUDWidget))
	{
		InGameHUD->InitializeUI(ASC);
	}

	GetWorld()->GetTimerManager().ClearTimer(ASCInitTimerHandle);
}
