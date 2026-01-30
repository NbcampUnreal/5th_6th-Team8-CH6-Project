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
	if (!HasAuthority() || !OtherActor || OtherActor == GetInstigator()) return;
	
	AActor* Shooter = GetInstigator();
	UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Shooter);
	
	if (!InstigatorASC)
	{
		Destroy();
		return;
	}
	
	float FinalDamage = InstigatorASC->GetNumericAttributeChecked(UPTWWeaponAttributeSet::GetDamageAttribute());
	
	TArray<FOverlapResult> OverlapResults;
	if (ExplosionOverlapSetter(OverlapResults))
	{
		ApplyExplosionDamage(OverlapResults, FinalDamage);
	}
	
	FGameplayCueParameters CueParams;
	CueParams.Location = GetActorLocation();
	CueParams.Instigator = Shooter;
	InstigatorASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.Explosion")), CueParams);
	
	Destroy();
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
	const FVector ExplosionLocation = GetActorLocation();
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		
		if (!HitActor || ProcessedActors.Contains(HitActor)) continue;
		ProcessedActors.Add(HitActor);
		
		FHitResult ObstarcleHit;
		
		if (CheckingBlock(ObstarcleHit, ExplosionLocation, HitActor))
		{
			if (ObstarcleHit.GetActor() != HitActor) continue;
		}
			
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (TargetASC && DamageSpecHandle.IsValid())
		{
			FGameplayEffectSpecHandle NewHandle = TargetASC->MakeOutgoingSpec(
					DamageSpecHandle.Data->Def->GetClass(), 
					1.0f, 
					TargetASC->MakeEffectContext()
			);
			
			if (NewHandle.IsValid())
			{
				NewHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -FinalDamage);
				TargetASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
				
				FGameplayCueParameters TargetCueParams;
				TargetCueParams.Location = HitActor->GetActorLocation();
				TargetCueParams.Instigator = GetInstigator();
				TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Weapon.HitImpact")), TargetCueParams);
			}
		}
	}
	
}

bool APTWProjectile::CheckingBlock(FHitResult& ObstarcleHit, const FVector ExplosionLocation, const AActor* HitActor)
{
	if (!HitActor) return false;
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(GetInstigator());
	
	FVector TargetPoint = HitActor->GetActorLocation() + FVector(0.f, 0.f, 40.f);
	
	return GetWorld()->LineTraceSingleByChannel(
		ObstarcleHit,
		ExplosionLocation,
		TargetPoint,
		ECC_Visibility,
		CollisionParams
	);
}

