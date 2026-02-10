#include "GC_WaterHit.h"

#include "Components/PostProcessComponent.h"


AGC_WaterHit::AGC_WaterHit()
{
	PrimaryActorTick.bCanEverTick = true;
}

 bool AGC_WaterHit::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
 {
	if (!MyTarget) return false;
	
	APawn* TargetPawn = Cast<APawn>(MyTarget);
	if (!TargetPawn || !TargetPawn->IsLocallyControlled()) 
	{
		return false; 
	}

	if (!PostProcessMaterial) return false;
	
	if (PostProcessComponent) 
	{
		return true; 
	}
	
	PostProcessComponent = NewObject<UPostProcessComponent>(this);
	PostProcessComponent->RegisterComponentWithWorld(GetWorld());
	
	PostProcessComponent->bUnbound = true;
	PostProcessComponent->Priority = 10.0f;
	
	DynamicMaterial = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
	
	FWeightedBlendable Blendable;
	Blendable.Weight = 1.0f;
	Blendable.Object = DynamicMaterial;

	PostProcessComponent->Settings.WeightedBlendables.Array.Add(Blendable);

	return true;
 }

bool AGC_WaterHit::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!MyTarget) return false;
	
	APawn* TargetPawn = Cast<APawn>(MyTarget);
	if (!TargetPawn || !TargetPawn->IsLocallyControlled()) 
	{
		return false; 
	}
	
	if (PostProcessComponent)
	{
		PostProcessComponent->DestroyComponent();
		PostProcessComponent = nullptr;
	}

	return true;
}




