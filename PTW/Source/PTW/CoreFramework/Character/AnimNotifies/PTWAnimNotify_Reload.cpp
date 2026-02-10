// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/AnimNotifies/PTWAnimNotify_Reload.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "Weapon/PTWWeaponActor.h"

FString UPTWAnimNotify_Reload::GetNotifyName_Implementation() const
{
	FString ActionName;
	switch (ActionType)
	{
	case EReloadEventAction::DropMag:   ActionName = "Drop Mag"; break;
	case EReloadEventAction::GrabMag:   ActionName = "Grab Mag"; break;
	case EReloadEventAction::InsertMag: ActionName = "Insert Mag"; break;
	}
	return FString::Printf(TEXT("Reload: %s"), *ActionName);
}

void UPTWAnimNotify_Reload::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	if (APTWWeaponActor* TargetWeapon = Cast<APTWWeaponActor>(MeshComp->GetOwner()))
	{
		TargetWeapon->HandleReloadEvent(ActionType);
	}
}
