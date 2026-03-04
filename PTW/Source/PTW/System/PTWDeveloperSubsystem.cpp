// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWDeveloperSubsystem.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CoreFramework/PTWDummyBotController.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGamestate.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AIController.h"
#include "Engine/World.h"
#include "CoreFramework/Character/Component/PTWDeveloperComponent.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "System/PTWItemSpawnManager.h"

void UPTWDeveloperSubsystem::SendCommandToServer(FName CommandName, int32 IntParam, float FloatParam, const FString& StringParam)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (UPTWDeveloperComponent* DevComp = PC->FindComponentByClass<UPTWDeveloperComponent>())
	{
		DevComp->Server_ExecuteDevCommand(CommandName, IntParam, FloatParam, StringParam);
	}
}

void UPTWDeveloperSubsystem::ProcessServerCommand(APlayerController* InstigatorPC, FName CommandName, int32 IntParam, float FloatParam, const FString& StringParam)
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (GetWorld()->GetNetMode() == NM_Client) return;

	if (APTWGameState* GameState = World->GetGameState<APTWGameState>())
	{
		FString PlayerName = TEXT("Host");

		if (InstigatorPC && InstigatorPC->PlayerState)
		{
			PlayerName = InstigatorPC->PlayerState->GetPlayerName();
		}

		FString AlertMsg = FString::Printf(TEXT("개발자 기능 [%s] 실행됨! (요청: %s)"), *CommandName.ToString(), *PlayerName);

		GameState->Multicast_SystemMessage(AlertMsg);
	}

	if (CommandName == FName("AddBot"))
	{
		AGameModeBase* GameMode = World->GetAuthGameMode();
		if (!GameMode) return;

		UClass* BotPawnClass = GameMode->DefaultPawnClass;
		if (!BotPawnClass) return;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APTWDummyBotController* BotController = World->SpawnActor<APTWDummyBotController>(
			APTWDummyBotController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (BotController)
		{
			AActor* PlayerStart = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass());
			FVector SpawnLocation = PlayerStart ? PlayerStart->GetActorLocation() : FVector(0.f, 0.f, 300.f);

			APawn* BotPawn = World->SpawnActor<APawn>(BotPawnClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

			if (BotPawn)
			{
				BotController->Possess(BotPawn);

				if (APlayerState* BotPS = BotController->GetPlayerState<APlayerState>())
				{
					FString BotName = FString::Printf(TEXT("DummyBot_%d"), FMath::RandRange(1, 999));
					BotPS->SetPlayerName(BotName);
				}

				if (APTWGameMode* MyGM = Cast<APTWGameMode>(GameMode))
				{
					MyGM->HandlePlayerJoined(BotController);
				}

				UE_LOG(LogTemp, Warning, TEXT("[Dev] Dummy Bot Added on Server! Name: %s"), *BotController->GetPlayerState<APlayerState>()->GetPlayerName());
			}
			else
			{
				BotController->Destroy();
			}
		}
	}
	else if (CommandName == FName("RemoveBots"))
	{
		TArray<AActor*> FoundBots;
		UGameplayStatics::GetAllActorsOfClass(World, APTWDummyBotController::StaticClass(), FoundBots);

		for (AActor* Actor : FoundBots)
		{
			if (APTWDummyBotController* BotCon = Cast<APTWDummyBotController>(Actor))
			{
				if (APawn* ControlledPawn = BotCon->GetPawn())
				{
					ControlledPawn->Destroy();
				}
				BotCon->Destroy();
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[Dev] All Dummy Bots Removed! Total: %d"), FoundBots.Num());
	}
	else if (CommandName == FName("TeleportBots"))
	{
		if (!InstigatorPC) return;
		ACharacter* MyChar = Cast<ACharacter>(InstigatorPC->GetPawn());
		if (!MyChar) return;

		const FVector MyLocation = MyChar->GetActorLocation();

		TArray<AActor*> FoundBots;
		UGameplayStatics::GetAllActorsOfClass(World, APTWDummyBotController::StaticClass(), FoundBots);

		for (AActor* Actor : FoundBots)
		{
			if (APTWDummyBotController* BotCon = Cast<APTWDummyBotController>(Actor))
			{
				if (APawn* BotPawn = BotCon->GetPawn())
				{
					FVector RandomOffset = FMath::VRand() * FVector(200.f, 200.f, 0.f);
					BotPawn->SetActorLocation(MyLocation + RandomOffset);
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[Dev] Teleported %d bots to player on Server!"), FoundBots.Num());
	}
	else if (CommandName == FName("AddGold"))
	{
		if (APTWPlayerState* PS = InstigatorPC->GetPlayerState<APTWPlayerState>())
		{
			FPTWPlayerData PlayerData = PS->GetPlayerData();
			PlayerData.Gold += IntParam;
			PS->SetPlayerData(PlayerData);
		}
	}
	else if (CommandName == FName("ForceStart") || CommandName == FName("ForceEnd"))
	{
		if (APTWGameMode* GM = Cast<APTWGameMode>(World->GetAuthGameMode()))
		{
			GM->EndTimer();
			UE_LOG(LogTemp, Warning, TEXT("[Dev] Force End/Start! 진행 타이머 강제 종료."));
		}
	}
	else if (CommandName == FName("ForceEnd"))
	{
		World->ServerTravel(TEXT("/Game/_PTW/Maps/Lobby?listen"));
		UE_LOG(LogTemp, Warning, TEXT("[Dev] Force Quit! 로비로 강제 트래블 실행."));
	}
	else if (CommandName == FName("ForceWin"))
	{
		for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
		{
			AController* OtherController = It->Get();
			if (OtherController && OtherController != InstigatorPC)
			{
				if (APawn* OtherPawn = OtherController->GetPawn())
				{
					OtherPawn->Destroy();
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[Dev] Force Win! 나를 제외한 모든 캐릭터 파괴."));
	}
	else if (CommandName == FName("ForceLose"))
	{
		for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
		{
			if (AController* Controller = It->Get())
			{
				if (APawn* Pawn = Controller->GetPawn())
				{
					Pawn->Destroy();
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[Dev] Force Lose! 전원 캐릭터 파괴 (모두 패배)."));
	}
	else if (CommandName == FName("ToggleGodMode"))
	{
		if (InstigatorPC)
		{
			if (ACharacter* MyChar = Cast<ACharacter>(InstigatorPC->GetPawn()))
			{
				if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyChar))
				{
					FGameplayTag GodModeTag = GameplayTags::State::Invincible;

					if (ASC->HasMatchingGameplayTag(GodModeTag))
					{
						ASC->RemoveLooseGameplayTag(GodModeTag);
						UE_LOG(LogTemp, Warning, TEXT("[Dev] God Mode (GAS Tag): OFF"));
					}
					else
					{
						ASC->AddLooseGameplayTag(GodModeTag);
						UE_LOG(LogTemp, Warning, TEXT("[Dev] God Mode (GAS Tag): ON"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[Dev] God Mode Failed: Ability System Component not found on Character!"));
				}
			}
		}
	}
	else if (CommandName == FName("SetGameSpeed"))
	{
		UGameplayStatics::SetGlobalTimeDilation(World, FloatParam);
	}
	else if (CommandName == FName("SetSuperSpeed"))
	{
		if (InstigatorPC)
		{
			if (ACharacter* MyChar = Cast<ACharacter>(InstigatorPC->GetPawn()))
			{
				MyChar->GetCharacterMovement()->MaxWalkSpeed = FloatParam;
			}
		}
		}
	else if (CommandName == FName("SetSuperJump"))
	{
		if (InstigatorPC)
		{
			if (ACharacter* MyChar = Cast<ACharacter>(InstigatorPC->GetPawn()))
			{
				MyChar->GetCharacterMovement()->JumpZVelocity = FloatParam;
			}
		}
		}
	else if (CommandName == FName("ToggleFly"))
	{
		if (InstigatorPC)
		{
			if (ACharacter* MyChar = Cast<ACharacter>(InstigatorPC->GetPawn()))
			{
				auto* MoveComp = MyChar->GetCharacterMovement();
				if (MoveComp->MovementMode == MOVE_Flying)
					MoveComp->SetMovementMode(MOVE_Walking);
				else
					MoveComp->SetMovementMode(MOVE_Flying);
			}
		}
		}
	else if (CommandName == FName("KillSelf"))
	{
		if (InstigatorPC)
		{
			if (APawn* MyPawn = InstigatorPC->GetPawn())
			{
				MyPawn->Destroy();
			}
		}
		}
	else if (CommandName == FName("GrantItem"))
	{
		if (InstigatorPC)
		{
			if (APTWPlayerState* PS = InstigatorPC->GetPlayerState<APTWPlayerState>())
			{
				if (UPTWItemSpawnManager* SpawnManager = World->GetSubsystem<UPTWItemSpawnManager>())
				{
					SpawnManager->SpawnItemByID(PS, StringParam);

					UE_LOG(LogTemp, Warning, TEXT("[Dev] Server granted Item: %s to Player!"), *StringParam);
				}
			}
		}
	}
}

ACharacter* UPTWDeveloperSubsystem::GetLocalPlayerCharacter() const
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		return Cast<ACharacter>(PC->GetPawn());
	}
	return nullptr;
}

// 서버에서 실행
void UPTWDeveloperSubsystem::AddDummyBot() 
{
	SendCommandToServer(FName("AddBot"));
}
void UPTWDeveloperSubsystem::RemoveAllDummyBots() 
{ 
	SendCommandToServer(FName("RemoveBots"));
}
void UPTWDeveloperSubsystem::TeleportBotsToMe() 
{ 
	SendCommandToServer(FName("TeleportBots"));
}
void UPTWDeveloperSubsystem::AddGold(int32 Amount) 
{ 
	SendCommandToServer(FName("AddGold"), Amount);
}
void UPTWDeveloperSubsystem::SetGameSpeed(float SpeedMultiplier)
{
	SendCommandToServer(FName("SetGameSpeed"), 0, SpeedMultiplier);
}
void UPTWDeveloperSubsystem::ForceGameStart()
{
	SendCommandToServer(FName("ForceStart"));
}
void UPTWDeveloperSubsystem::ForceRoundEnd()
{
	SendCommandToServer(FName("ForceEnd"));
}
void UPTWDeveloperSubsystem::ForceWin()
{
	SendCommandToServer(FName("ForceWin"));
}
void UPTWDeveloperSubsystem::ForceLose()
{
	SendCommandToServer(FName("ForceLose"));
}
void UPTWDeveloperSubsystem::ToggleGodMode()
{
	SendCommandToServer(FName("ToggleGodMode"));
}
void UPTWDeveloperSubsystem::SetSuperSpeed(float SpeedMultiplier) 
{ 
	SendCommandToServer(FName("SetSuperSpeed"), 0, SpeedMultiplier); 
}
void UPTWDeveloperSubsystem::SetSuperJump(float JumpZVelocity) 
{ 
	SendCommandToServer(FName("SetSuperJump"), 0, JumpZVelocity); 
}
void UPTWDeveloperSubsystem::ToggleFlyMode() 
{ 
	SendCommandToServer(FName("ToggleFly")); 
}
void UPTWDeveloperSubsystem::KillSelf() 
{ 
	SendCommandToServer(FName("KillSelf")); 
}
void UPTWDeveloperSubsystem::GrantItem(FString ItemID) 
{ 
	SendCommandToServer(FName("GrantItem"), 0, 0.f, ItemID); 
}

// 클라이언트에서만 실행
void UPTWDeveloperSubsystem::TogglePingAndFPS()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (bIsStatVisible)
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("stat none"), PC);
		bIsStatVisible = false;
	}
	else
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("stat fps"), PC);
		UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("stat net"), PC);
		bIsStatVisible = true;
	}
}
void UPTWDeveloperSubsystem::ToggleHUD()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("showhud"), PC);

	bIsHUDVisible = !bIsHUDVisible;

	FString StateStr = bIsHUDVisible ? TEXT("ON") : TEXT("OFF");
	UE_LOG(LogTemp, Warning, TEXT("[Dev] HUD Visibility: %s"), *StateStr);
}

void UPTWDeveloperSubsystem::ToggleCollisionRender()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), TEXT("show collision"), PC);

	bIsCollisionVisible = !bIsCollisionVisible;

	FString StateStr = bIsCollisionVisible ? TEXT("ON") : TEXT("OFF");
	UE_LOG(LogTemp, Warning, TEXT("[Dev] Collision Render: %s"), *StateStr);
}


