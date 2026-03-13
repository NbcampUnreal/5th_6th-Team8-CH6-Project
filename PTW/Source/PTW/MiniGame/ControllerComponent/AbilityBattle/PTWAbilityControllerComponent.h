// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MiniGame/ControllerComponent/PTWBaseControllerComponent.h"
#include "PTWAbilityControllerComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWAbilityControllerComponent : public UPTWBaseControllerComponent
{
	GENERATED_BODY()

public:	
	UPTWAbilityControllerComponent();

	void ShowDraftUI();
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> DraftWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> DraftWidget;
		
};
