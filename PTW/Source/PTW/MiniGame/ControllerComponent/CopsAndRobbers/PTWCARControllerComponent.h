#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWCARControllerComponent.generated.h"

/*
 * CopsAndRobbers 미니게임에서 PlayerController에 부착될 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWCARControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWCARControllerComponent();
	UFUNCTION(Client, Reliable)
	void ClientRPC_TargetDestroyNameTag(APlayerState* TargetState);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
