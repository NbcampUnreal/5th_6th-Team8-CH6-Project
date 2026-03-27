#include "PTWPickupRandomItemBox.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "MiniGame/Data/Delivery/PTWRandomItemBoxData.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"


APTWPickupRandomItemBox::APTWPickupRandomItemBox()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APTWPickupRandomItemBox::BeginPlay()
{
	Super::BeginPlay();
}

void APTWPickupRandomItemBox::OnPickedUp(class APTWPlayerCharacter* Player)
{
	if (!HasAuthority() || !Player) return;
	
	APTWPlayerController* Controller = Cast<APTWPlayerController>(Player->GetController());
	if (!Controller) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
	if (!ASC) return;
	
	APTWDeliveryGameMode* DeliveryGM = Cast<APTWDeliveryGameMode>(GetWorld()->GetAuthGameMode());
	if (DeliveryGM)
	{
		FRandomItemBoxData SelectedItem = DeliveryGM->GetRandomItemRowFromTable();
		
		if (SelectedItem.RandomItemGA)
		{
			ASC->TryActivateAbilityByClass(SelectedItem.RandomItemGA);
		}
		
		if (!SelectedItem.ActivateText.IsEmpty())
		{
			// 26.03.27
			//Controller->SendMessage(SelectedItem.ActivateText, ENotificationPriority::Normal, 3);
		}
	}
	
}


