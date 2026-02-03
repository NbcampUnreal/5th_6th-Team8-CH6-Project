// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Shop/PTWShopSubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Gameplay/Shop/PTWShopNPC.h"
#include "Gameplay/Shop/PTWShopSpot.h"
#include "Kismet/KismetMathLibrary.h"

void UPTWShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UDataTable* LoadedTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_ShopItems"));

	if (LoadedTable)
	{
		MasterShopTable = LoadedTable;
	}

	if (MasterShopTable)
	{
		CachedShopItems.Empty();
		static const FString ContextString(TEXT("ShopInit"));
		for (const FName& Name : MasterShopTable->GetRowNames())
		{
			if (FShopItemRow* Row = MasterShopTable->FindRow<FShopItemRow>(Name, ContextString))
				CachedShopItems.Add(Name, *Row);
		}
	}
}

void UPTWShopSubsystem::RegisterShopSpot(APTWShopSpot* Spot)
{
	if (Spot && !ShopSpots.Contains(Spot)) ShopSpots.Add(Spot);
}

void UPTWShopSubsystem::UnregisterShopSpot(APTWShopSpot* Spot)
{
	ShopSpots.Remove(Spot);
}

void UPTWShopSubsystem::InitializeShopsForRound(FGameplayTag NextMinigameTag, FGameplayTag RoundEventTag)
{
	if (ShopSpots.Num() == 0 || !ShopNPCClass) return;

	CurrentRoundEventTag = RoundEventTag;

	for (const auto& NPC : ActiveNPCs)
	{
		if (IsValid(NPC))
		{
			NPC->Destroy();
		}
	}
	ActiveNPCs.Empty();

	TArray<EShopCategory> SelectedCategories = SelectShopCategories();

	TArray<APTWShopSpot*> ShuffledSpots = ShopSpots;
	int32 LastIndex = ShuffledSpots.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		ShuffledSpots.Swap(i, Index);
	}

	int32 SpawnCount = FMath::Min(SelectedCategories.Num(), ShuffledSpots.Num());

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		APTWShopSpot* Spot = ShuffledSpots[i];
		EShopCategory Category = SelectedCategories[i];

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APTWShopNPC* NewNPC = GetWorld()->SpawnActor<APTWShopNPC>(ShopNPCClass, Spot->GetActorTransform(), Params);

		if (NewNPC)
		{
			TArray<FName> ShopItems = SelectItemsForShop(Category, NextMinigameTag);

			NewNPC->InitializeShop(Category, ShopItems, Spot->GetItemSpawnTransforms());

			ActiveNPCs.Add(NewNPC);
		}
	}
}

int32 UPTWShopSubsystem::GetItemPrice(FName ItemID) const
{
	if (const FShopItemRow* ItemData = CachedShopItems.Find(ItemID))
	{
		float FinalPrice = (float)ItemData->BasePrice * GetPriceMultiplier();

		return FMath::Max(1, FMath::RoundToInt(FinalPrice));
	}
	return 999999; // 에러코드
}

float UPTWShopSubsystem::GetPriceMultiplier() const
{
	//TODO : 현재 이벤트 태그에 따른 가격 배율 계산 (0.5 ~ 2.0)
	
	return 1.0f;
}

TArray<EShopCategory> UPTWShopSubsystem::SelectShopCategories()
{
	TArray<EShopCategory> Result;
	TMap<EShopCategory, int32> CountMap;

	TArray<EShopCategory> AllTypes = {
		EShopCategory::Attack, EShopCategory::Defense, EShopCategory::Utility,
		EShopCategory::Chaos, EShopCategory::Lobby
	};

	for (EShopCategory Type : AllTypes)
	{
		Result.Add(Type);
		CountMap.Add(Type, 1);
	}

	while (Result.Num() < 7)
	{
		int32 RandIdx = FMath::RandRange(0, AllTypes.Num() - 1);
		EShopCategory Target = AllTypes[RandIdx];

		if (CountMap[Target] < 3)
		{
			Result.Add(Target);
			CountMap[Target]++;
		}
	}

	return Result;
}

TArray<FName> UPTWShopSubsystem::SelectItemsForShop(EShopCategory Category, FGameplayTag BanTag)
{
	TArray<FName> Candidates;

	for (const auto& Elem : CachedShopItems)
	{
		const FShopItemRow& Row = Elem.Value;

		if (Row.Category == Category)
		{
			if (!BanTag.IsValid() || !Row.BannedGameModes.HasTag(BanTag))
			{
				Candidates.Add(Elem.Key);
			}
		}
	}

	TArray<FName> SelectedItems;
	int32 RequiredCount = 3;

	while (SelectedItems.Num() < RequiredCount && Candidates.Num() > 0)
	{
		int32 Idx = FMath::RandRange(0, Candidates.Num() - 1);
		SelectedItems.Add(Candidates[Idx]);
		Candidates.RemoveAt(Idx);
	}

	return SelectedItems;
}

const FShopItemRow* UPTWShopSubsystem::GetShopItemData(FName ItemID) const
{
	return CachedShopItems.Find(ItemID);
}
