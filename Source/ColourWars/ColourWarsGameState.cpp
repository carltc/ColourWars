// Fill out your copyright notice in the Description page of Project Settings.


#include "ColourWarsGameState.h"

const TMap<eMoveType, int32> AColourWarsGameState::NumberBlocksRequired
{
	{eMoveType::Invalid , 0 },
	{eMoveType::AddOne  , 1 },
	{eMoveType::Move    , 2 },
	{eMoveType::Combine , 1 }
};

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

eMoveType AColourWarsGameState::GetSelectedMove()
{
	return SelectedMove;
}

void AColourWarsGameState::SetSelectedMove(eMoveType MoveType)
{
	SelectedMove = MoveType;
	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlocks);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Selected Move set."));
}

void AColourWarsGameState::UnsetSelectedMove()
{
	SelectedMove = eMoveType::Move;
	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlocks);
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

TArray<AColourWarsBlock*> AColourWarsGameState::GetSelectedBlocks()
{
	return SelectedBlocks;
}

int32 AColourWarsGameState::NumberBlocksSelected()
{
	return SelectedBlocks.Num();
}

void AColourWarsGameState::ToggleBlockSelection(AColourWarsBlock* block)
{
	// Check if block has already been selected
	if (SelectedBlocks.Contains(block))
	{
		// Only deselect this block if it is the last block selected
		if (SelectedBlocks.Last() == block)
		{
			DeselectBlock(block);
		}
		// If it is the first block then reset the selection
		else if (SelectedBlocks[0] == block)
		{
			UnsetSelectedMove();
			DeselectAllBlocks();
		}
		else
		{
			return;
		}
	}
	else
	{
		// Check if a Move has been selected
		if (IsMoveSelected())
		{
			// Check if the full number of blocks has been selected
			if (SelectedBlocks.Num() >= NumberBlocksRequired[SelectedMove])
			{
				// Replace the last block with this block
				DeselectBlock(SelectedBlocks.Last());
			}

			// Select this block
			SelectBlock(block);
		}
		else
		{
			// Just set the block as the only selected
			DeselectAllBlocks();
			SelectBlock(block);
		}
	}

	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlocks);
}

void AColourWarsGameState::SelectBlock(AColourWarsBlock* block)
{
	SelectedBlocks.Add(block);
	block->SetBlockSelected();
}

void AColourWarsGameState::DeselectBlock(AColourWarsBlock* block)
{
	SelectedBlocks.Remove(block);
	block->SetBlockDeselected();
	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlocks);
}

void AColourWarsGameState::DeselectAllBlocks()
{
	GetGameGrid()->DeselectAllBlocks();

	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlocks);
}

bool AColourWarsGameState::CorrectNumberBlocksSelected()
{
	return SelectedBlocks.Num() == NumberBlocksRequired[SelectedMove];
}

void AColourWarsGameState::RefreshGameGrid()
{
	GameGrid->SetSelectableBlocks(SelectedMove, SelectedBlocks);
}

void AColourWarsGameState::MakeMove()
{
	if (MoveIsValid())
	{
		switch (SelectedMove)
		{
			case eMoveType::AddOne:
				GetGameGrid()->AddOneToBlock(SelectedBlocks[0]);
				break;
			case eMoveType::Move:
				GetGameGrid()->MoveBlock(SelectedBlocks[0], SelectedBlocks[1]);
				break;
			case eMoveType::Combine:
				GetGameGrid()->CombineNeighbourBlocks(SelectedBlocks[0]);
				break;
		}
	}
}

bool AColourWarsGameState::MoveIsValid() 
{
	if (!IsMoveSelected())
	{
		return false;
	}

	if (SelectedBlocks.Num() >= NumberBlocksRequired[SelectedMove])
	{
		/*switch (SelectedMove)
		{
			case eMoveType::Move:
				SelectedBlocks[0]->SetBlockScoreText(1);
				break;
		}*/

		return true;
	}

	return false;
}



