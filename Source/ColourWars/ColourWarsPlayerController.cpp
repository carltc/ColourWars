// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AColourWarsPlayerController::AColourWarsPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AColourWarsPlayerController::EndTurn()
{
	GetGameMode()->NextTurn();

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Turn ended."));
}

void AColourWarsPlayerController::SetMove(eMoveType MoveType)
{
	if (MoveType == eMoveType::Invalid)
	{
		GetGameState()->UnsetSelectedMove();
		GetGameState()->DeselectAllBlocks();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Move Unset."));
	}
	else
	{
		GetGameState()->SetSelectedMove(MoveType);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Move Set."));
	}
}

AColourWarsGameState* AColourWarsPlayerController::GetGameState()
{
	if (GameState == nullptr)
	{
		// Set the gamestate
		GameState = Cast<AColourWarsGameState>(UGameplayStatics::GetGameState(GetWorld()));
	}

	return GameState;
}

AColourWarsGameMode* AColourWarsPlayerController::GetGameMode()
{
	if (GameMode == nullptr)
	{
		// Set the gamemode
		GameMode = Cast<AColourWarsGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}

	return GameMode;
}
