// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PTWUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

/********** UPTWUISubsystem **********/

void UPTWUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("UPTWUISubsystem Initialized"));
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

	UE_LOG(LogTemp, Log, TEXT("UPTWUISubsystem Deinitialized"));

	Super::Deinitialize();
}

/********** Helpers **********/

APlayerController* UPTWUISubsystem::GetPlayerController() const
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		return LocalPlayer->GetPlayerController(GetWorld());
	}
	return nullptr;
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


/********** Input Policy **********/

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

/********** Stack-based UI **********/

void UPTWUISubsystem::PushWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	UUserWidget* Widget = GetOrCreateWidget(WidgetClass);
	if (!Widget)
		return;

	// 이미 떠 있으면 중복 방지
	if (Widget->IsInViewport())
		return;

	// 기본 정책 (필요하면 나중에 위젯별로 커스터마이즈 가능)
	FUIStackEntry Entry;
	Entry.Widget = Widget;
	Entry.ZOrder = 10; // Window Layer
	Entry.InputPolicy = EUIInputPolicy::UIOnly;

	Widget->AddToViewport(Entry.ZOrder);
	WidgetStack.Add(Entry);

	ApplyInputPolicy(Entry.InputPolicy);
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
		ApplyInputPolicy(EUIInputPolicy::GameOnly);
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


/********** HUD (Non-stack UI) **********/

void UPTWUISubsystem::ShowHUD(TSubclassOf<UUserWidget> HUDClass)
{
	if (HUDWidget)
		return;

	HUDWidget = GetOrCreateWidget(HUDClass);
	if (!HUDWidget)
		return;

	HUDWidget->AddToViewport(0); // HUD Layer (가장 뒤)
}
