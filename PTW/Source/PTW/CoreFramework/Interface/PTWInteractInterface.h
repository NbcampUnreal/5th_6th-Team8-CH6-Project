// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTWInteractInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPTWInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class PTW_API IPTWInteractInterface
{
	GENERATED_BODY()

public:
	/** * 상호작용 수행 함수
	 * @param InstigatorPawn : 상호작용을 시도한 캐릭터
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(APawn* InstigatorPawn);

	/* UI에 띄울 상호작용 텍스트 반환 (예: "열기", "줍기") */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionKeyword();

	/* 현재 상호작용이 가능한 상태인지 확인 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool IsInteractable();
};
