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
class UPTWHTTPRequestManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnServerBackAction);

enum class EPTWRoundLimit : uint8;

UCLASS()
class PTW_API UPTWServerBrowser : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnClickedBackButton();
	
	UFUNCTION()
	void OnClickedServerMenuButton();
	
	UFUNCTION()
	void OnClickedCreateServerButton();
	
	UFUNCTION()
	void OnClickedFindServerButton();
	
	UFUNCTION()
	void OnClickedQuickMatchButton();
	
	UFUNCTION()
	void OnClickedShortRoundButton();
	
	UFUNCTION()
	void OnClickedLongRoundButton();
	
	UFUNCTION()
	void OnFindSessionsComplete(const TArray<FOnlineSessionSearchResultBP>& SearchResults);
	
	UFUNCTION()
	void OnClickedTestButton();
	
	UFUNCTION()
	void DevJoinAction();
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
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> QuickMatchButton;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UPTWServerListRow> ServerListRowClass;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> DevJoinButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> TestButton;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPTWHTTPRequestManager> HTTPRequestManagerClass;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWHTTPRequestManager> HTTPRequestManager;
	
private:
	EPTWRoundLimit RoundLimit;
	
public:
	UPROPERTY(BlueprintAssignable, Category= "Events")
	FOnServerBackAction OnServerBackAction;
	
};
