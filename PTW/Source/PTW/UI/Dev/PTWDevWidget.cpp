// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Dev/PTWDevWidget.h"
#include "UI/PTWButton.h" 
#include "Components/EditableTextBox.h"
#include "System/PTWDeveloperSubsystem.h"
#include "Components/ComboBoxString.h"

void UPTWDevWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IsValid(Btn_AddBot)) Btn_AddBot->OnClicked.AddDynamic(this, &ThisClass::OnAddBotClicked);
	if (IsValid(Btn_RemoveBot)) Btn_RemoveBot->OnClicked.AddDynamic(this, &ThisClass::OnRemoveBotClicked);
	if (IsValid(Btn_TeleportBots)) Btn_TeleportBots->OnClicked.AddDynamic(this, &ThisClass::OnTeleportBotsClicked);

	if (IsValid(Btn_AddGold)) Btn_AddGold->OnClicked.AddDynamic(this, &ThisClass::OnAddGoldClicked);
	if (IsValid(Btn_SetTime)) Btn_SetTime->OnClicked.AddDynamic(this, &ThisClass::OnSetTimeClicked);
	if (IsValid(Btn_ForceStart)) Btn_ForceStart->OnClicked.AddDynamic(this, &ThisClass::OnForceStartClicked);
	if (IsValid(Btn_ForceEnd)) Btn_ForceEnd->OnClicked.AddDynamic(this, &ThisClass::OnForceEndClicked);
	if (IsValid(Btn_ForceWin)) Btn_ForceWin->OnClicked.AddDynamic(this, &ThisClass::OnForceWinClicked);
	if (IsValid(Btn_ForceLose)) Btn_ForceLose->OnClicked.AddDynamic(this, &ThisClass::OnForceLoseClicked);
	if (IsValid(Btn_SetNextMap))Btn_SetNextMap->OnClicked.AddDynamic(this, &ThisClass::OnSetNextMapClicked);

	if (IsValid(Btn_GodMode)) Btn_GodMode->OnClicked.AddDynamic(this, &ThisClass::OnGodModeClicked);
	if (IsValid(Btn_SuperSpeed)) Btn_SuperSpeed->OnClicked.AddDynamic(this, &ThisClass::OnSuperSpeedClicked);
	if (IsValid(Btn_SuperJump)) Btn_SuperJump->OnClicked.AddDynamic(this, &ThisClass::OnSuperJumpClicked);
	if (IsValid(Btn_FlyMode)) Btn_FlyMode->OnClicked.AddDynamic(this, &ThisClass::OnFlyModeClicked);
	if (IsValid(Btn_KillSelf)) Btn_KillSelf->OnClicked.AddDynamic(this, &ThisClass::OnKillSelfClicked);
	if (IsValid(Btn_RespawnSelf)) Btn_RespawnSelf->OnClicked.AddDynamic(this, &ThisClass::OnRespawnSelfClicked);

	if (IsValid(Btn_GrantItem)) Btn_GrantItem->OnClicked.AddDynamic(this, &ThisClass::OnGrantItemClicked);
	if (IsValid(Btn_ShowFPS)) Btn_ShowFPS->OnClicked.AddDynamic(this, &ThisClass::OnShowFPSClicked);
	if (IsValid(Btn_ToggleHUD)) Btn_ToggleHUD->OnClicked.AddDynamic(this, &ThisClass::OnToggleHUDClicked);
	if (IsValid(Btn_ShowCollision)) Btn_ShowCollision->OnClicked.AddDynamic(this, &ThisClass::OnShowCollisionClicked);


	if (IsValid(Combo_MapSelect))
	{
		Combo_MapSelect->ClearOptions();
		const UEnum* MapEnum = StaticEnum<EMiniGameMapType>();

		if (MapEnum)
		{
			for (int32 i = 0; i < MapEnum->NumEnums() - 1; ++i)
			{
				FText DisplayName = MapEnum->GetDisplayNameTextByIndex(i);
				Combo_MapSelect->AddOption(DisplayName.ToString());
			}

			Combo_MapSelect->SetSelectedIndex(0);
		}
	}
}

// ----------------------------------------------------
// 1. 봇 제어 로직
// ----------------------------------------------------
void UPTWDevWidget::OnAddBotClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->AddDummyBot();
}
void UPTWDevWidget::OnRemoveBotClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->RemoveAllDummyBots();
}
void UPTWDevWidget::OnTeleportBotsClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->TeleportBotsToMe();
}

// ----------------------------------------------------
// 2. 매치 흐름 제어 로직
// ----------------------------------------------------
void UPTWDevWidget::OnAddGoldClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>())
	{
		int32 Amount = Input_Gold ? FCString::Atoi(*Input_Gold->GetText().ToString()) : 1000;
		DevSys->AddGold(Amount);
	}
}
void UPTWDevWidget::OnSetTimeClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>())
	{
		float TimeVal = Input_Time ? FCString::Atof(*Input_Time->GetText().ToString()) : 1.0f;
		DevSys->SetGameSpeed(TimeVal);
	}
}
void UPTWDevWidget::OnForceStartClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ForceGameStart(); // TODO: 게임모드 연동 필요
}
void UPTWDevWidget::OnForceEndClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ForceRoundEnd(); // TODO: 게임모드 연동 필요
}
void UPTWDevWidget::OnForceWinClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ForceWin(); // TODO: 승리 처리 로직 연결
}
void UPTWDevWidget::OnForceLoseClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ForceLose(); // TODO: 패배 처리 로직 연결
}
void UPTWDevWidget::OnSetNextMapClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>())
	{
		if (IsValid(Combo_MapSelect))
		{
			int32 SelectedIndex = Combo_MapSelect->GetSelectedIndex();

			if (SelectedIndex >= 0)
			{
				EMiniGameMapType SelectedMap = static_cast<EMiniGameMapType>(SelectedIndex);
				DevSys->SetNextMapByEnum(SelectedMap);
			}
		}
	}
}

// ----------------------------------------------------
// 3. 캐릭터 제어 로직
// ----------------------------------------------------
void UPTWDevWidget::OnGodModeClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ToggleGodMode();
}
void UPTWDevWidget::OnSuperSpeedClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>())
	{
		float SpeedVal = Input_Speed ? FCString::Atof(*Input_Speed->GetText().ToString()) : 3.0f;
		DevSys->SetSuperSpeed(SpeedVal);
	}
}
void UPTWDevWidget::OnSuperJumpClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>())
	{
		float JumpVal = Input_Jump ? FCString::Atof(*Input_Jump->GetText().ToString()) : 2000.f;
		DevSys->SetSuperJump(JumpVal);
	}
}
void UPTWDevWidget::OnFlyModeClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ToggleFlyMode();
}
void UPTWDevWidget::OnKillSelfClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->KillSelf();
}
void UPTWDevWidget::OnRespawnSelfClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->RespawnSelf();
}

// ----------------------------------------------------
// 4. 아이템 & 디버깅 제어 로직
// ----------------------------------------------------
void UPTWDevWidget::OnGrantItemClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>())
	{
		FString ItemID = Input_ItemID ? Input_ItemID->GetText().ToString() : TEXT("");
		DevSys->GrantItem(ItemID); // TODO: 실제 인벤토리 컴포넌트에 아이템 추가 로직 연결
	}
}
void UPTWDevWidget::OnShowFPSClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->TogglePingAndFPS();
}
void UPTWDevWidget::OnToggleHUDClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ToggleHUD();
}
void UPTWDevWidget::OnShowCollisionClicked()
{
	if (UPTWDeveloperSubsystem* DevSys = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>()) DevSys->ToggleCollisionRender();
}
