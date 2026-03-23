// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "System/Session/PTWSessionConfig.h"
#include "PTWServerBrowser.generated.h"

class UBorder;
class UButton;
class UVerticalBox;
class UEditableText;
class UCheckBox;
class UPTWServerListRow;

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
	// 서버 탐색결과를 저장하는 리스트 위젯
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> ServerListVerticalBox;
	
#pragma region Server Menu
	// 서버 Context Menu를 표시하는 버튼
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ServerMenuButton;
	
	// 서버 Menu Context 보더
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> ServerMenuBorder;
	
	// 서버의 이름을 설정
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableText> ServerNameEditableText;
	
	// 서버에 최대 참여가능한 플레이어 수를 설정
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableText> ServerMaxPlayerEditableText;
	
	// AWS GameLift로 원격 Dedicated Server를 활성할지 여부
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> DedicatedCheckBox;
	
	// 서버를 짧은 라운드로 설정
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ShortRoundButton;
	
	// 서버를 긴 라운드로 설정
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LongRoundButton;
	
	// 종합 설정들을 기반으로 서버를 생성하는 버튼
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CreateServerButton;
#pragma endregion
	
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
	
private:
	EPTWRoundLimit RoundLimit;
	
public:
	UPROPERTY(BlueprintAssignable, Category= "Events")
	FOnServerBackAction OnServerBackAction;
	
};
