// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/GhostChase/PTWGhostChaseControllerComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "UI/CharacterUI/PTWPlayerName.h"
#include "UI/MiniGame/GhostChase/PTWTargetViewWidget.h"
#include "UI/PTWUISubsystem.h"

UPTWGhostChaseControllerComponent::UPTWGhostChaseControllerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UPTWGhostChaseControllerComponent::SetTarget(APawn* NewTarget)
{
	CurrentTargetPawn = NewTarget;

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (PC->IsLocalController())
		{
			UpdateTargetPOV(CurrentTargetPawn);
		}
		else
		{
			// 2. 클라이언트에게 RPC로 실행 명령 (복제 변수보다 빠르고 확실함)
			Client_UpdateTargetPOV(NewTarget);
		}
	}
}

bool UPTWGhostChaseControllerComponent::IsTarget(APawn* Pawn) const
{
	return Pawn && Pawn == CurrentTargetPawn;
}

void UPTWGhostChaseControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPTWGhostChaseControllerComponent, CurrentTargetPawn);
}

void UPTWGhostChaseControllerComponent::OnRep_CurrentTargetPawn()
{
	UpdateTargetPOV(CurrentTargetPawn);
}

void UPTWGhostChaseControllerComponent::UpdateTargetPOV(APawn* NewTarget)
{
	APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController()) return;

	// 기존 타이머 제거
	GetWorld()->GetTimerManager().ClearTimer(POVCaptureTimerHandle);

	// 기존 캡처 참조 정리
	if (CurrentActiveCapture)
	{
		RefreshTargetViewHiddenActors();
		CurrentActiveCapture->TextureTarget = nullptr;
		CurrentActiveCapture = nullptr;
	}

	// 타겟이 없으면 위젯 숨기고 종료
	ULocalPlayer* LP = PC->GetLocalPlayer();
	UPTWUISubsystem* UISubsystem = LP ? LP->GetSubsystem<UPTWUISubsystem>() : nullptr;

	if (!NewTarget)
	{
		if (UISubsystem && POVWidgetClass) UISubsystem->HideSystemWidget(POVWidgetClass);
		return;
	}

	// RenderTarget 생성 및 최적화
	if (!TargetPOVRT)
	{
		TargetPOVRT = NewObject<UTextureRenderTarget2D>(this);
		TargetPOVRT->InitCustomFormat(480, 270, PF_B8G8R8A8, false);
		TargetPOVRT->UpdateResourceImmediate(true);
	}

	// 타겟 캐릭터로부터 캡처 컴포넌트 확보
	if (APTWPlayerCharacter* TargetChar = Cast<APTWPlayerCharacter>(NewTarget))
	{
		CurrentActiveCapture = TargetChar->GetTargetPOVCapture();
		if (CurrentActiveCapture)
		{
			CurrentActiveCapture->TextureTarget = TargetPOVRT;

			// Hidden Actors 설정 (타겟 자신 및 부착물)
			CurrentActiveCapture->HiddenActors.Empty();
			CurrentActiveCapture->HiddenActors.Add(TargetChar);
			TArray<AActor*> AttachedActors;
			TargetChar->GetAttachedActors(AttachedActors);
			CurrentActiveCapture->HiddenActors.Append(AttachedActors);

			// 캡처 타이머 시작 (30 FPS)
			GetWorld()->GetTimerManager().SetTimer(
				POVCaptureTimerHandle,
				this,
				&UPTWGhostChaseControllerComponent::CaptureTargetPOV,
				0.033f,
				true
			);

			// UI 업데이트
			if (UISubsystem && POVWidgetClass)
			{
				UISubsystem->ShowSystemWidget(POVWidgetClass);
				if (UPTWTargetViewWidget* POVWidget = Cast<UPTWTargetViewWidget>(UISubsystem->GetOrCreateWidget(POVWidgetClass)))
				{
					POVWidget->SetRenderTarget(TargetPOVRT);
					UISubsystem->SetWidgetVisibility(POVWidgetClass, true);
				}
			}
		}
	}
}

void UPTWGhostChaseControllerComponent::RefreshTargetViewHiddenActors()
{
	if (!CurrentActiveCapture) return;

	// 목록 초기화
	CurrentActiveCapture->HiddenActors.Empty();

	// 현재 추적 대상(Target) 및 그 부착물 숨기기
	if (APawn* TargetPawn = CurrentTargetPawn)
	{
		CurrentActiveCapture->HiddenActors.Add(TargetPawn);

		TArray<AActor*> TargetAttached;
		TargetPawn->GetAttachedActors(TargetAttached);
		CurrentActiveCapture->HiddenActors.Append(TargetAttached);
	}
}

void UPTWGhostChaseControllerComponent::CaptureTargetPOV()
{
	if (CurrentActiveCapture && TargetPOVRT)
	{
		CurrentActiveCapture->CaptureScene();
	}
}

void UPTWGhostChaseControllerComponent::Client_UpdateTargetPOV_Implementation(APawn* NewTarget)
{
	UpdateTargetPOV(NewTarget);
}

void UPTWGhostChaseControllerComponent::ApplyNameTagHighlight(APawn* TargetPawn, UWidgetComponent* WidgetComp)
{
	if (!WidgetComp) return;

	// 위젯 클래스 캐스팅
	UPTWPlayerName* NameTagWidget = Cast<UPTWPlayerName>(WidgetComp->GetUserWidgetObject());
	if (!NameTagWidget) return;

	// 타겟 여부에 따라 색상 지정
	if (IsTarget(TargetPawn))
	{
		NameTagWidget->SetNameColor(TargetHighlightColor);
	}
	else
	{
		NameTagWidget->SetNameColor(FLinearColor::White); // 기본 색상
	}
}

void UPTWGhostChaseControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (POVCaptureTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(POVCaptureTimerHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}
