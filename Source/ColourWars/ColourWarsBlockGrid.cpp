// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsBlockGrid.h"
#include "ColourWarsBlock.h"
#include "ColourWarsPawn.h"
#include "ColourWarsGameMode.h"
#include "ColourWarsGameInstance.h"
#include "IntVector.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "cmath"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

AColourWarsBlockGrid::AColourWarsBlockGrid()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlockMaterial;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			, BlockMaterial(TEXT("/Game/Puzzle/Meshes/BlockMaterial_Inst.BlockMaterial_Inst"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Set defaults
	Size = 5;

	// Create static mesh component
	ScoreText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreText0"));
	ScoreText->SetRelativeLocation(FVector(0.f, 0.f, -200.f));
	ScoreText->SetRelativeRotation(FRotator(90.f,0.f,0.f));
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "<Score Unset>"), FText::AsNumber(0)));
	ScoreText->SetupAttachment(DummyRoot);
	ScoreText->HorizontalAlignment = EHorizTextAligment::EHTA_Center;

	// Create static mesh component
	PlayerTurnMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerTurnMesh0"));
	PlayerTurnMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	PlayerTurnMesh->SetRelativeScale3D(FVector(1.f, 6.f, 0.2f));
	PlayerTurnMesh->SetRelativeLocation(FVector(-911.f, 0.f, 0.f));
	PlayerTurnMesh->SetupAttachment(DummyRoot);
	PlayerTurnMesh->SetMaterial(0, ConstructorStatics.BlockMaterial.Get());
}

void AColourWarsBlockGrid::BeginPlay()
{
	Super::BeginPlay();

	// Set the player pawn
	PlayerPawn = Cast<AColourWarsPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	// Set the gamemode
	GameMode = Cast<AColourWarsGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	
	// Number of blocks
	GetGameGridSize();
	const int32 NumBlocks = Size * Size;
	BlockSpacing = 1500.f / Size; 
	BlocksScale = 4.f / Size;

	// Set starting block type
	int32 BlockTypeInt = 1;

	// Create list to be used to randomly assign block types
	TArray<eBlockType> BlockTypes;
	for (int32 BlockIndex = 0; BlockIndex < NumBlocks; BlockIndex++)
	{
		// Get block type
		eBlockType BlockType = static_cast<eBlockType>(BlockTypeInt);
		BlockTypes.Add(BlockType);

		// Increment the block type
		BlockTypeInt++;
		if (BlockTypeInt > GameMode->GetNumberOfPlayers())
		{
			BlockTypeInt = 1;
		}
	}

	int32 max;
	srand(time(0));

	// Loop to spawn each block
	for(int32 BlockIndex=0; BlockIndex<NumBlocks; BlockIndex++)
	{
		//const float HalfSize = ((float)Size-1.0f)/2.0f;
		const float Xcoord = BlockIndex/Size; // Divide by dimension
		const float Ycoord = BlockIndex%Size; // Modulo gives remainder

		// Get random blocktype
		max = BlockTypes.Num();
		int32 RandomIndex = rand() % max;

		IntVector newGridCoord = IntVector(Xcoord, Ycoord);

		// Spawn a block
		//SpawnNewBlock(BlockTypes[RandomIndex], newGridCoord, 0);
		SpawnNewBlock(eBlockType::None, newGridCoord, 0);

		// Remove block type just used
		BlockTypes.RemoveAt(RandomIndex);
	}

	// Set the capital blocks of each player
	this->SetCapitalBlocks();

	this->UpdateScore();

	// Set the game grid as this newly created grid
	GameMode->GetGameState()->SetGameGrid(this);

	// Set the playerturn mesh to the starting player colour
	SetPlayerTurnMeshColour();

	GameMode->BeginGame();
}

void AColourWarsBlockGrid::UpdateScore()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Updating score."));

	// Calculate the score of each player
	int32 redScore = 0;
	int32 greenScore = 0;
	int32 blueScore = 0;
	int32 purpleScore = 0;
	for (int32 BlockIndex = 0; BlockIndex < Blocks.Num(); BlockIndex++)
	{
		AColourWarsBlock* block = Blocks[BlockIndex];
		switch (block->GetBlockType())
		{
			case eBlockType::Red:
				redScore += block->GetScore();
				break;
			case eBlockType::Green:
				greenScore += block->GetScore();
				break;
			case eBlockType::Blue:
				blueScore += block->GetScore();
				break;
			case eBlockType::Purple:
				purpleScore += block->GetScore();
				break;
		}
	}

	// Update text
	if (GameMode->GetNumberOfPlayers() == 2)
	{
		ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "Red:{0} Green:{1}"), FText::AsNumber(redScore), FText::AsNumber(greenScore)));
	}
	else if (GameMode->GetNumberOfPlayers() == 3)
	{
		ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "Red:{0} Green:{1} Blue:{2}"), FText::AsNumber(redScore), FText::AsNumber(greenScore), FText::AsNumber(blueScore)));
	}
	else if (GameMode->GetNumberOfPlayers() == 4)
	{
		ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "Red:{0} Green:{1} Blue:{2} Purple:{3}"), FText::AsNumber(redScore), FText::AsNumber(greenScore), FText::AsNumber(blueScore), FText::AsNumber(purpleScore)));
	}
}

void AColourWarsBlockGrid::SetCapitalBlocks()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Setting capital blocks."));

	TArray<IntVector> StartingPositions;
	StartingPositions.Add(IntVector(0, 0));
	StartingPositions.Add(IntVector(Size - 1, Size - 1));
	StartingPositions.Add(IntVector(0, Size - 1));
	StartingPositions.Add(IntVector(Size - 1, 0));

	// Set capital block for each player
	for (int32 PlayerIndex = 1; PlayerIndex < GameMode->GetNumberOfPlayers() + 1; PlayerIndex++)
	{
		//// Get all blocks of this player type
		//TArray<AColourWarsBlock*> PlayerBlocks;
		//for (int32 BlockIndex = 0; BlockIndex < Blocks.Num(); BlockIndex++)
		//{
		//	AColourWarsBlock* block = Blocks[BlockIndex];

		//	// If block is same as player then add it to the player blocks
		//	if ((int32)block->BlockType == PlayerIndex)
		//	{
		//		PlayerBlocks.Add(block);
		//	}
		//}

		// Randomly select a player block to be the capital block
		//int32 RandomIndex = rand() % PlayerBlocks.Num();
		//PlayerBlocks[RandomIndex]->SetCapitalBlock();

		Blocks[ToGridIndex(StartingPositions[PlayerIndex - 1])]->SetBlockType(static_cast<eBlockType>(PlayerIndex));
		Blocks[ToGridIndex(StartingPositions[PlayerIndex - 1])]->SetCapitalBlock();
		Blocks[ToGridIndex(StartingPositions[PlayerIndex - 1])]->SetScore(PlayerIndex);
	}
}

/// <summary>
/// Apply the bonus for all capital blocks of the currert player block type on the grid
/// </summary>
void AColourWarsBlockGrid::ApplyCapitalBlocksBonus()
{
	for (int32 blockIndex = 0; blockIndex < Blocks.Num(); blockIndex++)
	{
		if (Blocks[blockIndex]->IsCapitalBlock() && Blocks[blockIndex]->GetBlockType() == GameMode->GetGameState()->GetCurrentPlayer())
		{
			Blocks[blockIndex]->ApplyCapitalBlockBonus();
		}
	}
}

void AColourWarsBlockGrid::DeselectAllBlocks()
{
	for (AColourWarsBlock* block : Blocks)
	{
		GameMode->GetGameState()->DeselectBlock(block);
	}
}

void AColourWarsBlockGrid::SpawnNewBlock(eBlockType BlockType, IntVector GridCoord, int32 startingScore)
{
	const float HalfSize = ((float)Size - 1.0f) / 2.0f;
	const float XOffset = (GridCoord.X * BlockSpacing) - (HalfSize * BlockSpacing); // Divide by dimension
	const float YOffset = (GridCoord.Y * BlockSpacing) - (HalfSize * BlockSpacing); // Modulo gives remainder

	const FVector WorldLocation = FVector(XOffset, YOffset, 0.f) + GetActorLocation();

	// Spawn a block
	AColourWarsBlock* NewBlock = GetWorld()->SpawnActor<AColourWarsBlock>(WorldLocation, FRotator(0, 0, 0));
	NewBlock->SetGridCoord(GridCoord);
	NewBlock->SetGridLocation(WorldLocation);
	NewBlock->SetActorScale3D(FVector(BlocksScale, BlocksScale, BlocksScale));
	NewBlock->SetScore(startingScore);

	// Tell the block about its owner
	if (NewBlock != nullptr)
	{
		NewBlock->SetOwningGrid(this);
	}

	NewBlock->SetBlockType(BlockType);

	// Add the block to the array of blocks
	Blocks.Add(NewBlock);
}

void AColourWarsBlockGrid::RemoveBlock(AColourWarsBlock* BlockToRemove)
{
	Blocks.Remove(BlockToRemove);
}

/// <summary>
/// Move the StartingBlock onto the EndingBlock
/// </summary>
/// <param name="StartingBlock"></param>
/// <param name="EndingBlock"></param>
/// <returns></returns>
void AColourWarsBlockGrid::MoveBlock(AColourWarsBlock* StartingBlock, AColourWarsBlock* EndingBlock)
{
	//// If blocks are not neighbours then don't move the block
	//if (!AreBlocksNeighbours(StartingBlock, EndingBlock) || !IsValidMove(StartingBlock, EndingBlock))
	//{
	//	return;
	//}

	// If blocks are same type then move score over to ending block
	if (StartingBlock->GetBlockType() == EndingBlock->GetBlockType())
	{
		EndingBlock->AddScore(StartingBlock->GetScore());
		StartingBlock->SetScore(0);
	}
	else
	{
		int32 const attackingCost = StartingBlock->AttackingCost(EndingBlock);
		EndingBlock->SetScore(StartingBlock->GetScore() - attackingCost);
		StartingBlock->SetScore(0);
		EndingBlock->SetBlockType(StartingBlock->GetBlockType());
		if (EndingBlock->IsCapitalBlock())
		{
			EndingBlock->UnsetCapitalBlock();
		}
		EndingBlock->BonusCheck();
	}

	// Add a score to the starting block
	StartingBlock->AddScore(1);

	// Move capital status if this block is capital block
	if (StartingBlock->IsCapitalBlock())
	{
		StartingBlock->UnsetCapitalBlock();
		EndingBlock->SetCapitalBlock();
	}
}

/// <summary>
/// Increase the score of this block based by 1
/// </summary>
void AColourWarsBlockGrid::AddOneToBlock(AColourWarsBlock* block)
{
	// Add 1 score as part of this move
	block->AddScore(1);
}

/// <summary>
/// Combine all of the scores of neighbour blocks into this block
/// </summary>
void AColourWarsBlockGrid::CombineNeighbourBlocks(AColourWarsBlock* block)
{
	block->SetScore(GetSumNeighboursScores(block));

	for (AColourWarsBlock* neighbourBlock : GetNeighbours(block, true))
	{
		// Check if this block type is the same type
		if (neighbourBlock->GetBlockType() == block->GetBlockType() && neighbourBlock->GetScore() > 1)
		{
			neighbourBlock->SetScore(1);
		}
	}
}

/// <summary>
/// Get the sum of all neighbour scores and central block
/// </summary>
/// <returns></returns>
int32 AColourWarsBlockGrid::GetSumNeighboursScores(AColourWarsBlock* centralBlock)
{
	int32 scoreSum = centralBlock->GetScore();;

	for (AColourWarsBlock* neighbourBlock : GetNeighbours(centralBlock, true))
	{
		if (neighbourBlock->GetBlockType() == centralBlock->GetBlockType())
		{
			scoreSum += neighbourBlock->GetScore() - 1;
		}
	}

	return scoreSum;
}

/// <summary>
/// Is the move for this block to take the other block valid?
/// </summary>
/// <param name="StartingBlock"></param>
/// <param name="EndingBlock"></param>
/// <returns></returns>
bool AColourWarsBlockGrid::IsValidMove(AColourWarsBlock* StartingBlock, AColourWarsBlock* EndingBlock)
{
	if (StartingBlock->GetBlockType() == EndingBlock->GetBlockType())
	{
		return true;
	}
	else
	{
		// Check if the 'attacking' block has enough score to 'take' the other block
		if (CanDefeat(StartingBlock, EndingBlock))
		{
			return true;
		}
	}

	return false;
}

bool AColourWarsBlockGrid::HasBlocks(eBlockType BlockType)
{
	for (int32 BlockIndex = 0; BlockIndex < Blocks.Num(); BlockIndex++)
	{
		AColourWarsBlock* block = Blocks[BlockIndex];

		if (block->GetBlockType() == BlockType)
		{
			return true;
		}
	}

	return false;
}

/// <summary>
/// Check if these 2 blocks are neighbours, either vertically or horizontally
/// </summary>
/// <param name="Block1"></param>
/// <param name="Block2"></param>
/// <returns></returns>
bool AColourWarsBlockGrid::AreBlocksNeighbours(AColourWarsBlock* Block1, AColourWarsBlock* Block2)
{
	int32 const Xdiff = abs(Block1->GetGridCoord().X - Block2->GetGridCoord().X);
	int32 const Ydiff = abs(Block1->GetGridCoord().Y - Block2->GetGridCoord().Y);

	return Xdiff == 1 || Ydiff == 1;
}

int32 AColourWarsBlockGrid::GetGameGridSize()
{
	if (GameInstance == nullptr)
	{
		// Set the game instance and number of players in this game
		GameInstance = Cast<UColourWarsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (GameInstance != nullptr)
		{
			Size = GameInstance->GameGridSize;
		}
	}

	return Size;
}

/// <summary>
/// Convert an index value to a grid coordinate
/// </summary>
/// <param name="Index"></param>
/// <returns></returns>
IntVector AColourWarsBlockGrid::ToGridCoord(int Index)
{
	IntVector gridCoord = IntVector(Index % GameInstance->GameGridSize, std::floor((double)(Index / GameInstance->GameGridSize)));

	return gridCoord;
}

/// <summary>
/// Convert an index value to a grid coordinate
/// </summary>
/// <param name="GridCoord"></param>
/// <returns></returns>
int AColourWarsBlockGrid::ToGridIndex(IntVector GridCoord)
{
	return (GridCoord.X * GameInstance->GameGridSize) + GridCoord.Y;
}

/// <summary>
/// Get all neighbour blocks to the central block
/// </summary>
/// <param name="CentralBlock"></param>
/// <returns></returns>
TArray<AColourWarsBlock*> AColourWarsBlockGrid::GetNeighbours(AColourWarsBlock* CentralBlock, bool diagonals)
{
	TArray<AColourWarsBlock*> neighbours;

	IntVector newGridCoord = IntVector(0, 0);

	if (CentralBlock->GetGridCoord().X > 0)
	{
		newGridCoord = CentralBlock->GetGridCoord();
		newGridCoord.X = CentralBlock->GetGridCoord().X - 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}

	if (CentralBlock->GetGridCoord().X < Size - 1)
	{
		newGridCoord = CentralBlock->GetGridCoord();
		newGridCoord.X = CentralBlock->GetGridCoord().X + 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}
	
	if (CentralBlock->GetGridCoord().Y > 0)
	{
		newGridCoord = CentralBlock->GetGridCoord();
		newGridCoord.Y = CentralBlock->GetGridCoord().Y - 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}

	if (CentralBlock->GetGridCoord().Y < Size - 1)
	{
		newGridCoord = CentralBlock->GetGridCoord();
		newGridCoord.Y = CentralBlock->GetGridCoord().Y + 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}

	if (diagonals)
	{
		if (CentralBlock->GetGridCoord().X > 0 && CentralBlock->GetGridCoord().Y > 0)
		{
			newGridCoord = CentralBlock->GetGridCoord();
			newGridCoord.X = CentralBlock->GetGridCoord().X - 1;
			newGridCoord.Y = CentralBlock->GetGridCoord().Y - 1;
			neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
		}

		if (CentralBlock->GetGridCoord().X < Size - 1 && CentralBlock->GetGridCoord().Y < Size - 1)
		{
			newGridCoord = CentralBlock->GetGridCoord();
			newGridCoord.X = CentralBlock->GetGridCoord().X + 1;
			newGridCoord.Y = CentralBlock->GetGridCoord().Y + 1;
			neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
		}

		if (CentralBlock->GetGridCoord().X < Size - 1 && CentralBlock->GetGridCoord().Y > 0)
		{
			newGridCoord = CentralBlock->GetGridCoord();
			newGridCoord.X = CentralBlock->GetGridCoord().X + 1;
			newGridCoord.Y = CentralBlock->GetGridCoord().Y - 1;
			neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
		}

		if (CentralBlock->GetGridCoord().X > 0 && CentralBlock->GetGridCoord().Y < Size - 1)
		{
			newGridCoord = CentralBlock->GetGridCoord();
			newGridCoord.X = CentralBlock->GetGridCoord().X - 1;
			newGridCoord.Y = CentralBlock->GetGridCoord().Y + 1;
			neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
		}
	}

	return neighbours;
}

void AColourWarsBlockGrid::SetSelectableBlocks(eMoveType MoveType, TArray<AColourWarsBlock*> SelectedBlocks)
{
	UnsetAllSelectableBlocks();

	int32 blocksSelected = GameMode->GetGameState()->NumberBlocksSelected();

	// If no blocks selected then just allow all blocks of the player to be selected
	if (blocksSelected == 0)
	{
		for (AColourWarsBlock* block : Blocks)
		{
			if (block->GetBlockType() == GameMode->GetGameState()->GetCurrentPlayer())
			{
				block->SetBlockSelectable(true);
				block->SetBlockScoreText(block->GetScore());
			}
		}
	}
	else if (blocksSelected == 1)
	{
		switch (MoveType)
		{
		case eMoveType::Move:
			for (AColourWarsBlock* neighbourBlock : GetNeighbours(SelectedBlocks[0], false))
			{
				if (neighbourBlock->GetBlockType() == GameMode->GetGameState()->GetCurrentPlayer())
				{
					neighbourBlock->SetBlockSelectable(true);
					neighbourBlock->SetBlockScoreText(neighbourBlock->GetScore() + SelectedBlocks[0]->GetScore());
				}

				if (neighbourBlock->GetBlockType() != GameMode->GetGameState()->GetCurrentPlayer()
					&& SelectedBlocks[0]->AttackingCost(neighbourBlock) < SelectedBlocks[0]->GetScore())
				{
					neighbourBlock->SetBlockSelectable(true);
					neighbourBlock->SetBlockScoreText(SelectedBlocks[0]->GetScore() - SelectedBlocks[0]->AttackingCost(neighbourBlock));
				}
			}
			SelectedBlocks[0]->SetBlockSelectable(true);
			SelectedBlocks[0]->SetBlockScoreText(1);
			break;

		case eMoveType::Combine:
			SelectedBlocks[0]->SetBlockSelectable(true);
			SelectedBlocks[0]->SetBlockScoreText(GetSumNeighboursScores(SelectedBlocks[0]));
			// Set neighbouring blocks to 1
			for (AColourWarsBlock* neighbourBlock : GetNeighbours(SelectedBlocks[0], true))
			{
				if (neighbourBlock->GetBlockType() == SelectedBlocks[0]->GetBlockType())
				{
					neighbourBlock->SetBlockScoreText(1);
				}
			}
			break;

		case eMoveType::AddOne:
			SelectedBlocks[0]->SetBlockSelectable(true);
			SelectedBlocks[0]->SetBlockScoreText(SelectedBlocks[0]->GetScore() + 1);
			break;

		default:
			break;
		}
	}
	else if (blocksSelected == 2)
	{
		switch (MoveType)
		{
			case eMoveType::Move:
				SelectedBlocks[0]->SetBlockSelectable(true);
				SelectedBlocks[0]->SetBlockScoreText(1);
				if (SelectedBlocks[0]->GetBlockType() == SelectedBlocks[1]->GetBlockType())
				{
					SelectedBlocks[1]->SetBlockSelectable(true);
					SelectedBlocks[1]->SetBlockScoreText(SelectedBlocks[0]->GetScore() + SelectedBlocks[1]->GetScore());
				}
				else
				{
					SelectedBlocks[1]->SetBlockSelectable(true);
					SelectedBlocks[1]->SetBlockScoreText(SelectedBlocks[0]->GetScore() - SelectedBlocks[1]->GetScore());
				}
				break;
			
			case eMoveType::Invalid:
			case eMoveType::Combine:
			case eMoveType::AddOne:
				// In these case don't select any more blocks as they don't need it
				break;

			default:
				break;
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Selectable blocks set."));
}

void AColourWarsBlockGrid::UnsetAllSelectableBlocks()
{
	for (AColourWarsBlock* block : Blocks)
	{
		block->SetBlockSelectable(false);
		block->SetBlockScoreText(block->GetScore());
	}
}

void AColourWarsBlockGrid::SetPlayerTurnMeshColour()
{
	PlayerTurnMesh->SetVectorParameterValueOnMaterials("Colour", AColourWarsBlock::BlockColours[GameMode->GetGameState()->GetCurrentPlayer()]);
}

/// <summary>
/// Check if this block can take the defending block
/// </summary>
/// <param name="DefendingBlock"></param>
/// <returns></returns>
bool AColourWarsBlockGrid::CanDefeat(AColourWarsBlock* AttackingBlock, AColourWarsBlock* DefendingBlock)
{
	if (AttackingBlock->GetScore() > AttackingBlock->AttackingCost(DefendingBlock))
	{
		return true;
	}

	return false;
}

/// <summary>
/// Is the move for this block to take the other block valid?
/// </summary>
/// <param name="OtherBlock"></param>
/// <returns></returns>
bool AColourWarsBlockGrid::ValidMove(AColourWarsBlock* Block, AColourWarsBlock* OtherBlock)
{
	if (Block->GetBlockType() == Block->GetBlockType())
	{
		return true;
	}
	else
	{
		// Check if the 'attacking' block has enough score to 'take' the other block
		if (CanDefeat(OtherBlock, Block))
		{
			return true;
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
