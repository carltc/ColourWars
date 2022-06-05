// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsGameMode.h"
#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.h"
#include "ColourWarsPlayerController.h"
#include "ColourWarsGameInstance.h"
#include "ColourWarsPawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AColourWarsGameMode::AColourWarsGameMode()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> RedMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> GreenMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> PurpleMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> OrangeMaterial;
		FConstructorStatics()
			: BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
			, RedMaterial(TEXT("/Game/Puzzle/Meshes/RedMaterial.RedMaterial"))
			, GreenMaterial(TEXT("/Game/Puzzle/Meshes/GreenMaterial.GreenMaterial"))
			, BlueMaterial(TEXT("/Game/Puzzle/Meshes/BlueMaterial.BlueMaterial"))
			, PurpleMaterial(TEXT("/Game/Puzzle/Meshes/PurpleMaterial.PurpleMaterial"))
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
	PurpleMaterial = ConstructorStatics.PurpleMaterial.Get();
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
	int32 previousPlayerInt = static_cast<int32>(CurrentPlayer);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Setting next player."));

	// Increment player and check if they have blocks left and if not increment again until a player does
	do 
	{
		playerInt++;

		if (playerInt > GetNumberOfPlayers())
		{
			playerInt = 1;
		}

		CurrentPlayer = static_cast<eBlockType>(playerInt);
	} 
	while (!GameGrid->HasBlocks(CurrentPlayer));

	// If it is still the same players turn after incrementing this means that it is the only player left
	if (previousPlayerInt == static_cast<int32>(CurrentPlayer))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Game over."));
		EndGame(CurrentPlayer);
	}

	// Set the player turn mesh
	GameGrid->PlayerTurnMesh->SetMaterial(0, GetPlayerColour(CurrentPlayer));
}

void AColourWarsGameMode::EndGame(eBlockType BlockType)
{
	GameOver = true;
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
		case eBlockType::Purple:
			return PurpleMaterial;
	}

	return OrangeMaterial;
}
