// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.h"
#include "GameFramework/GameStateBase.h"
#include "ColourWarsGameState.generated.h"

/**
 * 
 */
UCLASS()
class COLOURWARS_API AColourWarsGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

private:

	eMoveType SelectedMove;

	AColourWarsBlock* SelectedBlock;
	
	AColourWarsBlock* SecondarySelectedBlock;

	eBlockType CurrentPlayer = eBlockType::Red;

	/** Grid that owns us */
	AColourWarsBlockGrid* GameGrid;

	bool GameOver = false;

public:

	AColourWarsGameState();

	void SetGameGrid(AColourWarsBlockGrid* grid);

	AColourWarsBlockGrid* GetGameGrid();

	void SetSelectedMove(eMoveType MoveType);

	void UnsetSelectedMove();

	bool IsMoveSelected();

	void SelectBlock(AColourWarsBlock* block);

	void DeselectBlock(AColourWarsBlock* block);

	void DeselectAllBlocks();

	AColourWarsBlock* GetSelectedBlock();

	bool IsBlockSelected();

	void SetCurrentPlayer(eBlockType blockType);

	eBlockType GetCurrentPlayer();

	bool GetGameOver();

	void SetGameOver(bool gameOver);

	void RefreshGameGrid();

	void MakeMove();

};
