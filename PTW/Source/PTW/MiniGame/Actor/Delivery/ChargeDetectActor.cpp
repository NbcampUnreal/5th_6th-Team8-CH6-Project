#include "ChargeDetectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"


AChargeDetectActor::AChargeDetectActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AChargeDetectActor::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (!ASC) return;

		FGameplayAbilitySpec Spec = FGameplayAbilitySpec(ChargeAbilityClass);
		FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(Spec);
		ASC->TryActivateAbility(SpecHandle);
	}
}

