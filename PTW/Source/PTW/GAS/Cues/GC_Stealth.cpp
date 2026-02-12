#include "GC_Stealth.h"

#include "Components/WidgetComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"

// Sets default values
AGC_Stealth::AGC_Stealth()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AGC_Stealth::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(MyTarget);
	if (!Character) return false;
	
	UMeshComponent* Mesh = Character->GetMesh();
	for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
	{
		UMaterialInstanceDynamic* DynMat = Mesh->CreateDynamicMaterialInstance(i);
		if (DynMat)
		{
			DynMat->SetScalarParameterValue(TEXT("Opacity"), 0.2f);
		}
	}
	
	if (HasAuthority())
	{
		Character->SetStealthMode(true);
	}
	
	return true;
}

bool AGC_Stealth::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(MyTarget);
	if (!Character) return false;
	
	// 다시 불투명하게 복구
	UMeshComponent* Mesh = Character->GetMesh();
	for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
	{
		if (UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(i)))
		{
			DynMat->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
		}
	}
	
	if (HasAuthority())
	{
		Character->SetStealthMode(false);
	}
	
	return true;
}


