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

#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Character/Component/PTWUIControllerComponent.h"

#define LOCTEXT_NAMESPACE "BombActor"

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
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	BombAttributeSet = CreateDefaultSubobject<UPTWBombAttributeSet>(TEXT("BombAttributeSet"));
	
	ExplosionChannel = ECC_WeaponAttack;

	//Audio
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;
	AudioLoopComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FuseLoopComponent"));
	AudioLoopComponent->SetupAttachment(RootComponent);
	AudioLoopComponent->bAutoActivate = false;

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

	// 머티리얼 인스턴스 생성 (시각 효과용)
	if (BombMesh)
	{
		BombDynamicMat = BombMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (AudioLoopSound)
	{
		AudioLoopComponent->SetSound(AudioLoopSound);
		AudioLoopComponent->Play();
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		BindToLocalPlayerController();
	}
}

void APTWBombActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		UnBindToLocalPlayerController();
	}

	Super::EndPlay(EndPlayReason);
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
	//UE_LOG(LogTemp, Warning, TEXT("[Bomb] RemainingTime: %.0f"), Data.NewValue);

	UpdateBombEffects(Data.NewValue);
}

void APTWBombActor::SetBombOwner(APawn* NewOwnerPawn)
{
	if (!HasAuthority()) return;

	const FString OldName = BombOwnerPawn && BombOwnerPawn->GetPlayerState()
		? BombOwnerPawn->GetPlayerState()->GetPlayerName() : TEXT("None");

	const FString NewName = NewOwnerPawn && NewOwnerPawn->GetPlayerState()
		? NewOwnerPawn->GetPlayerState()->GetPlayerName() : TEXT("None");
	
	bTimeExpiredNotified = false;

	BombOwnerPawn = NewOwnerPawn;
	OnRep_BombOwnerPawn();

	OnBombOwnerChanged.Broadcast(BombOwnerPawn);
}

void APTWBombActor::OnRep_BombOwnerPawn()
{
	if (BombOwnerPawn)
	{
		APlayerState* PS = BombOwnerPawn->GetPlayerState();
		const FString Name = PS ? PS->GetPlayerName() : TEXT("Unknown");
		//UE_LOG(LogTemp, Warning, TEXT("[Bomb] Owner Changed -> PlayerState: %s"), *Name);
	}

	AttachToOwnerPawn();

	if (BombOwnerPawn)
	{
		if (AController* OwnerController = BombOwnerPawn->GetController())
		{
			if (APTWPlayerController* Controller = Cast<APTWPlayerController>(OwnerController))
			{
				Controller->SendMessage(
					LOCTEXT("GetBomb", "GetBomb!"),
					ENotificationPriority::High,
					3.f);
			}
		}
	}

	OnBombOwnerChanged.Broadcast(BombOwnerPawn);
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

void APTWBombActor::Multicast_PlayExplosionCue_Implementation(const FVector& Loc, AActor* InstigatorActor)
{

	if (AbilitySystemComponent && ExplosionCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location   = Loc;
		CueParams.Instigator = InstigatorActor ? InstigatorActor : this;

		AbilitySystemComponent->ExecuteGameplayCue(ExplosionCueTag, CueParams);
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

	if (GetNetMode() != NM_DedicatedServer)
	{
		UnBindToLocalPlayerController();
	}

	// 오버랩 수집
	TArray<FOverlapResult> OverlapResults;
	ExplosionOverlapSetter(OverlapResults);

	// 데미지 적용
	const float FinalDamage = BaseBombDamage;
	ApplyExplosionDamage(OverlapResults, FinalDamage, InstigatorActor);
	
	SetActorEnableCollision(false);
	
	if (AudioComponent && AudioComponent->IsPlaying()) AudioComponent->Stop();
	if (AudioLoopComponent && AudioLoopComponent->IsPlaying()) AudioLoopComponent->Stop();
	
	SetLifeSpan(0.2f);
	
	ForceNetUpdate();
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

	const bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ExplosionChannel,
		SphereShape,
		CollisionParams
	);

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

	if (BombOwnerPawn && BombOwnerPawn->GetPlayerState())
	{
		if (UAbilitySystemComponent* OwnerASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(BombOwnerPawn))
		{
			FGameplayEffectContextHandle OwnerContext = OwnerASC->MakeEffectContext();
			OwnerContext.AddSourceObject(this);

			FGameplayEffectSpecHandle OwnerHandle = OwnerASC->MakeOutgoingSpec(
				DamageEffectClass,
				1.0f,
				OwnerContext
			);

			if (OwnerHandle.IsValid())
			{
				OwnerHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, 9999.f);
				OwnerASC->ApplyGameplayEffectSpecToSelf(*OwnerHandle.Data.Get());

				ProcessedActors.Add(BombOwnerPawn);
				
				if (HitImpactCueTag.IsValid())
				{
					FGameplayCueParameters OwnerCueParams;
					OwnerCueParams.Location = BombOwnerPawn->GetActorLocation();
					OwnerCueParams.Instigator = InstigatorActor ? InstigatorActor : this;

					OwnerASC->ExecuteGameplayCue(HitImpactCueTag, OwnerCueParams);
				}
			}
		}
	}
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || ProcessedActors.Contains(HitActor)) continue;
		if (HitActor == this) continue;

		APawn* HitPawn = Cast<APawn>(HitActor);
		if (!HitPawn) continue;

		if (!HitPawn->GetPlayerState()) continue;

		ProcessedActors.Add(HitActor);

		// 엄폐 확인
		FHitResult ObstacleHit;
		if (CheckingBlock(ObstacleHit, ExplosionLocation, HitActor))
		{
			if (ObstacleHit.GetActor() != HitActor) continue;
		}

		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;

		FGameplayEffectContextHandle TargetContext = TargetASC->MakeEffectContext();
		TargetContext.AddSourceObject(this);

		FGameplayEffectSpecHandle NewHandle = TargetASC->MakeOutgoingSpec(
			DamageEffectClass,
			1.0f,
			TargetContext
		);
		if (!NewHandle.IsValid()) continue;

		NewHandle.Data->SetSetByCallerMagnitude(DamageSetByCallerTag, FinalDamage);
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

void APTWBombActor::UpdateBombEffects(float NewTime)
{
	if (NewTime <= 0.0f)
	{
		if (AudioComponent && AudioComponent->IsPlaying()) AudioComponent->Stop();
		if (AudioLoopComponent && AudioLoopComponent->IsPlaying()) AudioLoopComponent->Stop();
		if (BombDynamicMat) BombDynamicMat->SetScalarParameterValue(FName("BlinkSpeed"), 0.0f);
		
		if (HasAuthority() && !bTimeExpiredNotified)
		{
			bTimeExpiredNotified = true;
			
			AActor* InstigatorActor = BombOwnerPawn ? Cast<AActor>(BombOwnerPawn) : this;
			
			if (BombOwnerPawn && ExplosionCueTag.IsValid())
			{
				UAbilitySystemComponent* OwnerASC =
					UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(BombOwnerPawn);

				if (OwnerASC)
				{
					FGameplayCueParameters Params;
					Params.Location     = GetActorLocation();
					Params.Instigator   = InstigatorActor;
					Params.SourceObject = this;

					OwnerASC->ExecuteGameplayCue(ExplosionCueTag, Params);
				}
			}
			
			RequestExplode(InstigatorActor);
			
			OnBombTimeExpired.Broadcast(InstigatorActor);
		}

		return;
	}

	const float MaxTime = 10.0f;
	const float Interval = 2.0f;

	int32 NewPhaseIndex = FMath::FloorToInt((MaxTime - NewTime) / Interval);

	if (!CountdownSounds.IsValidIndex(NewPhaseIndex)) return;

	bool bNeedUpdate = (NewPhaseIndex != CurrentSoundPhaseIndex);

	if (!bNeedUpdate && AudioComponent->IsPlaying())
	{
		float SegmentStartTime = MaxTime - (NewPhaseIndex * Interval);
		float ExpectedPlaybackTime = SegmentStartTime - NewTime;
	}

	if (bNeedUpdate || !AudioComponent->IsPlaying())
	{
		USoundBase* TargetSound = CountdownSounds[NewPhaseIndex];

		if (AudioComponent->Sound != TargetSound)
		{
			AudioComponent->SetSound(TargetSound);
		}

		float PhaseStartTime = MaxTime - (NewPhaseIndex * Interval);

		float StartOffset = FMath::Max(0.0f, PhaseStartTime - NewTime);

		AudioComponent->Play(StartOffset);

		CurrentSoundPhaseIndex = NewPhaseIndex;
	}

	if (BombDynamicMat)
	{
		float BlinkSpeed = FMath::GetMappedRangeValueClamped(FVector2D(10.f, 0.f), FVector2D(2.0f, 16.0f), NewTime);
		BombDynamicMat->SetScalarParameterValue(FName("BlinkSpeed"), BlinkSpeed);
	}
}

void APTWBombActor::BindToLocalPlayerController()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC);
	if (!PTWPC) return;

	if (PTWPC->UIControllerComponent)
	{
		PTWPC->UIControllerComponent->BindBombDelegate(this);
	}

	// 현재 오너 즉시 동기화
	if (BombOwnerPawn)
	{
		OnBombOwnerChanged.Broadcast(BombOwnerPawn);
	}
}

void APTWBombActor::UnBindToLocalPlayerController()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC);
	if (!PTWPC) return;

	if (PTWPC->UIControllerComponent)
	{
		PTWPC->UIControllerComponent->UnBindBombDelegate();
	}
}

#undef LOCTEXT_NAMESPACE
