// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Game/GameMode/PTWTransitionGameMode.h"

APTWTransitionGameMode::APTWTransitionGameMode()
{
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = true;
	bUseSeamlessTravel = true;
}
