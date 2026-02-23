// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PTWOptionsWidget.h"

#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

#include "CoreFramework/PTWGameUserSettings.h"
#include "CoreFramework/PTWPlayerController.h"
#include "UI/PTWUISubsystem.h"

void UPTWOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	PopulateResolutionList();
	PopulateQualityList();
	InitializeUIFromCurrentSettings();
	CacheInitialSettings();
	BindEvents();

	if (CategorySwitcher)
	{
		CategorySwitcher->SetActiveWidgetIndex(0);
	}
}

void UPTWOptionsWidget::NativeDestruct()
{
	if (bIsDirty)
	{
		RestoreInitialSettings();
	}

	Super::NativeDestruct();
}

void UPTWOptionsWidget::PopulateResolutionList()
{
	if (!Combo_Resolution) return;

	Combo_Resolution->ClearOptions();

	TArray<FIntPoint> CommonResolutions =
	{
		FIntPoint(1280, 720),
		FIntPoint(1600, 900),
		FIntPoint(1920, 1080),
		FIntPoint(2560, 1440),
		FIntPoint(3840, 2160)
	};

	for (const FIntPoint& Res : CommonResolutions)
	{
		FString Option = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
		Combo_Resolution->AddOption(Option);
	}
}

void UPTWOptionsWidget::PopulateQualityList()
{
	TArray<UComboBoxString*> QualityCombos = {
		Combo_Quality, Combo_ViewDistance, Combo_Shadow, Combo_Texture,
		Combo_Effects, Combo_PostProcess, Combo_Foliage, Combo_Shading, Combo_AntiAliasing
	};

	for (UComboBoxString* Combo : QualityCombos)
	{
		if (!Combo) continue;
		Combo->ClearOptions();
		Combo->AddOption(NSLOCTEXT("Options", "Low", "Low").ToString());
		Combo->AddOption(NSLOCTEXT("Options", "Medium", "Medium").ToString());
		Combo->AddOption(NSLOCTEXT("Options", "High", "High").ToString());
		Combo->AddOption(NSLOCTEXT("Options", "Epic", "Epic").ToString());
	}
}

void UPTWOptionsWidget::InitializeUIFromCurrentSettings()
{
	if (!GEngine) return;

	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	if (CheckBox_Windowed)
	{
		CheckBox_Windowed->SetIsChecked(
			Settings->GetFullscreenMode() == EWindowMode::Windowed);
	}

	if (Combo_Resolution)
	{
		FIntPoint Res = Settings->GetScreenResolution();
		FString ResString = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
		Combo_Resolution->SetSelectedOption(ResString);
	}

	// 세부 그래픽 설정
	if (Combo_ViewDistance) Combo_ViewDistance->SetSelectedIndex(Settings->GetViewDistanceQuality());
	if (Combo_Shadow)       Combo_Shadow->SetSelectedIndex(Settings->GetShadowQuality());
	if (Combo_Texture)      Settings->GetTextureQuality() != -1 ? Combo_Texture->SetSelectedIndex(Settings->GetTextureQuality()) : Combo_Texture->SetSelectedIndex(0);
	if (Combo_Effects)      Combo_Effects->SetSelectedIndex(Settings->GetVisualEffectQuality());
	if (Combo_PostProcess)  Combo_PostProcess->SetSelectedIndex(Settings->GetPostProcessingQuality());
	if (Combo_Foliage)      Combo_Foliage->SetSelectedIndex(Settings->GetFoliageQuality());
	if (Combo_Shading)      Combo_Shading->SetSelectedIndex(Settings->GetShadingQuality());
	if (Combo_AntiAliasing) Combo_AntiAliasing->SetSelectedIndex(Settings->GetAntiAliasingQuality());
	if (CheckBox_VSync)     CheckBox_VSync->SetIsChecked(Settings->bUseVSync);

	if (Combo_Quality)
	{
		int32 FirstVal = Settings->GetViewDistanceQuality();
		bool bIsCustom = false;

		TArray<int32> Qualities = {
			Settings->GetShadowQuality(),
			Settings->GetTextureQuality(),
			Settings->GetVisualEffectQuality(),
			Settings->GetPostProcessingQuality(),
			Settings->GetFoliageQuality(),
			Settings->GetShadingQuality(),
			Settings->GetAntiAliasingQuality()
		};

		for (int32 Val : Qualities)
		{
			if (Val != FirstVal)
			{
				bIsCustom = true;
				break;
			}
		}

		if (bIsCustom)
		{
			FString CustomText = NSLOCTEXT("Options", "Custom", "Custom").ToString();
			if (Combo_Quality->FindOptionIndex(CustomText) == INDEX_NONE)
			{
				Combo_Quality->AddOption(CustomText);
			}
			Combo_Quality->SetSelectedOption(CustomText);
		}
		else
		{
			// 모든 세부 설정이 같으면 해당 레벨 표시 (4 이상인 경우 Epic(3)으로 클램프)
			int32 SafeLevel = FMath::Clamp(FirstVal, 0, 3);
			Combo_Quality->SetSelectedIndex(SafeLevel);
		}
	}

	if (Slider_MasterVolume && ET_MasterVolume)
	{
		Slider_MasterVolume->SetValue(Settings->MasterVolume);
		ET_MasterVolume->SetText(FormatFloatToText(Settings->MasterVolume)); // 초기 텍스트 세팅
	}

	if (Slider_MouseSensitivity && ET_MouseSensitivity)
	{
		Slider_MouseSensitivity->SetValue(Settings->MouseSensitivity);
		ET_MouseSensitivity->SetText(FormatFloatToText(Settings->MouseSensitivity)); // 초기 텍스트 세팅
	}
}

void UPTWOptionsWidget::CacheInitialSettings()
{
	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	InitialSnapshot.Resolution = Settings->GetScreenResolution();
	InitialSnapshot.WindowMode = Settings->GetFullscreenMode();
	InitialSnapshot.ScalabilityLevel = Settings->GetOverallScalabilityLevel();
	InitialSnapshot.ViewDistanceQuality = Settings->GetViewDistanceQuality();
	InitialSnapshot.ShadowQuality = Settings->GetShadowQuality();
	InitialSnapshot.TextureQuality = Settings->GetTextureQuality();
	InitialSnapshot.EffectsQuality = Settings->GetVisualEffectQuality();
	InitialSnapshot.PostProcessQuality = Settings->GetPostProcessingQuality();
	InitialSnapshot.FoliageQuality = Settings->GetFoliageQuality();
	InitialSnapshot.ShadingQuality = Settings->GetShadingQuality();
	InitialSnapshot.AntiAliasingQuality = Settings->GetAntiAliasingQuality();
	InitialSnapshot.bVSync = Settings->bUseVSync;
	InitialSnapshot.MasterVolume = Settings->MasterVolume;
	InitialSnapshot.MouseSensitivity = Settings->MouseSensitivity;

	bIsDirty = false;
}

void UPTWOptionsWidget::UpdateDisplaySettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	// 해상도 로직
	FString Left, Right;
	if (Combo_Resolution && Combo_Resolution->GetSelectedOption().Split(TEXT(" x "), &Left, &Right))
	{
		Settings->SetScreenResolution(FIntPoint(FCString::Atoi(*Left), FCString::Atoi(*Right)));
	}

	if (CheckBox_Windowed) Settings->SetFullscreenMode(CheckBox_Windowed->IsChecked() ? EWindowMode::Windowed : EWindowMode::Fullscreen);
	if (CheckBox_VSync) Settings->SetVSyncEnabled(CheckBox_VSync->IsChecked());

	Settings->ApplySettings(false);
	bIsDirty = true;
}

void UPTWOptionsWidget::UpdateAudioSettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings || !Slider_MasterVolume) return;

	//  설정값 업데이트
	float NewVolume = Slider_MasterVolume->GetValue();
	Settings->MasterVolume = NewVolume;

	// 사운드 믹스를 통한 실시간 볼륨 조절
	if (MasterSoundMix && MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			MasterSoundMix,
			MasterSoundClass,
			NewVolume, // 볼륨 (0.0 ~ 1.0)
			1.0f,      // 피치
			0.0f,      // 페이드 인 시간 (0 = 즉시 반영)
			true       // 기존 설정을 무시하고 강제 적용
		);

		// 변경된 믹스 푸시
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);
	}

	bIsDirty = true;
}

void UPTWOptionsWidget::UpdateInputSettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings || !Slider_MouseSensitivity) return;

	float NewSensitivity = Slider_MouseSensitivity->GetValue();
	Settings->MouseSensitivity = NewSensitivity;

	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		PC->ApplyMouseSensitivity(NewSensitivity);
	}

	bIsDirty = true;
}

void UPTWOptionsWidget::UpdateScalabilitySettings()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	// 세부 품질 적용
	if (Combo_ViewDistance) Settings->SetViewDistanceQuality(Combo_ViewDistance->GetSelectedIndex());
	if (Combo_Shadow)       Settings->SetShadowQuality(Combo_Shadow->GetSelectedIndex());
	if (Combo_Texture)      Settings->SetTextureQuality(Combo_Texture->GetSelectedIndex());
	if (Combo_Effects)      Settings->SetVisualEffectQuality(Combo_Effects->GetSelectedIndex());
	if (Combo_PostProcess)  Settings->SetPostProcessingQuality(Combo_PostProcess->GetSelectedIndex());
	if (Combo_Foliage)      Settings->SetFoliageQuality(Combo_Foliage->GetSelectedIndex());
	if (Combo_Shading)      Settings->SetShadingQuality(Combo_Shading->GetSelectedIndex());
	if (Combo_AntiAliasing) Settings->SetAntiAliasingQuality(Combo_AntiAliasing->GetSelectedIndex());

	// Custom 여부 판별 로직 - 모든 세부 설정 값이 동일한지 확인
	int32 FirstVal = Settings->GetViewDistanceQuality();
	bool bIsCustom = false;

	TArray<int32> Qualities = {
		Settings->GetShadowQuality(),
		Settings->GetTextureQuality(),
		Settings->GetVisualEffectQuality(),
		Settings->GetPostProcessingQuality(),
		Settings->GetFoliageQuality(),
		Settings->GetShadingQuality(),
		Settings->GetAntiAliasingQuality()
	};

	for (int32 Val : Qualities)
	{
		if (Val != FirstVal)
		{
			bIsCustom = true;
			break;
		}
	}

	// 전체 품질 UI 업데이트
	if (Combo_Quality)
	{
		FString CustomText = NSLOCTEXT("Options", "Custom", "Custom").ToString();

		if (bIsCustom)
		{
			// 목록에 "Custom"이 없다면 추가
			if (Combo_Quality->FindOptionIndex(CustomText) == INDEX_NONE)
			{
				Combo_Quality->AddOption(CustomText);
			}
			Combo_Quality->SetSelectedOption(CustomText);
		}
		else
		{
			// 모든 값이 같아지면 "Custom" 옵션을 리스트에서 제거하여 선택 불가능하게 만들기
			Combo_Quality->RemoveOption(CustomText);

			Combo_Quality->SetSelectedIndex(FirstVal);
			Settings->SetOverallScalabilityLevel(FirstVal);
		}
	}

	// Settings->ApplySettings(false);
	bIsDirty = true;
}

void UPTWOptionsWidget::RestoreInitialSettings()
{
	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	// 디스플레이 복구
	Settings->SetScreenResolution(InitialSnapshot.Resolution);
	Settings->SetFullscreenMode(InitialSnapshot.WindowMode);

	// 프리셋 복구
	Settings->SetOverallScalabilityLevel(InitialSnapshot.ScalabilityLevel);

	// 세부 그래픽 옵션 복구
	Settings->SetViewDistanceQuality(InitialSnapshot.ViewDistanceQuality);
	Settings->SetShadowQuality(InitialSnapshot.ShadowQuality);
	Settings->SetTextureQuality(InitialSnapshot.TextureQuality);
	Settings->SetVisualEffectQuality(InitialSnapshot.EffectsQuality);
	Settings->SetPostProcessingQuality(InitialSnapshot.PostProcessQuality);
	Settings->SetFoliageQuality(InitialSnapshot.FoliageQuality);
	Settings->SetShadingQuality(InitialSnapshot.ShadingQuality);
	Settings->SetAntiAliasingQuality(InitialSnapshot.AntiAliasingQuality);

	// VSync 복구
	Settings->SetVSyncEnabled(InitialSnapshot.bVSync);

	// 오디오 복구
	Settings->MasterVolume = InitialSnapshot.MasterVolume;

	// 마우스 감도 복구
	Settings->MouseSensitivity = InitialSnapshot.MouseSensitivity;

	/* (옵션창 취소 시 되돌리기) */
	// 사운드 반영
	if (MasterSoundMix && MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			MasterSoundMix,
			MasterSoundClass,
			InitialSnapshot.MasterVolume,
			1.0f,
			0.0f,
			true
		);

		UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);
	}

	// 마우스 감도 반영
	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		PC->ApplyMouseSensitivity(InitialSnapshot.MouseSensitivity);
	}

	Settings->ApplySettings(false);
	InitializeUIFromCurrentSettings();
	bIsDirty = false;
}

void UPTWOptionsWidget::BindEvents()
{
	if (CheckBox_Windowed)
	{
		CheckBox_Windowed->OnCheckStateChanged.AddDynamic(this, &UPTWOptionsWidget::OnCheckWindowedChanged);
	}

	if (Combo_Resolution)
	{
		Combo_Resolution->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnResolutionChanged);
	}

	if (Combo_Quality)
	{
		Combo_Quality->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnQualityChanged);
	}

	TArray<UComboBoxString*> DetailCombos = {
	Combo_ViewDistance, Combo_Shadow, Combo_Texture,
	Combo_Effects, Combo_PostProcess, Combo_Foliage,
	Combo_Shading, Combo_AntiAliasing
	};
	for (UComboBoxString* Combo : DetailCombos)
	{
		if (Combo)
		{
			// OnQualityChanged는 내부에서 UpdateScalabilitySettings()를 호출
			Combo->OnSelectionChanged.AddDynamic(this, &UPTWOptionsWidget::OnQualityChanged);
		}
	}

	if (CheckBox_VSync)
	{
		CheckBox_VSync->OnCheckStateChanged.AddDynamic(this, &UPTWOptionsWidget::OnVSyncChanged);
	}

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnMasterVolumeChanged);
	}
	if (ET_MasterVolume)
	{
		ET_MasterVolume->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnMasterVolumeTextCommitted);
	}

	if (Slider_MouseSensitivity)
	{
		// 드래그 중에는 값만 업데이트
		Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnMouseSensitivityChanged);

		// 마우스를 뗄 때 엔진에 최종 적용
		Slider_MouseSensitivity->OnMouseCaptureEnd.AddDynamic(this, &UPTWOptionsWidget::OnSensitivityCaptureEnd);
	}
	if (ET_MouseSensitivity)
	{
		ET_MouseSensitivity->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnMouseSensitivityTextCommitted);
	}

	if (Button_Save)
	{
		Button_Save->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedSave);
	}

	if (Button_Cancel)
	{
		Button_Cancel->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedCancel);
	}

	if (Button_Graphics)
	{
		Button_Graphics->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedGraphics);
	}

	if (Button_Sound)
	{
		Button_Sound->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedSound);
	}

	if (Button_Game)
	{
		Button_Game->OnClicked.AddDynamic(
			this, &UPTWOptionsWidget::OnClickedGame);
	}
}

bool UPTWOptionsWidget::ValidateAndApplyTextEntry(const FText& InText, USlider* TargetSlider, UEditableText* TargetET, float MinValue, float MaxValue)
{
	if (!TargetSlider || !TargetET) return false;

	FString StringValue = InText.ToString();

	// 공백 체크
	if (StringValue.IsEmpty())
	{
		TargetET->SetText(FormatFloatToText(TargetSlider->GetValue()));
		return false;
	}

	// 숫자 변환 시도
	float NewValue = FCString::Atof(*StringValue);

	// 예외 케이스 처리: 변환값이 0인데 입력이 "0"이 아닌 경우 (즉, 문자 입력)
	if (NewValue == 0.0f && !StringValue.Equals(TEXT("0")) && !StringValue.Equals(TEXT("0.0")))
	{
		// 잘못된 입력일 경우 현재 슬라이더의 실제 값을 가져와 텍스트 박스에 다시 덮어씌움
		TargetET->SetText(FormatFloatToText(TargetSlider->GetValue()));
		return false;
	}

	// 유효한 입력인 경우: 범위 제한 후 슬라이더 및 텍스트 업데이트
	NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);
	TargetSlider->SetValue(NewValue);

	// 입력을 포맷에 맞춰 재출력 (예: 사용자가 0.5555 입력 시 0.56으로 보정 표시)
	TargetET->SetText(FormatFloatToText(NewValue));

	return true;
}

FText UPTWOptionsWidget::FormatFloatToText(float Value) const
{
	FNumberFormattingOptions NumberOptions = FNumberFormattingOptions::DefaultNoGrouping();
	NumberOptions.SetMaximumFractionalDigits(1);
	NumberOptions.SetMinimumFractionalDigits(1);

	return FText::AsNumber(Value, &NumberOptions);
}

void UPTWOptionsWidget::OnCheckWindowedChanged(bool bChecked)
{
	UpdateDisplaySettings();
}

void UPTWOptionsWidget::OnResolutionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	// 초기화 과정에서 호출되는 것 방지
	if (SelectionType == ESelectInfo::Direct) return;

	UpdateDisplaySettings();
}

void UPTWOptionsWidget::OnQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;

	// "Custom" 텍스트가 들어온 경우 로직을 수행하지 않고 리턴
	if (SelectedItem == NSLOCTEXT("Options", "Custom", "Custom").ToString())
	{
		return;
	}

	// 사용자가 전체 품질 콤보박스를 직접 건드린 경우
	if (Combo_Quality && Combo_Quality->GetSelectedOption() == SelectedItem)
	{
		int32 NewIndex = Combo_Quality->GetSelectedIndex();
		if (NewIndex != -1)
		{
			if (Combo_ViewDistance) Combo_ViewDistance->SetSelectedIndex(NewIndex);
			if (Combo_Shadow)       Combo_Shadow->SetSelectedIndex(NewIndex);
			if (Combo_Texture)      Combo_Texture->SetSelectedIndex(NewIndex);
			if (Combo_Effects)      Combo_Effects->SetSelectedIndex(NewIndex);
			if (Combo_PostProcess)  Combo_PostProcess->SetSelectedIndex(NewIndex);
			if (Combo_Foliage)      Combo_Foliage->SetSelectedIndex(NewIndex);
			if (Combo_Shading)      Combo_Shading->SetSelectedIndex(NewIndex);
			if (Combo_AntiAliasing) Combo_AntiAliasing->SetSelectedIndex(NewIndex);
		}
	}

	// 만약 세부 항목 콤보박스를 건드렸다면 바로 엔진에 적용
	UpdateScalabilitySettings();
}

void UPTWOptionsWidget::OnVSyncChanged(bool bChecked)
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	Settings->SetVSyncEnabled(bChecked);
	bIsDirty = true;
}

void UPTWOptionsWidget::OnMasterVolumeChanged(float Value)
{
	if (ET_MasterVolume) ET_MasterVolume->SetText(FormatFloatToText(Value));

	UpdateAudioSettings();
}

void UPTWOptionsWidget::OnMasterVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 숫자 검증 및 동기화 (0.0 ~ 1.0)
	if (ValidateAndApplyTextEntry(Text, Slider_MasterVolume, ET_MasterVolume, 0.0f, 1.0f))
	{
		// 유효한 입력이었을 때만 엔진 설정 업데이트
		UpdateAudioSettings();
	}
}

void UPTWOptionsWidget::OnMouseSensitivityChanged(float Value)
{
	if (ET_MouseSensitivity)
	{
		ET_MouseSensitivity->SetText(FormatFloatToText(Value));
	}

	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		PC->ApplyMouseSensitivity(Value);
	}
}

void UPTWOptionsWidget::OnSensitivityCaptureEnd()
{
	UpdateInputSettings();
}

void UPTWOptionsWidget::OnMouseSensitivityTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 숫자 검증 및 동기화 (0.0 ~ 5.0 범위)
	if (ValidateAndApplyTextEntry(Text, Slider_MouseSensitivity, ET_MouseSensitivity, 0.0f, 5.0f))
	{
		// 유효한 입력이었을 때만 엔진 설정 업데이트
		UpdateInputSettings();
	}
}

void UPTWOptionsWidget::OnClickedSave()
{
	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	UE_LOG(LogTemp, Error, TEXT("OptionWidget : Save1."));

	if (!Settings) return;

	UE_LOG(LogTemp, Error, TEXT("OptionWidget : Save2."));

	Settings->ApplySettings(false);
	Settings->SaveSettings();

	CacheInitialSettings();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->PopWidget();
		}
	}
}

void UPTWOptionsWidget::OnClickedCancel()
{
	if (bIsDirty)
	{
		RestoreInitialSettings();
	}

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->PopWidget();
		}
	}
}

void UPTWOptionsWidget::OnClickedGraphics()
{
	if (CategorySwitcher)
		CategorySwitcher->SetActiveWidgetIndex(0);
}

void UPTWOptionsWidget::OnClickedSound()
{
	if (CategorySwitcher)
		CategorySwitcher->SetActiveWidgetIndex(1);
}

void UPTWOptionsWidget::OnClickedGame()
{
	if (CategorySwitcher)
		CategorySwitcher->SetActiveWidgetIndex(2);
}
