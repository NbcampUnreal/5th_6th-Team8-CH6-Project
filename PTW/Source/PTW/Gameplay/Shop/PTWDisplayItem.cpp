// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Shop/PTWDisplayItem.h"
#include "Components/WidgetComponent.h"
#include "System/Shop/PTWShopSubsystem.h"
#include "CoreFramework/PTWPlayerState.h" 
#include "Kismet/GameplayStatics.h"

APTWDisplayItem::APTWDisplayItem()
{
	PrimaryActorTick.bCanEverTick = false;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	InfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InfoWidget"));
	InfoWidget->SetupAttachment(RootComponent);
	InfoWidget->SetWidgetSpace(EWidgetSpace::World);
	InfoWidget->SetCollisionProfileName(TEXT("NoCollision"));
}

void APTWDisplayItem::InitDisplay(FName NewItemID)
{
	ItemID = NewItemID;
	if (UPTWShopSubsystem* Sys = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		CachedPrice = Sys->GetItemPrice(ItemID);
		
		if (const FShopItemRow* Data = Sys->GetShopItemData(ItemID))
		{
			if (!Data->DisplayMesh.IsNull())
				ItemMesh->SetStaticMesh(Data->DisplayMesh.LoadSynchronous());

			// TODO : 3D Widget 업데이트 호출
		}
	}
}

void APTWDisplayItem::TryPurchase(APlayerController* Player)
{
	if (!Player || !ParentShop) return;

	if (APTWPlayerState* PS = Player->GetPlayerState<APTWPlayerState>())
	{

	}
}

void APTWDisplayItem::OnInteract_Implementation(APawn* InstigatorPawn)
{
	if (!InstigatorPawn) return;

	if (APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController()))
	{
		TryPurchase(PC);
	}
}

FText APTWDisplayItem::GetInteractionKeyword_Implementation()
{
	FString ActionStr = FString::Printf(TEXT("구매하기 (%d G)"), CachedPrice);
	return FText::FromString(ActionStr);
}

bool APTWDisplayItem::IsInteractable_Implementation()
{
	return !ItemID.IsNone() && ParentShop != nullptr;
}

