// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ColourWarsBlock.h"
#include "ColourWarsGameState.h"
#include "ColourWarsGameMode.h"
#include "GameFramework/PlayerController.h"
#include "ColourWarsPlayerController.generated.h"

/** PlayerController class used to enable cursor */
UCLASS()
class AColourWarsPlayerController : public APlayerController
{
	GENERATED_BODY()

private:

	/** Pointer to game state */
	AColourWarsGameState* GameState;

	AColourWarsGameState* GetGameState();
	
	/** Pointer to game state */
	AColourWarsGameMode* GameMode;

	AColourWarsGameMode* GetGameMode();

public:
	AColourWarsPlayerController();

	UFUNCTION(BluePrintCallable, meta = (DisplayName = "End Turn"), Category = Moves)
		void EndTurn();

	UFUNCTION(BluePrintCallable, meta = (DisplayName = "Set Move"), Category = Moves)
		void SetMove(eMoveType MoveType);
};


