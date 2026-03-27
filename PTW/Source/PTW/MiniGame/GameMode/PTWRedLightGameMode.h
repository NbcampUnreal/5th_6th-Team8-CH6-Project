// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "GameplayTagContainer.h"
#include "PTWRedLightGameMode.generated.h"

class APTWRedLightCharacter;
class UPTWItemDefinition;
class UGameplayEffect;

UCLASS()
class PTW_API APTWRedLightGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight")
	TSubclassOf<APTWRedLightCharacter> TaggerClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Weapon")
	TObjectPtr<UPTWItemDefinition> TaggerWeaponDef;

	virtual void StartRound() override;

	UPROPERTY(EditDefaultsOnly, Category = "RedLight")
	float MaxAllowedSpeed = 10.0f;

	UFUNCTION(BlueprintCallable, Category = "RedLight")
	void AssignTagger(APlayerController* TaggerPC);

	void OnRedLightStateChanged(bool bIsRedLight, APTWRedLightCharacter* TaggerChar);
	bool IsPlayerCaught(ACharacter* PlayerToCheck) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Combat")
	TSubclassOf<UGameplayEffect> InvincibleEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Combat")
	FGameplayTag InvincibleTag;

	void PlayerFinished(ACharacter* FinishedPlayer);

	UFUNCTION(BlueprintImplementableEvent, Category = "RedLight|Events")
	void ReceiveOnPhaseChanged(bool bIsRedLight);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RedLight|Classes")
	TSubclassOf<AActor> CachedBlueprintClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Classes")
	TObjectPtr<AActor> CachedBlueprintInstance;

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void BeginPlay() override;

protected:
	FTimerHandle MovementCheckTimer;

	UPROPERTY()
	APTWRedLightCharacter* CurrentTagger;

	UPROPERTY()
	TSet<ACharacter*> CaughtPlayers;

	void CheckPlayerMovements();


};
