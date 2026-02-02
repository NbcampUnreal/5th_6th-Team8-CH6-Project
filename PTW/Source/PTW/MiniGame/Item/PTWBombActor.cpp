// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWBombActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "GAS/PTWBombAttributeSet.h"
#include "GameplayTagContainer.h"

APTWBombActor::APTWBombActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Collision
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(50.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Mesh
	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BombMesh->SetupAttachment(RootComponent);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// GAS
	AbilitySystemComponent =
		CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	BombAttributeSet = CreateDefaultSubobject<UPTWBombAttributeSet>(TEXT("BombAttributeSet"));
}

UAbilitySystemComponent* APTWBombActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APTWBombActor::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// RemainingTime 변화 감지 
	const UPTWBombAttributeSet* BombAS = AbilitySystemComponent->GetSet<UPTWBombAttributeSet>();
	if (BombAS)
	{
		RemainingTimeChangedHandle =
			AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(BombAS->GetRemainingTimeAttribute())
			.AddUObject(this, &APTWBombActor::HandleRemainingTimeChanged);
	}

	if (HasAuthority())
	{
		if (ExplodeAbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(ExplodeAbilityClass, 1, 0));
		}

		// RemainingTime = 30 세팅
		ApplyEffectToSelf(SetTimeEffectClass);

		// 1초마다 RemainingTime -1
		ApplyEffectToSelf(CountdownEffectClass);
	}
}

void APTWBombActor::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!AbilitySystemComponent || !EffectClass) return;

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	Ctx.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Ctx);
	if (Spec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void APTWBombActor::HandleRemainingTimeChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("[Bomb] RemainingTime: %.0f"), Data.NewValue);
	
}

void APTWBombActor::RequestExplode(AActor* InstigatorActor)
{
	if (!HasAuthority())
	{
		ServerRequestExplode(InstigatorActor);
		return;
	}
	
	if (bExplodeRequested) return;
	bExplodeRequested = true;

	SendExplodeEvent(InstigatorActor);
}

void APTWBombActor::ServerRequestExplode_Implementation(AActor* InstigatorActor)
{
	RequestExplode(InstigatorActor);
}

void APTWBombActor::SendExplodeEvent(AActor* InstigatorActor)
{
	if (!AbilitySystemComponent) return;

	const FGameplayTag ExplodeTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Bomb.Explode"));

	FGameplayEventData Payload;
	Payload.EventTag = ExplodeTag;
	Payload.Instigator = InstigatorActor ? InstigatorActor : this;
	Payload.Target = this;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, ExplodeTag, Payload);
}
