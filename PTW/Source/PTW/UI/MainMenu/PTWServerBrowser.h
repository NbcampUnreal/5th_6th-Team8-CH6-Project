// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWServerBrowser.generated.h"

class UBorder;
class UButton;
class UVerticalBox;
class UEditableText;
class UPTWServerListRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnServerBackAction);

enum class EPTWRoundLimit : uint8;

UCLASS()
class PTW_API UPTWServerBrowser : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category= "Events")
	FOnServerBackAction OnServerBackAction;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnClickedBackButton();			// 메인메뉴로 이동
	
	UFUNCTION()
	void OnClickedServerMenuButton();	// 로비 생성 전, 생성 메뉴 토글
	
	UFUNCTION()
	void OnClickedCreateServerButton();	// 로비 생성
	
	UFUNCTION()
	void OnClickedFindServerButton();	// 로비 찾기
	
	UFUNCTION()
	void OnClickedShortRoundButton();
	
	UFUNCTION()
	void OnClickedLongRoundButton();
	
	UFUNCTION()
	void OnFindSessionsComplete(const TArray<FOnlineSessionSearchResultBP>& SearchResults);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> ServerListVerticalBox;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> ServerMenuBorder;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ServerMenuButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableText> ServerNameEditableText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableText> ServerMaxPlayerEditableText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ShortRoundButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LongRoundButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CreateServerButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> FindServerButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BackButton;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UPTWServerListRow> ServerListRowClass;
	
private:
	EPTWRoundLimit RoundLimit;
};
