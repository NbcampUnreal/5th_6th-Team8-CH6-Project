// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWAPIData.h"

FString UPTWAPIData::GEtAPIEndPoint(const FGameplayTag& APIEndPoint)
{
	const FString ResourceName = Resources.FindChecked(APIEndPoint);
	
	return InvokeURL + "/" + Stage + "/" + ResourceName;
}
