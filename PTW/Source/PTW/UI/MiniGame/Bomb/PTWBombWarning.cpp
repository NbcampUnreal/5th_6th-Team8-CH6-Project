// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/Bomb/PTWBombWarning.h"
#include "Components/Image.h"
#include "PTW/MiniGame/Item/BombItem/PTWBombActor.h"
#include "GAS/PTWBombAttributeSet.h"
#include "AbilitySystemComponent.h"

void UPTWBombWarning::SetTargetBomb(APTWBombActor* InBomb)
{
	// 기존 바인딩이 있다면 제거
	if (TargetBomb && TimeChangedHandle.IsValid())
	{
		if (UAbilitySystemComponent* OldASC = TargetBomb->GetAbilitySystemComponent())
		{
			OldASC->GetGameplayAttributeValueChangeDelegate(UPTWBombAttributeSet::GetRemainingTimeAttribute()).Remove(TimeChangedHandle);
		}
	}

	TargetBomb = InBomb;
	if (!TargetBomb) return;

	// 새로운 폭탄의 ASC에서 RemainingTime 변경 이벤트 바인딩
	if (UAbilitySystemComponent* ASC = TargetBomb->GetAbilitySystemComponent())
	{
		TimeChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UPTWBombAttributeSet::GetRemainingTimeAttribute())
			.AddUObject(this, &UPTWBombWarning::OnBombTimeChanged);

		// 초기값으로 한 번 설정
		bool bFound = false;
		float CurrentTime = ASC->GetGameplayAttributeValue(UPTWBombAttributeSet::GetRemainingTimeAttribute(), bFound);
		if (bFound)
		{
			FOnAttributeChangeData InitialData;
			InitialData.NewValue = CurrentTime;
			OnBombTimeChanged(InitialData);
		}
	}
}

void UPTWBombWarning::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WarningImage) return;

	// Brush에 설정된 Material 가져오기
	UObject* Resource = WarningImage->Brush.GetResourceObject();
	UMaterialInterface* BaseMat = Cast<UMaterialInterface>(Resource);

	if (BaseMat)
	{
		// Dynamic Material 생성
		WarningDynamicMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		WarningImage->SetBrushFromMaterial(WarningDynamicMat);
	}


}

void UPTWBombWarning::NativeDestruct()
{
	// 위젯이 파괴될 때 델리게이트 해제 (메모리 누수 및 크래시 방지)
	if (TargetBomb && TimeChangedHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = TargetBomb->GetAbilitySystemComponent())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UPTWBombAttributeSet::GetRemainingTimeAttribute()).Remove(TimeChangedHandle);
		}
	}

	Super::NativeDestruct();
}

void UPTWBombWarning::OnBombTimeChanged(const FOnAttributeChangeData& Data)
{
	float RemainingTime = Data.NewValue;

	// 점멸 속도 계산 (이벤트가 발생할 때만 1번 계산)
	// 시간이 줄어들수록 속도가 빨라지게 매핑
	float SpeedMultiplier = FMath::Clamp(10.0f / (RemainingTime + 1.0f), 1.0f, 10.0f);
	float CurrentSpeed = BaseBlinkSpeed * SpeedMultiplier;

	// 머터리얼의 Scalar Parameter 업데이트
	if (WarningDynamicMat)
	{
		WarningDynamicMat->SetScalarParameterValue(FName("BlinkSpeed"), CurrentSpeed);
	}

	//// 색상 변경 (임계치 이하일 때)
	//if (WarningImage)
	//{
	//	FLinearColor TargetColor = (RemainingTime < 3.0f) ? FLinearColor::Red : FLinearColor::White;
	//	WarningImage->SetColorAndOpacity(TargetColor);
	//}
}
