// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_Blind.h"

#include "Components/PostProcessComponent.h"


// Sets default values
AGC_Blind::AGC_Blind()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
	// 핵심!: 큐가 해제될 때 액터가 즉시 삭제되는 것을 막고, Fade-Out 연출 후 우리가 직접 지웁니다.
	bAutoDestroyOnRemove = false; 
    
	CurrentOpacity = 0.0f;
	TargetOpacity = 0.0f;
	bIsFadingOut = false;
}

// Called when the game starts or when spawned
void AGC_Blind::BeginPlay()
{
	Super::BeginPlay();
}

bool AGC_Blind::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	UE_LOG(LogTemp, Warning, TEXT("AGC_Blind OnActive_Implementation"));
	if (!MyTarget) return false;
    
	APawn* TargetPawn = Cast<APawn>(MyTarget);
	if (!TargetPawn || !TargetPawn->IsLocallyControlled()) 
	{
		return false; 
	}

	if (!PostProcessMaterial) return false;
    
	if (!PostProcessComponent) 
	{
		PostProcessComponent = NewObject<UPostProcessComponent>(this);
		PostProcessComponent->RegisterComponentWithWorld(GetWorld());
		PostProcessComponent->bUnbound = true;
		PostProcessComponent->Priority = 10.0f;
	}
    
	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
		FWeightedBlendable Blendable;
		Blendable.Weight = 1.0f;
		Blendable.Object = DynamicMaterial;
		PostProcessComponent->Settings.WeightedBlendables.Array.Add(Blendable);
	}

	// 어빌리티 시작: 목표 투명도를 1로 설정하여 화면을 까맣게 덮기 시작함
	TargetOpacity = 1.0f;
	bIsFadingOut = false;
	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AGC_Blind::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	UE_LOG(LogTemp, Warning, TEXT("AGC_Blind OnRemove_Implementation"));
	TargetOpacity = 0.0f;
	bIsFadingOut = true;
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

// Called every frame
void AGC_Blind::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (DynamicMaterial && !FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.01f))
	{
		// FInterpTo를 사용해 부드럽게 수치 변경
		CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaTime, FadeSpeed);
		DynamicMaterial->SetScalarParameterValue(OpacityParamName, CurrentOpacity);
	}
	// Fade-out 중이고, 투명도가 0에 거의 도달했다면 (연출이 완전히 끝났다면)
	else if (bIsFadingOut && DynamicMaterial && FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.01f))
	{
		// 포스트 프로세스 찌꺼기 정리
		if (PostProcessComponent)
		{
			PostProcessComponent->DestroyComponent();
			PostProcessComponent = nullptr;
		}
        
		// 연출이 다 끝났으므로 큐 액터 최종 파괴
		Destroy();
	}
}

