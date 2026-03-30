// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/PTWDeadArea.h"

#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"

// Sets default values
APTWDeadArea::APTWDeadArea()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
}

void APTWDeadArea::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APTWDeadArea::OnComponentBeginOverlap);
}

void APTWDeadArea::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	
	APTWBaseCharacter* PlayerCharacter = Cast<APTWBaseCharacter>(OtherActor);
	if (!PlayerCharacter) return;
	
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(PlayerCharacter);
	if (!CombatInterface) return;

	APTWPlayerState* PlayerState = Cast<APTWPlayerState>(PlayerCharacter->GetPlayerState());
	if (!PlayerState) return;

	UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();
	if (!ASC) return;
	
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	
	CombatInterface->ApplyGameplayEffectToSelf(DeadEffectClass,1,ContextHandle);

	PlayerCharacter->HandleDeath(nullptr);
}




