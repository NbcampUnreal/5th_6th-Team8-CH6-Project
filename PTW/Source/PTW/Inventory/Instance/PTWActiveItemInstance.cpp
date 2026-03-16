// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWActiveItemInstance.h"

#include "Net/UnrealNetwork.h"

void UPTWActiveItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentCount);
}

bool UPTWActiveItemInstance::UsingActiveItem()
{
	if (CurrentCount < 0) return false;

	if (CurrentCount == 0)
	{
		CurrentCount = -1; 
		OnCurrentCountChanged.Broadcast(CurrentCount);
		OnItemDepleted.Broadcast();
		return false; 
	}

	CurrentCount--;
	
	// 서버에서도 즉시 UI 갱신을 위해 호출 (서버가 로컬 플레이어인 경우 대비)
	OnCurrentCountChanged.Broadcast(CurrentCount);

	return true;
}

void UPTWActiveItemInstance::OnRep_CurrentCount()
{
	OnCurrentCountChanged.Broadcast(CurrentCount);
}
