// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerState.h"

#include "GAS/PTWWeaponAttributeSet.h"
#include "PTW/GAS/PTWAbilitySystemComponent.h"
#include "PTW/GAS/PTWAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayAbilitySpec.h"
#include "Gameplay/Shop/PTWShopNPC.h"
#include "GAS/PTWDeliveryAttributeSet.h"
#include "CoreFramework/Game/Gamestate/PTWGamestate.h"
#include "CoreFramework/PTWPlayerController.h"
#include "Game/GameMode/PTWGameMode.h"
#include "Game/GameMode/PTWLobbyGameMode.h"
#include "Inventory/PTWInventoryComponent.h"

APTWPlayerState::APTWPlayerState()
{
	NetUpdateFrequency = 100.0f;
	NetPriority = 5.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UPTWAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UPTWAttributeSet>(TEXT("AttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UPTWWeaponAttributeSet>(TEXT("WeaponAttributeSet"));
	
	InventoryComponent = CreateDefaultSubobject<UPTWInventoryComponent>(TEXT("Inventory"));
	
	CurrentPlayerData.PlayerName = "";
	CurrentPlayerData.TotalWinPoints = 0;
	CurrentPlayerData.Gold = 0.0f;
}

void APTWPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWPlayerState, CurrentPlayerData);
	DOREPLIFETIME(APTWPlayerState, PlayerRoundData);
	DOREPLIFETIME(APTWPlayerState, LobbyItemData);
	DOREPLIFETIME(APTWPlayerState, MiniGameComponent);
}

UAbilitySystemComponent* APTWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APTWPlayerState::SetPlayerData(const FPTWPlayerData& NewData)
{
	if (HasAuthority())
	{
		CurrentPlayerData = NewData;
		OnPlayerDataUpdated.Broadcast(CurrentPlayerData);
	}
}

void APTWPlayerState::SetPlayerRoundData(const FPTWPlayerRoundData& NewData)
{
	if (HasAuthority())
	{
		PlayerRoundData = NewData;
	}
}

FPTWPlayerData APTWPlayerState::GetPlayerData() const
{
	return CurrentPlayerData;
}

FPTWPlayerRoundData APTWPlayerState::GetPlayerRoundData() const
{
	return PlayerRoundData;
}

void APTWPlayerState::SetLobbyItemData(const FPTWLobbyItemData& NewData)
{
	if (HasAuthority())
	{
		LobbyItemData = NewData;
	}
}

void APTWPlayerState::SetMiniGameComponent(UActorComponent* NewMiniGameComponent)
{
	if (HasAuthority())
	{
		MiniGameComponent = NewMiniGameComponent;
	}
}

void APTWPlayerState::ServerVotePredictedPlayer_Implementation(FUniqueNetIdRepl PredictedPlayer)
{
	LobbyItemData.PredictedData.PredictedPlayer = PredictedPlayer.ToString();
}

void APTWPlayerState::InjectAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass || !HasAuthority()) return;

	if (!AdditionalAbilities.Contains(AbilityClass))
	{
		AdditionalAbilities.Add(AbilityClass);
	}
	if (AbilitySystemComponent)
	{
		if (!AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
		{
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void APTWPlayerState::InjectEffect(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!EffectClass || !HasAuthority()) return;

	if (!AdditionalEffects.Contains(EffectClass))
	{
		AdditionalEffects.Add(EffectClass);
	}

	if (AbilitySystemComponent)
	{
		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();

		if (EffectCDO->DurationPolicy == EGameplayEffectDurationType::Infinite)
		{

		}

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void APTWPlayerState::ApplyAdditionalAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AdditionalAbilities)
	{
		if (AbilityClass)
		{
			if (!AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
				AbilitySystemComponent->GiveAbility(Spec);
			}
		}
	}
}

void APTWPlayerState::ApplyAdditionalEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	for (const TSubclassOf<UGameplayEffect>& EffectClass : AdditionalEffects)
	{
		if (!EffectClass) continue;

		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();

		 if (EffectCDO->DurationPolicy == EGameplayEffectDurationType::Infinite)
		 {
		      if (AbilitySystemComponent->HasAnyMatchingGameplayTags(EffectCDO->InheritableGameplayEffectTags.CombinedTags)) continue;
		 }

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void APTWPlayerState::ApplyInvincible(float Duration)
{
	if (!IsValid(AbilitySystemComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyInvincible 실패: ASC가 없습니다."));
		return;
	}

	if (!IsValid(InvincibleEffectClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyInvincible 실패: InvincibleEffectClass가 없습니다."));
		return;
	}
	
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
		InvincibleEffectClass, 1.0f, Context);

	if (!Spec.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyInvincible 실패: Spec 생성에 실패했습니다."));
		return;
	}
	
	Spec.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag("Data.Duration"), Duration);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void APTWPlayerState::SetDeathOrder(int32 Order)
{
	if (HasAuthority())
	{
		PlayerRoundData.DeathOrder = Order;
	}
}

void APTWPlayerState::SetTeamId(int32 TeamId)
{
	if (HasAuthority())
	{
		PlayerRoundData.TeamId = TeamId;
	}
}

void APTWPlayerState::AddKillCount(int32 KillCount)
{
	if (HasAuthority())
	{
		PlayerRoundData.KillCount += KillCount;
	}
}

void APTWPlayerState::AddDeathCount(int32 DeathCount)
{
	if (HasAuthority())
	{
		PlayerRoundData.DeathCount += DeathCount;
	}
}

void APTWPlayerState::AddScore(int32 AddScore)
{
	if (HasAuthority())
	{
		PlayerRoundData.Score += AddScore;
	}
}

void APTWPlayerState::ResetRoundData()
{
	if (HasAuthority())
	{
		PlayerRoundData = FPTWPlayerRoundData();
	}
}

void APTWPlayerState::VotePredictedPlayer(FUniqueNetIdRepl PredictedPlayer)
{
	if (HasAuthority())
	{
		LobbyItemData.PredictedData.PredictedPlayer = PredictedPlayer.ToString();
	}
	else
	{
		ServerVotePredictedPlayer(PredictedPlayer);
	}
}

void APTWPlayerState::ResetInventoryItemId()
{
	if (HasAuthority())
	{
		CurrentPlayerData.InventoryItemIDs.Empty();
	}
}

void APTWPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	bIsReadyToPlay = false;
}

void APTWPlayerState::OnRep_CurrentPlayerData()
{
	OnPlayerDataUpdated.Broadcast(CurrentPlayerData);
}

void APTWPlayerState::OnRep_PlayerRoundData()
{
	OnPlayerRoundDataUpdated.Broadcast(PlayerRoundData);
}

void APTWPlayerState::OnRep_LobbyItemData()
{
	
}

void APTWPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		GI->RegisterPlayerState(this);
	}
}

void APTWPlayerState::OnRep_UniqueId()
{
	Super::OnRep_UniqueId();
	OnPlayerUniqueIdReplicated.Broadcast(this, GetUniqueId().ToString());
}

void APTWPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnPlayerNameReplicated.Broadcast(this, GetPlayerName());
}

void APTWPlayerState::OnRep_Owner()
{
	Super::OnRep_Owner();
	OnPlayerOwnerReplicated.Broadcast(this, GetOwner());
}

void APTWPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APTWPlayerState::ServerRequestPurchase_Implementation(APTWShopNPC* ShopNPC, FName ItemID, int32 Cost)
{
	FString NewItemIDStr = ItemID.ToString();

	if (NewItemIDStr.StartsWith(TEXT("Active_"), ESearchCase::IgnoreCase))
	{
		for (const FString& OwnedID : CurrentPlayerData.InventoryItemIDs)
		{
			if (OwnedID.StartsWith(TEXT("Active_"), ESearchCase::IgnoreCase))
			{
				UE_LOG(LogTemp, Warning, TEXT("[Purchase Blocked] Player already has an Active Item: %s"), *OwnedID);

				return;
			}
		}
	}
	if (CurrentPlayerData.Gold >= Cost)
	{
		CurrentPlayerData.Gold -= Cost;

		if (NewItemIDStr.StartsWith(TEXT("Chaos_"), ESearchCase::IgnoreCase))
		{
			if (APTWGameState* GameState = GetWorld()->GetGameState<APTWGameState>())
			{
				FPTWChaosItemEntry ChaosEntry;

				ChaosEntry.ItemId = ItemID;
				ChaosEntry.PlayerName = GetPlayerName();

				GameState->AddChaosItemEntry(ChaosEntry);
				UE_LOG(LogTemp, Log, TEXT("[Chaos Item] 카오스 아이템 구매 성공! GameState에 등록됨: %s (구매자: %s)"), *NewItemIDStr, *GetPlayerName());
			}
		}
		else if (NewItemIDStr.StartsWith(TEXT("Lobby_"), ESearchCase::IgnoreCase))
		{
			if (APTWLobbyGameMode* LobbyGameMode = Cast<APTWLobbyGameMode>(GetWorld()->GetAuthGameMode()))
			{
				LobbyGameMode->ApplyLobbyItem(this, ItemID);
				UE_LOG(LogTemp, Log, TEXT("[Lobby Item] 로비 아이템 구매 성공! (구매자: %s)"), *GetPlayerName());
			}
		}
		else
		{
			CurrentPlayerData.InventoryItemIDs.Add(NewItemIDStr);
			UE_LOG(LogTemp, Log, TEXT("[Inventory Item] 일반 아이템 구매 성공! 인벤토리에 등록됨: %s"), *NewItemIDStr);
		}

		FString DebugMsg = FString::Printf(TEXT("[Server] 구매 성공! 아이템: [%s] -> 구매자: [%s]"),
			*NewItemIDStr,
			*GetPlayerName()
		);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DebugMsg);
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMsg);

		OnRep_CurrentPlayerData();
		ForceNetUpdate();

		ClientPurchaseSuccess(ShopNPC);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Purchase Failed] Not enough gold."));
	}
}

void APTWPlayerState::ClientPurchaseSuccess_Implementation(APTWShopNPC* ShopNPC)
{
	if (ShopNPC)
	{
		ShopNPC->CloseShop();
	}
}

void APTWPlayerState::AddGold(int32 Amount)
{
	if (HasAuthority())
	{
		CurrentPlayerData.Gold += Amount;

		OnPlayerDataUpdated.Broadcast(CurrentPlayerData);
		ForceNetUpdate();
	}
}

void APTWPlayerState::ClearGAS()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	AbilitySystemComponent->ClearAllAbilities();

	FGameplayEffectQuery Query;
	AbilitySystemComponent->RemoveActiveEffects(Query);

	AbilitySystemComponent->InitAbilityActorInfo(this, nullptr);

	UE_LOG(LogTemp, Warning, TEXT("[GAS] %s 의 GAS 캐시가 완벽하게 초기화되었습니다! (전생 기억 삭제)"), *GetPlayerName());
}
