// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_Blind.h"
#include "Components/PostProcessComponent.h"

AGC_Blind::AGC_Blind()
{
	PrimaryActorTick.bCanEverTick = true;
	bAutoDestroyOnRemove = false;
	
	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->bUnbound = true;
	PostProcessComponent->Priority = 10.0f;
	PostProcessComponent->SetupAttachment(RootComponent);
}

void AGC_Blind::BeginPlay()
{
	Super::BeginPlay();
}

bool AGC_Blind::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// GC 효과 재생
	if (IsValid(MyTarget) && IsValid(PostProcessMaterial))
	{
		APawn* TargetPawn = Cast<APawn>(MyTarget);
		if (!IsValid(TargetPawn) || !TargetPawn->IsLocallyControlled())
		{
			return Super::OnActive_Implementation(MyTarget, Parameters);
		}
		
		if (!IsValid(DynamicMaterial))
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
			PostProcessComponent->AddOrUpdateBlendable(DynamicMaterial, 1.0f);
		}
		TargetOpacity = 1.0f;
		bIsFadingOut = false;
	}
	
	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AGC_Blind::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	TargetOpacity = 0.0f;
	bIsFadingOut = true;
	
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

void AGC_Blind::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Blind CurrentOpacity -> TargetOpacity 천천히 변경  (Fade-in or Fade-out)
	if (!FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.01f))
	{
		if (!IsValid(PostProcessComponent)) return;
		
		CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaTime, FadeSpeed);
		DynamicMaterial->SetScalarParameterValue(OpacityParamName, CurrentOpacity);
		return;
	}
	// Fade-out 상태 and 투명도:0
	if (bIsFadingOut && FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.01f))
	{
		if (IsValid(PostProcessComponent))
		{
			PostProcessComponent->DestroyComponent();
			PostProcessComponent = nullptr;
		}
		Destroy();
	}
}

