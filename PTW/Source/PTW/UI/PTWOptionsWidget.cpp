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
	if (!Combo_Quality) return;

	Combo_Quality->ClearOptions();

	// 엔진의 Scalability Level (0~3)에 대응하는 문자열 추가
	Combo_Quality->AddOption(NSLOCTEXT("Options", "Quality_Low", "Low").ToString());

	Combo_Quality->AddOption(NSLOCTEXT("Options", "Quality_Medium", "Medium").ToString());

	Combo_Quality->AddOption(NSLOCTEXT("Options", "Quality_High", "High").ToString());

	Combo_Quality->AddOption(NSLOCTEXT("Options", "Quality_VeryHigh", "VeryHigh").ToString());
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

	if (Combo_Quality)
	{
		// 엔진에서 현재 레벨(0~3)을 가져옴
		int32 CurrentLevel = Settings->GetOverallScalabilityLevel();

		// 만약 엔진 설정이 4(Cinematic)라면 우리가 쓸 최대치인 3으로 고정
		int32 SafeLevel = FMath::Clamp(CurrentLevel, 0, 3);

		// 인덱스를 세팅하면 자동으로 "매우 높음" 등이 선택됨
		Combo_Quality->SetSelectedIndex(SafeLevel);
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
	InitialSnapshot.MasterVolume = Settings->MasterVolume;
	InitialSnapshot.MouseSensitivity = Settings->MouseSensitivity;

	bIsDirty = false;
}

void UPTWOptionsWidget::ApplyCurrentUIToEngine()
{
	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	// 창모드
	if (CheckBox_Windowed)
	{
		Settings->SetFullscreenMode(
			CheckBox_Windowed->IsChecked()
			? EWindowMode::Windowed
			: EWindowMode::Fullscreen);
	}

	// 해상도
	if (Combo_Resolution)
	{
		FString ResString = Combo_Resolution->GetSelectedOption();
		FString Left, Right;

		if (ResString.Split(TEXT(" x "), &Left, &Right))
		{
			int32 X = FCString::Atoi(*Left);
			int32 Y = FCString::Atoi(*Right);
			if (X > 0 && Y > 0)
			{
				Settings->SetScreenResolution(FIntPoint(X, Y));
			}
		}
	}

	// 품질
	if (Combo_Quality)
	{
		// "낮음"(0), "중간"(1), "높음"(2), "매우 높음"(3) 순서대로 인덱스를 가져옴
		int32 SelectedLevel = Combo_Quality->GetSelectedIndex();

		if (SelectedLevel != -1) // 선택된게 있다면
		{
			Settings->SetOverallScalabilityLevel(SelectedLevel);
		}
	}

	Settings->ApplySettings(false);

	// 오디오
	if (Slider_MasterVolume && MasterSoundMix && MasterSoundClass)
	{
		float NewVolume = Slider_MasterVolume->GetValue();
		Settings->MasterVolume = NewVolume;

		// Sound Mix 내의 특정 Sound Class 볼륨을 실시간으로 변경
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			MasterSoundMix,
			MasterSoundClass,
			NewVolume, // 볼륨 (0.0 ~ 1.0)
			1.0f,      // Pitch
			0.0f,      // Fade In 타임 (즉시 반영)
			true       // 기존 설정을 무시하고 이 값으로 강제 적용
		);

		// 변경된 믹서를 엔진에 적용
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);
	}

	// 감도
	if (Slider_MouseSensitivity)
	{
		float NewSensitivity = Slider_MouseSensitivity->GetValue();
		Settings->MouseSensitivity = NewSensitivity;

		if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
		{
			PC->ApplyMouseSensitivity(NewSensitivity);
		}
	}

	bIsDirty = true;
}

void UPTWOptionsWidget::RestoreInitialSettings()
{
	UPTWGameUserSettings* Settings =
		Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());

	if (!Settings) return;

	Settings->SetScreenResolution(InitialSnapshot.Resolution);
	Settings->SetFullscreenMode(InitialSnapshot.WindowMode);
	Settings->SetOverallScalabilityLevel(InitialSnapshot.ScalabilityLevel);

	Settings->MasterVolume = InitialSnapshot.MasterVolume;
	Settings->MouseSensitivity = InitialSnapshot.MouseSensitivity;

	Settings->ApplySettings(false);

	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		PC->ApplyMouseSensitivity(InitialSnapshot.MouseSensitivity);
	}

	bIsDirty = false;
}

void UPTWOptionsWidget::BindEvents()
{
	if (CheckBox_Windowed)
	{
		CheckBox_Windowed->OnCheckStateChanged.AddDynamic(
			this, &UPTWOptionsWidget::OnCheckWindowedChanged);
	}

	if (Combo_Resolution)
	{
		Combo_Resolution->OnSelectionChanged.AddDynamic(
			this, &UPTWOptionsWidget::OnResolutionChanged);
	}

	if (Combo_Quality)
	{
		Combo_Quality->OnSelectionChanged.AddDynamic(
			this, &UPTWOptionsWidget::OnQualityChanged);
	}

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->OnValueChanged.AddDynamic(
			this, &UPTWOptionsWidget::OnMasterVolumeChanged);
	}
	if (ET_MasterVolume)
	{
		ET_MasterVolume->OnTextCommitted.AddDynamic(this, &UPTWOptionsWidget::OnMasterVolumeTextCommitted);
	}

	if (Slider_MouseSensitivity)
	{
		// 드래그 중에는 값만 업데이트 (선택 사항)
		Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &UPTWOptionsWidget::OnMouseSensitivityChanged);

		// 마우스를 뗄 때 엔진에 최종 적용 (더 안정적임)
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

FText UPTWOptionsWidget::FormatFloatToText(float Value) const
{
	FNumberFormattingOptions NumberOptions = FNumberFormattingOptions::DefaultNoGrouping();
	NumberOptions.SetMaximumFractionalDigits(1);
	NumberOptions.SetMinimumFractionalDigits(1);

	return FText::AsNumber(Value, &NumberOptions);
}

void UPTWOptionsWidget::OnCheckWindowedChanged(bool bChecked)
{
	ApplyCurrentUIToEngine();
}

void UPTWOptionsWidget::OnResolutionChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	// 초기화 과정에서 호출되는 것 방지
	if (SelectionType == ESelectInfo::Direct)
		return;

	ApplyCurrentUIToEngine();
}

void UPTWOptionsWidget::OnQualityChanged(
	FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct)
		return;

	ApplyCurrentUIToEngine();
}

void UPTWOptionsWidget::OnMasterVolumeChanged(float Value)
{
	if (ET_MasterVolume)
	{
		ET_MasterVolume->SetText(FormatFloatToText(Value));
	}
	ApplyCurrentUIToEngine();
}

void UPTWOptionsWidget::OnMasterVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 숫자로 변환 가능한지 확인
	float NewValue = FCString::Atof(*Text.ToString());

	// 슬라이더 범위 내로 고정 (예: 0.0 ~ 1.0)
	NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->SetValue(NewValue);
	}

	// 텍스트 박스도 정돈된 포맷으로 다시 설정
	ET_MasterVolume->SetText(FormatFloatToText(NewValue));

	ApplyCurrentUIToEngine();
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
	ApplyCurrentUIToEngine();
}

void UPTWOptionsWidget::OnMouseSensitivityTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	float NewValue = FCString::Atof(*Text.ToString());

	// 감도 범위 (예: 0.0 ~ 2.0 사이로 설정 시)
	NewValue = FMath::Clamp(NewValue, 0.0f, 5.0f);

	if (Slider_MouseSensitivity)
	{
		Slider_MouseSensitivity->SetValue(NewValue);
	}

	ET_MouseSensitivity->SetText(FormatFloatToText(NewValue));

	ApplyCurrentUIToEngine();
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
