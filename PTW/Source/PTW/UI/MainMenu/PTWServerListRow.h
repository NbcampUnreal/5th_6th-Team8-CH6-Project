// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "Blueprint/UserWidget.h"
#include "PTWServerListRow.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class PTW_API UPTWServerListRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(const FOnlineSessionSearchResultBP& SearchResult);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnClickedJoinButton();
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ServerID;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ServerName;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> JoinButton;
	
	FOnlineSessionSearchResult SessionData;
};
