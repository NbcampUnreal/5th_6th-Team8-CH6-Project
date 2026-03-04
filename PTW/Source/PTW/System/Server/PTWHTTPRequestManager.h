// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "UObject/Object.h"
#include "PTWHTTPRequestManager.generated.h"

class UPTWAPIData;

UCLASS(Blueprintable, BlueprintType)
class PTW_API UPTWHTTPRequestManager : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> APIData;
	
	void RequestListFleets();
	
	void ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
