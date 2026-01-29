// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWDamageIndicator.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

void UPTWDamageIndicator::Init(const FVector& DamageCauserLocation)
{
	RemainingLifeTime = LifeTime;

	UpdateRotation(DamageCauserLocation);
	SetRenderOpacity(1.f);
}

void UPTWDamageIndicator::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RemainingLifeTime -= InDeltaTime;

	UpdateOpacity();

	if (RemainingLifeTime <= 0.f)
	{
		RemoveFromParent();
	}
}

void UPTWDamageIndicator::UpdateOpacity()
{
	if (RemainingLifeTime > FadeStartTime)
		return;

	const float Alpha = FMath::Clamp(
		RemainingLifeTime / FadeStartTime,
		0.f,
		1.f
	);

	SetRenderOpacity(Alpha);
}

void UPTWDamageIndicator::UpdateRotation(const FVector& DamageCauserLocation)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->PlayerCameraManager) return;

	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FVector CameraForward = PC->PlayerCameraManager->GetActorForwardVector();

	const FVector ToDamage = (DamageCauserLocation - CameraLocation).GetSafeNormal();

	// 카메라 기준 로컬 회전
	const float AngleRad = FMath::Atan2(
		FVector::DotProduct(
			FVector::CrossProduct(CameraForward, ToDamage),
			PC->PlayerCameraManager->GetActorUpVector()
		),
		FVector::DotProduct(CameraForward, ToDamage)
	);

	const float AngleDeg = FMath::RadiansToDegrees(AngleRad);

	SetRenderTransformAngle(AngleDeg);
}
