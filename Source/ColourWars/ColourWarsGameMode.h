// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ColourWarsBlock.h"
#include "GameFramework/GameModeBase.h"
#include "ColourWarsGameMode.generated.h"

/** GameMode class to specify pawn and playercontroller */
UCLASS(minimalapi)
class AColourWarsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AColourWarsGameMode();

	AColourWarsBlock::eBlockType CurrentPlayer = AColourWarsBlock::eBlockType::Red;

	/** Pointer to white material used on the focused block */
	UPROPERTY()
	class UMaterial* BaseMaterial;

	/** Pointer to red material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* RedMaterial;

	/** Pointer to green material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* GreenMaterial;

	/** Pointer to blue material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* BlueMaterial;

	/** Pointer to orange material used on active blocks */
	UPROPERTY()
	class UMaterialInstance* OrangeMaterial;

	/** Grid that owns us */
	UPROPERTY()
		class AColourWarsBlockGrid* GameGrid;

	void NextTurn();

	void IncrementPlayer();

	class UMaterialInstance* GetPlayerColour(AColourWarsBlock::eBlockType BlockType);
};



