// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWLobbyBrowser.generated.h"

class UBorder;
class UButton;
class UVerticalBox;
class UEditableText;
class UPTWLobbyListRow;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyBackAction);

UCLASS()
class PTW_API UPTWLobbyBrowser : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category= "Events")
	FOnLobbyBackAction OnLobbyBackAction;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnClickedBackButton();			// 메인메뉴로 이동
	
	UFUNCTION()
	void OnClickedLobbyMenuButton();	// 로비 생성 전, 생성 메뉴 토글
	
	UFUNCTION()
	void OnClickedCreateLobbyButton();	// 로비 생성
	
	UFUNCTION()
	void OnClickedFindLobbyButton();	// 로비 찾기
	
	UFUNCTION()
	void OnFindLobbiesComplete(const TArray<FBlueprintSessionResult>& SessionResults);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> LobbyListVerticalBox;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> LobbyMenuBorder;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LobbyMenuButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableText> LobbyNameEditableText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableText> LobbyMaxPlayerEditableText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CreateLobbyButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> FindLobbyButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BackButton;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UPTWLobbyListRow> LobbyListRowClass;
};
