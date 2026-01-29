// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/AnimNotifies/PTWAnimNotify_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

UPTWAnimNotify_SendGameplayEvent::UPTWAnimNotify_SendGameplayEvent()
{

}

void UPTWAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (OwnerActor && EventTag.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		Payload.Instigator = OwnerActor;
		Payload.Target = OwnerActor;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
	}
}

FString UPTWAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return FString::Printf(TEXT("SendEvent: %s"), *EventTag.ToString());
	}

	return Super::GetNotifyName_Implementation();
}
