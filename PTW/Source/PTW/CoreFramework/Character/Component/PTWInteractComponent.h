// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWInteractComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, const FText&, ActionText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PTW_API UPTWInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWInteractComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* 상호작용 실행 함수 */
	void PerformInteraction();

	/* 상호작용 가능 여부 반환 함수 */
	bool HasValidTarget() const { return CurrentInteractableActor != nullptr; }

	/* [Server] 현재 시점을 기준으로 상호작용 가능한 액터를 즉시 찾아서 반환합니다.*/
	AActor* GetInteractTargetUnsafe();

public:
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableFound OnInteractableFound;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableLost OnInteractableLost;

protected:
	void TraceInteractable();

	void ToggleHighlight(AActor* TargetActor, bool bEnable);
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float InteractionDistance = 300.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	UPROPERTY(VisibleInstanceOnly, Category = "State")
	TObjectPtr<AActor> CurrentInteractableActor;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Visual")
	TObjectPtr<UMaterialInterface> OutlineOverlayMaterial;
	UPROPERTY()
	TObjectPtr<AActor> LastHighlightedActor;

	const int32 StencilValue = 252;
};
