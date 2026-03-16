#include "PTWDeliveryControllerComponent.h"

#include "CoreFramework/PTWPlayerController.h"
#include "UI/PTWUISubsystem.h"
#include "UI/MiniGame/Delivery/PTWBatterLevelWidget.h"
#include "UI/MiniGame/Delivery/PTWDeliveryHUD.h"

UPTWDeliveryControllerComponent::UPTWDeliveryControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


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



