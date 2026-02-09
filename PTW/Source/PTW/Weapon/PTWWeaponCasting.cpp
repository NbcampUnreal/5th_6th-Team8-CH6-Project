
#include "PTWWeaponCasting.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NaniteSceneProxy.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
APTWWeaponCasting::APTWWeaponCasting()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(SceneComponent);
	
	CastingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("CastingMeshComponent");
	CastingMeshComponent->SetupAttachment(SceneComponent);
	
	CastingMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CastingMeshComponent->SetSimulatePhysics(true);
	CastingMeshComponent->SetEnableGravity(true);
	CastingMeshComponent->SetNotifyRigidBodyCollision(true);
	
	CastingMeshComponent->OnComponentHit.AddDynamic(this, &APTWWeaponCasting::OnHit);
}

// Called when the game starts or when spawned
void APTWWeaponCasting::BeginPlay()
{
	Super::BeginPlay();
	
	bIsSoundPlay = false;
	
	if (CastingMeshComponent)
	{
		FVector EjectVelocity = GetActorRightVector() * 300.f; 
		EjectVelocity += GetActorUpVector() * 200.f; 
		EjectVelocity += GetActorForwardVector() * 100.f; 
        
		CastingMeshComponent->SetPhysicsLinearVelocity(EjectVelocity);
	}
	
	
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	
	TWeakObjectPtr<APTWWeaponCasting> WeakObject = this;
	
	TimerDelegate.BindLambda([WeakObject]()
	{
		if (WeakObject.IsValid())
		{
			WeakObject->Destroy();
		}
	});
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, DestroyTimer, false);
}

void APTWWeaponCasting::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (CastingDropSound && !bIsSoundPlay)
	{
		bIsSoundPlay = true;
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			CastingDropSound,
			GetActorLocation(),
			GetActorRotation());
	}
}


