// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWKillLogUI.generated.h"

class UVerticalBox;
class UPTWKillLogEntry;

/**
 * 
 */
UCLASS()
class PTW_API UPTWKillLogUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* 델리게이트 바인드 */
	void BindGameStates();
	void UnbindGameStates();

	/* 델리게이트와 연결될 함수 */
	UFUNCTION()
	void OnKilllogReceived(AActor* DeadActor, AActor* KillerActor);

	/* 킬로그 엔트리 추가 */
	UFUNCTION(BlueprintCallable)
	void AddKillLog(const FString& Killer, const FString& Victim); // 인자에 무기 종류 추가해야함

	/* 로그가 쌓일 컨테이너 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> LogList;

	/* 엔트리 위젯 클래스 (에디터에서 설정) */
	UPROPERTY(EditDefaultsOnly, Category = "KillLog")
	TSubclassOf<UPTWKillLogEntry> KillLogEntryClass;

	/* 최대 유지 개수 */
	UPROPERTY(EditDefaultsOnly, Category = "KillLog")
	int32 MaxLogCount = 5;

	/* 한 줄 유지 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "KillLog")
	float LogLifeTime = 4.0f;

	/* 현재 활성 로그 */
	UPROPERTY()
	TArray<TObjectPtr<UPTWKillLogEntry>> ActiveEntries;

private:
	void HandleEntryExpired(UPTWKillLogEntry* Entry);
	void RemoveOldest();
};
