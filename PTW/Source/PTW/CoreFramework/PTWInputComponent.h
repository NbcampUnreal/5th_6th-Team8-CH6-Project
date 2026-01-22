// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "PTWInputConfig.h"
#include "PTWInputComponent.generated.h"


UCLASS()
class PTW_API UPTWInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UPTWInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);
};

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UPTWInputComponent::BindAbilityActions(const UPTWInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FPTWInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag).GetHandle());
			}
			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Canceled, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}

		}
	}
}
