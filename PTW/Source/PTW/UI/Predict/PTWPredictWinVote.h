// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWPredictWinVote.generated.h"

class UVerticalBox;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class PTW_API UPTWPredictWinVote : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	// 상단에 선택된 이름을 보여줄 텍스트 (BindWidget)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SelectedPlayer;

	// 플레이어 버튼들이 들어갈 컨테이너
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PlayerButtonList;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_OK;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Cancel;

	// 개별 플레이어 버튼 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PlayerEntryClass;

private:
	UFUNCTION()
	void OnOKClicked();

	UFUNCTION()
	void OnCancelClicked();

	// 버튼 클릭 시 호출될 함수 (이름 업데이트용)
	void SetSelectedPlayer(const FString& PlayerName);

	FString CurrentSelectedName;
};
