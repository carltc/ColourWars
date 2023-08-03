// Fill out your copyright notice in the Description page of Project Settings.


#include "ColourWarsGameState.h"

AColourWarsGameState::AColourWarsGameState()
{
	// Set starting player
	//CurrentPlayer = eBlockType::Red;
}

void AColourWarsGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AColourWarsGameState::SetGameGrid(AColourWarsBlockGrid* grid)
{
	GameGrid = grid;
}

AColourWarsBlockGrid* AColourWarsGameState::GetGameGrid()
{
	return GameGrid;
}

void AColourWarsGameState::SetSelectedMove(eMoveType MoveType)
{
	SelectedMove = MoveType;
	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlock);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Selected Move set."));
}

void AColourWarsGameState::UnsetSelectedMove()
{
	SelectedMove = eMoveType::Invalid;
	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlock);
}

bool AColourWarsGameState::IsMoveSelected()
{
	return SelectedMove != eMoveType::Invalid;
}

bool AColourWarsGameState::GetGameOver()
{
	return GameOver;
}

void AColourWarsGameState::SetGameOver(bool gameOver)
{
	GameOver = gameOver;
}

eBlockType AColourWarsGameState::GetCurrentPlayer()
{
	return CurrentPlayer;
}

void AColourWarsGameState::SetCurrentPlayer(eBlockType blockType)
{
	CurrentPlayer = blockType;
}

AColourWarsBlock* AColourWarsGameState::GetSelectedBlock()
{
	return SelectedBlock;
}

void AColourWarsGameState::SelectBlock(AColourWarsBlock* block)
{
	// If both blocks are selected then replace the 2nd block
	if (SelectedBlock != nullptr && SecondarySelectedBlock != nullptr)
	{
		// If the 1st block has been clicked on then do nothing
		if (block == SelectedBlock)
		{
			return;
		}


		// If 2nd block clicked on then deselect it
		if (block == SecondarySelectedBlock)
		{
			DeselectBlock(SecondarySelectedBlock);
		}
		// Otherwise replace the 2nd block with the new block
		else
		{
			DeselectBlock(SecondarySelectedBlock);

			SecondarySelectedBlock = block;
			block->SetBlockSelected();
		}
	}
	// If 1st block only is selected
	else if (SelectedBlock != nullptr && SecondarySelectedBlock == nullptr)
	{
		// If 1st block clicked on then unselect it
		if (block == SelectedBlock)
		{
			DeselectBlock(SelectedBlock);
		}
		// If another block is clicked on then set as 2nd selection
		else
		{
			SecondarySelectedBlock = block;
			block->SetBlockSelected();
		}
	}
	// Is no blocks are selected then set this block as 1st block
	else
	{
		SelectedBlock = block;
		block->SetBlockSelected();
	}

	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlock);
}

void AColourWarsGameState::DeselectBlock(AColourWarsBlock* block)
{
	if (block == SelectedBlock)
	{
		SelectedBlock = nullptr;
		block->SetBlockDeselected();
	}

	if (block == SecondarySelectedBlock)
	{
		SecondarySelectedBlock = nullptr;
		block->SetBlockDeselected();
	}
}

void AColourWarsGameState::DeselectAllBlocks()
{
	GetGameGrid()->DeselectAllBlocks();

	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlock);
}

bool AColourWarsGameState::IsBlockSelected()
{
	return SelectedBlock != nullptr;
}

void AColourWarsGameState::RefreshGameGrid()
{
	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlock);
}

void AColourWarsGameState::MakeMove()
{
	switch (SelectedMove)
	{
		case eMoveType::Attack:
			GetGameGrid()->MoveBlock(SelectedBlock, SecondarySelectedBlock);
			break;
	}
}



