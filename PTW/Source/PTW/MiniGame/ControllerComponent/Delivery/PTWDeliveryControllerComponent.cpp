#include "PTWDeliveryControllerComponent.h"

#include "CoreFramework/PTWPlayerController.h"
#include "UI/PTWUISubsystem.h"
#include "UI/MiniGame/Delivery/PTWBatterLevelWidget.h"

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

// Called when the game starts
void UPTWDeliveryControllerComponent::BeginPlay()
{
	Super::BeginPlay();
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
	
	UUserWidget* WidetInstance = UISubsystem->CreatePersistentWidget(BatteryWidgetClass);
	if (!WidetInstance) return;
	
	if (UPTWBatterLevelWidget* BatterLevelWidget = Cast<UPTWBatterLevelWidget>(WidetInstance))
	{
		BatterLevelWidget->InitWithASC(UISubsystem->GetLocalPlayerASC());
		BatterLevelWidget->SetVisibility(ESlateVisibility::Visible);
	}
}



