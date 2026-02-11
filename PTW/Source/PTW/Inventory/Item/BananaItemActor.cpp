// Fill out your copyright notice in the Description page of Project Settings.


#include "BananaItemActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ABananaItemActor::ABananaItemActor()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = false;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	SetRootComponent(BoxComponent);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(BoxComponent);
	
	BoxComponent->OnComponentHit.AddDynamic(this, &ABananaItemActor::OnHit);
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ABananaItemActor::OnOverlap);
	
}

void ABananaItemActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (Hit.Normal.Z > 0.7f)
	{
		ProjectileMovementComponent->StopMovementImmediately();
		ProjectileMovementComponent->Deactivate();
		SetActorRotation(Hit.Normal.Rotation());
		
		UE_LOG(LogTemp, Warning, TEXT("BananaItemActor::OnHit"));
	}
}

void ABananaItemActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ProjectileMovementComponent && ProjectileMovementComponent->IsActive())
	{
		return;
	}
	
	if (!HasAuthority())
	{
		return;
	}
	
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Get Owner : %s"), *GetOwner()->GetName());
		
		// TODO : 미끄러짐 효과 적용
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
		if (ASC)
		{
			FGameplayAbilitySpec AbilitySpec(SlippingAbilityClass);
			ASC->GiveAbilityAndActivateOnce(AbilitySpec);
			
			
			UE_LOG(LogTemp, Warning, TEXT("BananaItemActor::OnOverlap"));
			Destroy();
		}
	}
}


