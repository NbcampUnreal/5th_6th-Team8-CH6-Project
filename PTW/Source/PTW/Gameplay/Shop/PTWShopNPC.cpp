// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Shop/PTWShopNPC.h"
#include "PTWDisplayItem.h"
#include "Net/UnrealNetwork.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Kismet/GameplayStatics.h"

APTWShopNPC::APTWShopNPC()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NPCMesh"));
	NPCMesh->SetupAttachment(RootComponent);

	StandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StandMesh"));
	StandMesh->SetupAttachment(RootComponent);
	StandMesh->SetCollisionProfileName(TEXT("BlockAll"));

	DecoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DecoMesh"));
	DecoMesh->SetupAttachment(RootComponent);
	DecoMesh->SetCollisionProfileName(TEXT("NoCollision"));
	bReplicates = true;
	bIsLocallyOpen = true;
}

void APTWShopNPC::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() != NM_DedicatedServer)
	{
		CheckShopAvailability();
	}
}

void APTWShopNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWShopNPC, ShopCategory);
	DOREPLIFETIME(APTWShopNPC, DisplayItems);
}

void APTWShopNPC::InitializeShop(EShopCategory InCategory, const TArray<FName>& InItemIDs, const TArray<FTransform>& DisplayLocs)
{
	if (!HasAuthority()) return;

	ShopCategory = InCategory;

	int32 Count = FMath::Min(InItemIDs.Num(), DisplayLocs.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FTransform SpawnTM = DisplayLocs[i] * GetActorTransform();

		FActorSpawnParameters Params;
		Params.Owner = this;

		APTWDisplayItem* Item = GetWorld()->SpawnActor<APTWDisplayItem>(DisplayItemClass, SpawnTM, Params);
		if (Item)
		{
			Item->InitDisplay(InItemIDs[i]);
			Item->SetParentShop(this);
			DisplayItems.Add(Item);
		}
	}
}


void APTWShopNPC::CheckShopAvailability()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APTWPlayerState* PS = PC->GetPlayerState<APTWPlayerState>();
	if (!PS) return;

	//TODO : 로컬 클라이언트에서 닫은 상점인지 체크하기
}

void APTWShopNPC::CloseShop()
{
	if (HasAuthority())
	{
		if (NPCMesh) NPCMesh->SetVisibility(false);
		if (StandMesh) StandMesh->SetVisibility(false);
		if (DecoMesh) DecoMesh->SetVisibility(false);

		for (APTWDisplayItem* Item : DisplayItems)
		{
			if (Item)
			{
				if (UStaticMeshComponent* Mesh = Item->FindComponentByClass<UStaticMeshComponent>())
				{
					Mesh->SetVisibility(false);
				}
			}
		}
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		for (APTWDisplayItem* Item : DisplayItems)
		{
			if (Item)
			{
				Item->SetActorHiddenInGame(true);
				Item->SetActorEnableCollision(false);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Shop Closed Visuals Executed (Mode: %s)."),
		HasAuthority() ? TEXT("Host/Server") : TEXT("Client"));
}
