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
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> RedMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> GreenMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> OrangeMaterial;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			, BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
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
	// Switch current player block type to next type
	IncrementPlayer();
}

void AColourWarsGameMode::IncrementPlayer()
{
	int32 playerInt = static_cast<int32>(CurrentPlayer);

	playerInt++;

	if (playerInt > 3)
	{
		playerInt = 1;
	}

	CurrentPlayer = static_cast<AColourWarsBlock::eBlockType>(playerInt);

	// Set the player turn mesh
	GameGrid->PlayerTurnMesh->SetMaterial(0, GetPlayerColour(CurrentPlayer));
}

class UMaterialInstance* AColourWarsGameMode::GetPlayerColour(AColourWarsBlock::eBlockType BlockType)
{
	switch (BlockType)
	{
		case AColourWarsBlock::eBlockType::Red:
			return RedMaterial;
		case AColourWarsBlock::eBlockType::Green:
			return GreenMaterial;
		case AColourWarsBlock::eBlockType::Blue:
			return BlueMaterial;
	}

	return OrangeMaterial;
}
