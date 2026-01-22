// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "Blueprint/UserWidget.h"
#include "PTWLobbyListRow.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class PTW_API UPTWLobbyListRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(const FBlueprintSessionResult& SessionResult);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnClickedJoinButton();
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> LobbyID;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> LobbyName;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> JoinButton;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	FBlueprintSessionResult SessionData;
};
