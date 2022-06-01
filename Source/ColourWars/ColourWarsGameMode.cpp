// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsGameMode.h"
#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.h"
#include "ColourWarsPlayerController.h"
#include "ColourWarsPawn.h"

AColourWarsGameMode::AColourWarsGameMode()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> RedMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> GreenMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> OrangeMaterial;
		FConstructorStatics()
			: BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
			, RedMaterial(TEXT("/Game/Puzzle/Meshes/RedMaterial.RedMaterial"))
			, GreenMaterial(TEXT("/Game/Puzzle/Meshes/GreenMaterial.GreenMaterial"))
			, BlueMaterial(TEXT("/Game/Puzzle/Meshes/BlueMaterial.BlueMaterial"))
			, OrangeMaterial(TEXT("/Game/Puzzle/Meshes/OrangeMaterial.OrangeMaterial"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Save a pointer to the orange material
	BaseMaterial = ConstructorStatics.BaseMaterial.Get();
	RedMaterial = ConstructorStatics.RedMaterial.Get();
	GreenMaterial = ConstructorStatics.GreenMaterial.Get();
	BlueMaterial = ConstructorStatics.BlueMaterial.Get();
	OrangeMaterial = ConstructorStatics.OrangeMaterial.Get();

	// no pawn by default
	DefaultPawnClass = AColourWarsPawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = AColourWarsPlayerController::StaticClass();
}

void AColourWarsGameMode::NextTurn()
{
	GameGrid->UpdateScore();

	// Switch current player block type to next type
	IncrementPlayer();
}

void AColourWarsGameMode::IncrementPlayer()
{
	int32 playerInt = static_cast<int32>(CurrentPlayer);
	int32 increments = 0;

	// Increment player and check if they have blocks left and if not increment again until a player does
	do 
	{
		playerInt++;
		increments++;

		if (playerInt > 3)
		{
			playerInt = 1;
		}

		CurrentPlayer = static_cast<eBlockType>(playerInt);
	} 
	while (!GameGrid->HasBlocks(CurrentPlayer));

	// If 2 players were incremented through (back to starting player) then the game is over and a player has won.
	if (increments > 1)
	{
		EndGame(CurrentPlayer);
	}

	// Set the player turn mesh
	GameGrid->PlayerTurnMesh->SetMaterial(0, GetPlayerColour(CurrentPlayer));
}

void AColourWarsGameMode::EndGame(eBlockType BlockType)
{
	GameOver = true;
}

class UMaterialInstance* AColourWarsGameMode::GetPlayerColour(eBlockType BlockType)
{
	switch (BlockType)
	{
		case eBlockType::Red:
			return RedMaterial;
		case eBlockType::Green:
			return GreenMaterial;
		case eBlockType::Blue:
			return BlueMaterial;
	}

	return OrangeMaterial;
}
