// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Pickup/PTWPickupCoin.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerState.h"

APTWPickupCoin::APTWPickupCoin()
{
	GoldAmount = 100;
}

void APTWPickupCoin::OnPickedUp(APTWPlayerCharacter* Player)
{
	if (Player)
	{
		if (APTWPlayerState* PS = Player->GetPlayerState<APTWPlayerState>())
		{
			PS->AddGold(GoldAmount);
		}
	}
}
