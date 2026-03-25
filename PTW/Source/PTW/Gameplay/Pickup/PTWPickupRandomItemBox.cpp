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
			FGameplayAbilitySpec Spec(SelectedItem.RandomItemGA, 1);
			Spec.RemoveAfterActivation = true;
			Spec.ReplicationID = 0;
			ASC->GiveAbilityAndActivateOnce(Spec);
		}
		
		if (!SelectedItem.ActivateText.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("Item Activated: %s"), *SelectedItem.ActivateText.ToString());
		}
	}
	
}


