#include "GC_Delivery.h"

#include "CoreFramework/PTWPlayerCharacter.h"


AGC_Delivery::AGC_Delivery()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetupAttachment(SceneComponent);
}

bool AGC_Delivery::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// TODO : Attach
	APTWPlayerCharacter* MyCharacter = Cast<APTWPlayerCharacter>(MyTarget);
	if (!MyCharacter) return false;
	
	this->AttachToActor(MyCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("DeliverySocket"));
	
	return Super::WhileActive_Implementation(MyTarget, Parameters);
}

bool AGC_Delivery::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// TODO : Attach 제거
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}




