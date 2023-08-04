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

	TArray<AColourWarsBlock*> SelectedBlocks;

	eBlockType CurrentPlayer = eBlockType::Red;

	/** Grid that owns us */
	AColourWarsBlockGrid* GameGrid;

	bool GameOver = false;

public:

	const static TMap<eMoveType, int32> NumberBlocksRequired;

	AColourWarsGameState();

	void SetGameGrid(AColourWarsBlockGrid* grid);

	AColourWarsBlockGrid* GetGameGrid();

	UFUNCTION(BluePrintCallable, BluePrintPure)
		eMoveType GetSelectedMove();

	void SetSelectedMove(eMoveType MoveType);

	void UnsetSelectedMove();

	bool IsMoveSelected();

	int32 NumberBlocksSelected();

	void SelectBlock(AColourWarsBlock* block);

	void DeselectBlock(AColourWarsBlock* block);

	void DeselectAllBlocks();

	TArray<AColourWarsBlock*> GetSelectedBlocks();

	bool CorrectNumberBlocksSelected();

	void SetCurrentPlayer(eBlockType blockType);

	eBlockType GetCurrentPlayer();

	bool GetGameOver();

	void SetGameOver(bool gameOver);

	void RefreshGameGrid();

	void MakeMove();

	UFUNCTION(BlueprintCallable, BluePrintPure)
		bool MoveIsValid();

};
