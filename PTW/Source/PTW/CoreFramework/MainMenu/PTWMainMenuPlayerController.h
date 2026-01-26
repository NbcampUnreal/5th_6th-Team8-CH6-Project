// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PTWMainMenuPlayerController.generated.h"

class UPTWMainMenu;
class UPTWLobbyBrowser;
/**
 * 
 */


UCLASS()
class PTW_API APTWMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APTWMainMenuPlayerController();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPTWMainMenu> MainMenuClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UPTWMainMenu> MainMenuInstance;
};
