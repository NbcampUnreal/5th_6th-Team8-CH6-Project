#include "PTWDeliveryControllerComponent.h"

#include "CoreFramework/PTWPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UI/PTWUISubsystem.h"
#include "UI/MiniGame/Delivery/PTWBatterLevelWidget.h"
#include "UI/MiniGame/Delivery/PTWDeliveryHUD.h"

UPTWDeliveryControllerComponent::UPTWDeliveryControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWDeliveryControllerComponent::AddBatteryUI()
{
	if (GetOwner()->HasAuthority())
	{
		ClientRPC_AddBatteryUI();
	}
}

void UPTWDeliveryControllerComponent::ShowCountDownWidget()
{
	if (GetOwner()->HasAuthority())
	{
		ClientRPC_ShowCountDownWidget();
	}
}

void UPTWDeliveryControllerComponent::SetCountDownText(int32 Count)
{
	if (GetOwner()->HasAuthority())
	{
		ClientRPC_SetCountDownText(Count);
	}
}

void UPTWDeliveryControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, MyCurrentRank);
}


// Called when the game starts
void UPTWDeliveryControllerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPTWDeliveryControllerComponent::ClientRPC_ShowCountDownWidget_Implementation()
{
	if (DeliveryHUDWidgetInstance)
	{
		DeliveryHUDWidgetInstance->InitCountDownWidget();
	}
}

void UPTWDeliveryControllerComponent::ClientRPC_SetCountDownText_Implementation(int32 Count)
{
	if (DeliveryHUDWidgetInstance)
	{
		DeliveryHUDWidgetInstance->UpdateCountDownWidgetCount(Count);
	}
}

void UPTWDeliveryControllerComponent::ClientRPC_AddBatteryUI_Implementation()
{
	APTWPlayerController* PTWPC = Cast<APTWPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PTWPC) return;

	UPTWUISubsystem* UISubsystem = PTWPC->GetUISubSystem();
	if (!UISubsystem) 
	{
		UE_LOG(LogTemp, Error, TEXT("UISubsystem is still NULL on Client!"));
		return;
	}
	
	UUserWidget* WidetInstance = UISubsystem->CreatePersistentWidget(DeliveryHUDClass);
	if (!WidetInstance) return;
	
	if (UPTWDeliveryHUD* HUDWidget = Cast<UPTWDeliveryHUD>(WidetInstance))
	{
		DeliveryHUDWidgetInstance = HUDWidget;
		DeliveryHUDWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		DeliveryHUDWidgetInstance->InitBatterLevelWidget(UISubsystem->GetLocalPlayerASC());
	}
}

void UPTWDeliveryControllerComponent::RaceRankUpdate()
{
	if (GetNetMode() == NM_DedicatedServer) return;
	
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC && PC->IsLocalPlayerController())
	{
		if (DeliveryHUDWidgetInstance)
		{
			int32 Total = 0;
			if (GetWorld() && GetWorld()->GetGameState())
			{
				Total = GetWorld()->GetGameState()->PlayerArray.Num();
			}
			DeliveryHUDWidgetInstance->UpdateRank(MyCurrentRank, Total);
		}
	}
}

void UPTWDeliveryControllerComponent::OnRep_CurrentRank(int32 OldRank)
{
	if (MyCurrentRank != OldRank)
	{
		RaceRankUpdate();
	}
}



