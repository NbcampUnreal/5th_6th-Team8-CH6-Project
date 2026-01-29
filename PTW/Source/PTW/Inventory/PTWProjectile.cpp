#include "PTWProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GAS/PTWWeaponAttributeSet.h"

APTWProjectile::APTWProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CapsuleComponent);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	
	CapsuleComponent->OnComponentHit.AddDynamic(this, &APTWProjectile::OnHit);
	ProjectileMovementComponent->UpdatedComponent = CapsuleComponent;
	
	InitialLifeSpan = 3.0f; 
}

void APTWProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void APTWProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		if (HasAuthority())
		{
			AActor* Shooter = GetInstigator();
			UAbilitySystemComponent* ASC  = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Shooter);
	
			float FinalDamage = 0.0f;
			if (ASC)
			{
				FinalDamage = ASC->GetNumericAttributeChecked(UPTWWeaponAttributeSet::GetDamageAttribute());
			}
			
			TArray<FOverlapResult> OverlapResults;
			
			if (ExplosionOverlapSetter(OverlapResults))
			{
				ApplyExplosionDamage(OverlapResults, FinalDamage);
			}
			
			
			FGameplayCueParameters CueParams;
			CueParams.Location = GetActorLocation();
			ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Explosion")), CueParams);
			
			Destroy();
		}
	}
}

bool APTWProjectile::ExplosionOverlapSetter(TArray<FOverlapResult>& OverlapResults)
{
	FVector ExplosionLocation = GetActorLocation();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRad);
	FCollisionQueryParams CollisionParams;
				
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(GetInstigator());
				
	bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ECC_WeaponAttack,
		SphereShape,
		CollisionParams
		);
				
	DrawDebugSphere(GetWorld(), ExplosionLocation, ExplosionRad, 32, FColor::Red, false, 2.0f);
	
	return bHasOverlap;
}

void APTWProjectile::ApplyExplosionDamage(TArray<FOverlapResult>& OverlapResults, float FinalDamage)
{
	TSet<AActor*> ProcessedActors; // 중복 제거를 위해 사용
	FVector ExplosionLocation = GetActorLocation();
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		
		if (!HitActor || ProcessedActors.Contains(HitActor)) continue;
		ProcessedActors.Add(HitActor);
		
		
		FHitResult ObstarcleHit;
		bool bBlocked = CheckingBlock(ObstarcleHit, ExplosionLocation, HitActor);
		
		if (!bBlocked || ObstarcleHit.GetActor() == HitActor)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
			
			if (TargetASC && DamageSpecHandle.IsValid())
			{
				FGameplayEffectSpec* NewSpec = new FGameplayEffectSpec(*DamageSpecHandle.Data.Get());
				FGameplayEffectSpecHandle NewHandle(NewSpec);
						
				NewHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Damage")), 
				-FinalDamage); 
						
				TargetASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
				
				FGameplayCueParameters CueParams;
				CueParams.Location = HitActor->GetActorLocation();
				
				TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.HitImpact")), CueParams);
			}
		}
	}
}

bool APTWProjectile::CheckingBlock(FHitResult& ObstarcleHit, const FVector ExplosionLocation, const AActor* HitActor)
{
	FVector EyeLocation = ExplosionLocation;
	FVector TargetLocation = HitActor->GetActorLocation() + FVector(0, 0, 50.0f);
		
	FCollisionQueryParams CollisionParams;
				
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(GetInstigator());
		
	bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		ObstarcleHit,
		EyeLocation,
		TargetLocation,
		ECC_Visibility,
		CollisionParams
		);
	
	return bBlocked;
}

