// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWLobbyPortal.generated.h"

enum class EPTWGamePhase : uint8;
class APTWGameState;
class USphereComponent;
/**
 * 로비에서 플레이어의 포탈 진입/이탈을 감지하는 액터
 * - 오버랩된 플레이어를 추적하여 현재 포탈 진입 인원 집계
 * - 포탈 활성/비활성 상태를 복제하여 클라이언트에 동기화
 */
UCLASS()
class PTW_API APTWLobbyPortal : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APTWLobbyPortal();
	
	/** 포탈 사용 가능 여부 설정(서버에서 호출) */
	void SetPortalEnabled(bool bEnable);
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);
	
	// 현재 포탈 내 플레이어 수 갱신(예: UI/상태 반영)
	void UpdatePortalCount();
	
	// 포탈 활성/비활성 적용(시각/충돌/상호작용 등)
	void ApplyPortalEnabled(bool bEnable);
	
	// OtherActor로부터 PlayerState를 안전하게 추출
	bool GetOverlappingPlayerState(AActor* OtherActor, APlayerState*& OutPlayerState) const ;
	
	// 포탈 진입/이탈 공통 처리(추적 Set 갱신)
	void PortalOverlap(AActor* OtherActor, bool bEnter);

	UFUNCTION()
	void PortalEnable(EPTWGamePhase GamePhase);
	
	UPROPERTY(VisibleAnywhere, Category = "Component");
	TObjectPtr<USphereComponent> SphereCollision;
	
	// 현재 포탈 영역에 들어와 있는 플레이어들
	TSet<TObjectPtr<APlayerState>> PlayerInPortal;
	
	UPROPERTY(ReplicatedUsing=OnRep_PortalEnabled)
	bool bPortalEnabled;

	UFUNCTION()
	void OnRep_PortalEnabled();

	
};
