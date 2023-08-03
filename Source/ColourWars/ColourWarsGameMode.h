// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ColourWarsBlock.h"
#include "ColourWarsGameState.h"
#include "GameFramework/GameModeBase.h"
#include "ColourWarsGameMode.generated.h"

/** GameMode class to specify pawn and playercontroller */
UCLASS(minimalapi)
class AColourWarsGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

private:
	int32 NumberOfPlayers;

	/** Pointer to game state */
	AColourWarsGameState* GameState;

public:
	AColourWarsGameMode();

	/** Pointer to game instance */
	UPROPERTY()
		class UColourWarsGameInstance* GameInstance;
	
	void NextTurn();

	void IncrementPlayer();
	
	void ApplyCapitalBlockBonus();

	void EndGame(eBlockType BlockType);

	void SetGameGrid(AColourWarsBlockGrid* grid);

	UFUNCTION(BlueprintCallable)
	int32 GetNumberOfPlayers();

	AColourWarsGameState* GetGameState();

	void BeginGame();

};



