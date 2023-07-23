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

private:
	int32 NumberOfPlayers;

public:
	AColourWarsGameMode();

	/** Pointer to game instance */
	UPROPERTY()
		class UColourWarsGameInstance* GameInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	eBlockType CurrentPlayer = eBlockType::Red;

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

	/** Pointer to blue material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* PurpleMaterial;

	/** Pointer to orange material used on active blocks */
	UPROPERTY()
	class UMaterialInstance* OrangeMaterial;

	/** Grid that owns us */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game")
		class AColourWarsBlockGrid* GameGrid;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
		bool GameOver = false;

	void NextTurn();

	void IncrementPlayer();
	
	void ApplyCapitalBlockBonus();

	void EndGame(eBlockType BlockType);

	UFUNCTION(BlueprintCallable)
	int32 GetNumberOfPlayers();

	class UMaterialInstance* GetPlayerColour(eBlockType BlockType);
};



