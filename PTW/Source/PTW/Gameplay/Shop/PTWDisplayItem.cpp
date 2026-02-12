// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Shop/PTWDisplayItem.h"
#include "Components/WidgetComponent.h"
#include "System/Shop/PTWShopSubsystem.h"
#include "CoreFramework/PTWPlayerState.h" 
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/Shop/PTWItemInfoWidget.h"

APTWDisplayItem::APTWDisplayItem()
{
	PrimaryActorTick.bCanEverTick = false;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	bReplicates = true;

	InfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InfoWidget"));
	InfoWidget->SetupAttachment(RootComponent);
	InfoWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InfoWidget->SetCollisionProfileName(TEXT("NoCollision"));

	InfoWidget->SetupAttachment(RootComponent);
	InfoWidget->SetDrawAtDesiredSize(true);
	InfoWidget->SetVisibility(false);
}

void APTWDisplayItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWDisplayItem, ItemID);
}

void APTWDisplayItem::InitDisplay(FName NewItemID)
{
	ItemID = NewItemID;

	UpdateItemVisuals();
}


void APTWDisplayItem::TryPurchase(APlayerController* Player)
{
	if (!Player || !ParentShop) return;

	int32 FinalPrice = 0;
	if (UPTWShopSubsystem* Sys = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		FinalPrice = Sys->GetItemPrice(ItemID);
	}
	else
	{
		return;
	}
	if (APTWPlayerState* PS = Player->GetPlayerState<APTWPlayerState>())
	{
		PS->ServerRequestPurchase(ParentShop, ItemID, FinalPrice);
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
	FString ActionStr = FString::Printf(TEXT("F를 눌러 구매하기 (%d G)"), CachedPrice);
	return FText::FromString(ActionStr);
}

bool APTWDisplayItem::IsInteractable_Implementation()
{
	if (!GetRootComponent()) return false;

	if (!GetRootComponent()->IsVisible())
	{
		return false;
	}

	return true;
}

void APTWDisplayItem::OnRep_ItemID()
{
	UpdateItemVisuals();
}

void APTWDisplayItem::UpdateItemVisuals()
{
	if (UPTWShopSubsystem* Sys = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		CachedPrice = Sys->GetItemPrice(ItemID);
		const FShopItemRow* Data = Sys->GetShopItemData(ItemID);

		if (Data)
		{
			if (!Data->DisplayMesh.IsNull())
				ItemMesh->SetStaticMesh(Data->DisplayMesh.LoadSynchronous());

			// TODO : 위젯 정보 갱신
			if (InfoWidget && ItemInfoWidgetClass)
			{
				InfoWidget->SetWidgetClass(ItemInfoWidgetClass);

				if (UPTWItemInfoWidget* UI = Cast<UPTWItemInfoWidget>(InfoWidget->GetUserWidgetObject()))
				{
					UI->SetItemID(ItemID);
				}
			}
		}
	}
}

void APTWDisplayItem::BeginPlay()
{
	Super::BeginPlay();

	if (!ItemID.IsNone())
	{
		UpdateItemVisuals();
	}
}
