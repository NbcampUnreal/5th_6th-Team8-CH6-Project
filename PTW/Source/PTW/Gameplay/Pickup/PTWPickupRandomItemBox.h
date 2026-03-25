// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/PTWPickupItemBase.h"
#include "PTWPickupRandomItemBox.generated.h"

UCLASS()
class PTW_API APTWPickupRandomItemBox : public APTWPickupItemBase
{
	GENERATED_BODY()

public:
	APTWPickupRandomItemBox();
protected:
	virtual void BeginPlay() override;
	virtual void OnPickedUp(class APTWPlayerCharacter* Player);
	
};
