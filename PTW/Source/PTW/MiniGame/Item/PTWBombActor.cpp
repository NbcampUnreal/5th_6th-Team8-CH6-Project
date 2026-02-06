// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWBombActor.h"

#include "PTW.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"

#include "GAS/PTWBombAttributeSet.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

APTWBombActor::APTWBombActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

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
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	BombAttributeSet = CreateDefaultSubobject<UPTWBombAttributeSet>(TEXT("BombAttributeSet"));
	
	ExplosionChannel = ECC_WeaponAttack;

	DamageSetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
	ExplosionCueTag      = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Explosion"));
	HitImpactCueTag      = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.HitImpact"));
}

void APTWBombActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWBombActor, BombOwnerPawn);
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
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(BombAS->GetRemainingTimeAttribute())
			.AddUObject(this, &APTWBombActor::HandleRemainingTimeChanged);
	}

	// 타이머
	if (HasAuthority())
	{
		ApplyEffectToSelf(SetTimeEffectClass);
		ApplyEffectToSelf(CountdownEffectClass);
	}

	if (BombOwnerPawn)
	{
		AttachToOwnerPawn();
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

void APTWBombActor::SetBombOwner(APawn* NewOwnerPawn)
{
	if (!HasAuthority()) return;

	BombOwnerPawn = NewOwnerPawn;
	OnRep_BombOwnerPawn();
}

void APTWBombActor::OnRep_BombOwnerPawn()
{
	if (BombOwnerPawn)
	{
		APlayerState* PS = BombOwnerPawn->GetPlayerState();
		const FString Name = PS ? PS->GetPlayerName() : TEXT("Unknown");
		UE_LOG(LogTemp, Warning, TEXT("[Bomb] Owner Changed -> PlayerState: %s"), *Name);
	}

	AttachToOwnerPawn();
}

void APTWBombActor::AttachToOwnerPawn()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if (!BombOwnerPawn) return;

	USkeletalMeshComponent* SkelMesh = BombOwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
	if (SkelMesh && SkelMesh->DoesSocketExist(TEXT("BombHeadSocket")))
	{
		AttachToComponent(
			SkelMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("BombHeadSocket")
		);
	}
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

	// 오버랩 수집
	TArray<FOverlapResult> OverlapResults;
	ExplosionOverlapSetter(OverlapResults);

	// 데미지 적용
	const float FinalDamage = BaseBombDamage;
	ApplyExplosionDamage(OverlapResults, FinalDamage, InstigatorActor);

	// 폭발 큐 실행
	if (AbilitySystemComponent && ExplosionCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		CueParams.Instigator = InstigatorActor ? InstigatorActor : this;

		AbilitySystemComponent->ExecuteGameplayCue(ExplosionCueTag, CueParams);
	}

	Destroy();
}

void APTWBombActor::ServerRequestExplode_Implementation(AActor* InstigatorActor)
{
	RequestExplode(InstigatorActor);
}

bool APTWBombActor::ExplosionOverlapSetter(TArray<FOverlapResult>& OverlapResults)
{
	const FVector ExplosionLocation = GetActorLocation();
	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRad);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	if (BombOwnerPawn) CollisionParams.AddIgnoredActor(BombOwnerPawn);

	const bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ExplosionChannel,
		SphereShape,
		CollisionParams
	);

	DrawDebugSphere(GetWorld(), ExplosionLocation, ExplosionRad, 32, FColor::Red, false, 2.0f);
	return bHasOverlap;
}

bool APTWBombActor::CheckingBlock(FHitResult& OutHit, const FVector ExplosionLocation, AActor* HitActor)
{
	if (!HitActor) return false;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (BombOwnerPawn) Params.AddIgnoredActor(BombOwnerPawn);

	const FVector TargetPoint = HitActor->GetActorLocation() + FVector(0.f, 0.f, 40.f);

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		ExplosionLocation,
		TargetPoint,
		ECC_Visibility,
		Params
	);
}

void APTWBombActor::ApplyExplosionDamage(TArray<FOverlapResult>& OverlapResults, float FinalDamage, AActor* InstigatorActor)
{
	if (!DamageEffectClass) return;

	TSet<AActor*> ProcessedActors;
	const FVector ExplosionLocation = GetActorLocation();

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || ProcessedActors.Contains(HitActor)) continue;

		// 자기 자신 / 소유자 제외
		if (HitActor == this || HitActor == BombOwnerPawn) continue;

		ProcessedActors.Add(HitActor);

		// 엄폐 확인
		FHitResult ObstacleHit;
		if (CheckingBlock(ObstacleHit, ExplosionLocation, HitActor))
		{
			if (ObstacleHit.GetActor() != HitActor) continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;

		FGameplayEffectSpecHandle NewHandle = TargetASC->MakeOutgoingSpec(
			DamageEffectClass,
			1.0f,
			TargetASC->MakeEffectContext()
		);
		if (!NewHandle.IsValid()) continue;

		NewHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, -FinalDamage);
		TargetASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());

		// 피격 큐
		if (HitImpactCueTag.IsValid())
		{
			FGameplayCueParameters TargetCueParams;
			TargetCueParams.Location = HitActor->GetActorLocation();
			TargetCueParams.Instigator = InstigatorActor ? InstigatorActor : this;

			TargetASC->ExecuteGameplayCue(HitImpactCueTag, TargetCueParams);
		}
	}
}
