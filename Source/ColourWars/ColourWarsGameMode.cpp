// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsGameMode.h"
#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.h"
#include "ColourWarsPlayerController.h"
#include "ColourWarsGameInstance.h"
#include "ColourWarsPawn.h"
#include "ColourWarsGameState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AColourWarsGameMode::AColourWarsGameMode()
{
	// no pawn by default
	DefaultPawnClass = AColourWarsPawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = AColourWarsPlayerController::StaticClass();
	// use our own game state class
	GameStateClass = AColourWarsGameState::StaticClass();
}

void AColourWarsGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AColourWarsGameMode::NextTurn()
{
	GameState->MakeMove();

	ApplyCapitalBlockBonus();

	// Deselect the selected blocks
	GameState->DeselectAllBlocks();

	GameState->GetGameGrid()->UpdateScore();

	// Switch current player block type to next type
	IncrementPlayer();
}

void AColourWarsGameMode::IncrementPlayer()
{
	int32 playerInt = static_cast<int32>(GameState->GetCurrentPlayer());
	int32 previousPlayerInt = static_cast<int32>(GameState->GetCurrentPlayer());

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Setting next player."));

	// Increment player and check if they have blocks left and if not increment again until a player does
	do
	{
		playerInt++;

		if (playerInt > GetNumberOfPlayers())
		{
			playerInt = 1;
		}

		GameState->SetCurrentPlayer(static_cast<eBlockType>(playerInt));
	} while (!GameState->GetGameGrid()->HasBlocks(GameState->GetCurrentPlayer()));

	// If it is still the same players turn after incrementing this means that it is the only player left
	if (previousPlayerInt == static_cast<int32>(GameState->GetCurrentPlayer()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Game over."));
		EndGame(GameState->GetCurrentPlayer());
	}

	// Set the player turn mesh
	GameState->GetGameGrid()->SetPlayerTurnMeshColour();

	GameState->RefreshGameGrid();
}

void AColourWarsGameMode::ApplyCapitalBlockBonus()
{
	GameState->GetGameGrid()->ApplyCapitalBlocksBonus();
}

void AColourWarsGameMode::EndGame(eBlockType BlockType)
{
	GetGameState()->SetGameOver(true);
}

void AColourWarsGameMode::SetGameGrid(AColourWarsBlockGrid* grid)
{
	GameState->SetGameGrid(grid);
}

int32 AColourWarsGameMode::GetNumberOfPlayers()
{
	if (GameInstance == nullptr)
	{
		// Set the game instance and number of players in this game
		GameInstance = Cast<UColourWarsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (GameInstance != nullptr)
		{
			NumberOfPlayers = GameInstance->NumberOfPlayers;
		}
	}

	return NumberOfPlayers;
}

AColourWarsGameState* AColourWarsGameMode::GetGameState()
{
	if (GameState == nullptr)
	{
		// Set the gamemode
		GameState = Cast<AColourWarsGameState>(UGameplayStatics::GetGameState(GetWorld()));
	}

	return GameState;
}

void AColourWarsGameMode::BeginGame()
{
	GameState->SetCurrentPlayer(eBlockType::Red);

	GameState->DeselectAllBlocks();

	GameState->SetSelectedMove(eMoveType::Move);

	GameState->RefreshGameGrid();
}

