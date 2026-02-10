// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "PTWOptionsWidget.generated.h"

class UCheckBox;
class UComboBoxString;
class USlider;
class UButton;
class UWidgetSwitcher;
class USoundClass;
class USoundMix;
class UEditableText;

USTRUCT()
struct FOptionSnapshot
{
	GENERATED_BODY()

	FIntPoint Resolution;
	EWindowMode::Type WindowMode;
	int32 ScalabilityLevel;

	float MasterVolume = 1.f;
	float MouseSensitivity = 1.f;
};
/**
 * 
 */
UCLASS()
class PTW_API UPTWOptionsWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* UI 바인딩 */
	// 창모드
	UPROPERTY(meta = (BindWidget))
	UCheckBox* CheckBox_Windowed;
	// 해상도
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Resolution;
	// 그래픽 품질
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* Combo_Quality;
	// 마스터볼륨
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MasterVolume;
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_MasterVolume;
	// 마우스 감도
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MouseSensitivity;
	UPROPERTY(meta = (BindWidget))
	UEditableText* ET_MouseSensitivity;
	// 세이브 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Save;
	// 캔슬 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Cancel;

	/* 카테고리 UI */
	// 카테고리 스위치
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* CategorySwitcher;
	// 그래픽 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Graphics;
	// 사운드 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Sound;
	// 게임설정 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Game;

	/* 에디터에서 생성한 SM_Master를 할당할 변수 */
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundMix* MasterSoundMix;

	/* 에디터에서 생성한 SC_Master를 할당할 변수 */
	UPROPERTY(EditAnywhere, Category = "Settings|Sound")
	USoundClass* MasterSoundClass;

private:
	FOptionSnapshot InitialSnapshot; // 초기값 저장
	bool bIsDirty = false; // 설정 변경 여부

	/* 내부 로직 */

	void PopulateResolutionList();
	void PopulateQualityList();
	void InitializeUIFromCurrentSettings();
	void CacheInitialSettings();
	void ApplyCurrentUIToEngine();
	void RestoreInitialSettings();
	void BindEvents();

	// 숫자 포맷팅
	FText FormatFloatToText(float Value) const;

	/* 설정 변경 이벤트 */

	UFUNCTION()
	void OnCheckWindowedChanged(bool bChecked);

	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);
	UFUNCTION()
	void OnMasterVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);
	UFUNCTION()
	void OnSensitivityCaptureEnd();
	UFUNCTION()
	void OnMouseSensitivityTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnClickedSave();

	UFUNCTION()
	void OnClickedCancel();

	/* 카테고리 전환 */

	UFUNCTION()
	void OnClickedGraphics();

	UFUNCTION()
	void OnClickedSound();

	UFUNCTION()
	void OnClickedGame();
};
