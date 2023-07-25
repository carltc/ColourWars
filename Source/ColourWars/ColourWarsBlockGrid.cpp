// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsBlockGrid.h"
#include "ColourWarsBlock.h"
#include "ColourWarsPawn.h"
#include "ColourWarsGameMode.h"
#include "ColourWarsGameInstance.h"
#include "GridCoord.h"
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
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
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
}

void AColourWarsBlockGrid::BeginPlay()
{
	Super::BeginPlay();

	// Set the player pawn
	PlayerPawn = Cast<AColourWarsPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	// Set the gamemode
	GameMode = Cast<AColourWarsGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->GameGrid = this;

	// Set the playerturn mesh to the starting player colour
	PlayerTurnMesh->SetMaterial(0, GameMode->RedMaterial);

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
		//const float XOffset = ((BlockIndex/Size) * BlockSpacing) - (HalfSize * BlockSpacing); // Divide by dimension
		//const float YOffset = ((BlockIndex%Size) * BlockSpacing) - (HalfSize * BlockSpacing); // Modulo gives remainder

		// Get random blocktype
		max = BlockTypes.Num();
		int32 RandomIndex = rand() % max;

		GridCoord newGridCoord;
		newGridCoord.X = Xcoord;
		newGridCoord.Y = Ycoord;

		// Spawn a block
		SpawnNewBlock(BlockTypes[RandomIndex], newGridCoord);

		// Remove block type just used
		BlockTypes.RemoveAt(RandomIndex);
	}

	// Set the capital blocks of each player
	this->SetCapitalBlocks();

	this->UpdateScore();
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
		switch (block->BlockType)
		{
			case eBlockType::Red:
				redScore += block->Score;
				break;
			case eBlockType::Green:
				greenScore += block->Score;
				break;
			case eBlockType::Blue:
				blueScore += block->Score;
				break;
			case eBlockType::Purple:
				purpleScore += block->Score;
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

	// Set capital block for each player
	for (int32 PlayerIndex = 1; PlayerIndex < GameMode->GetNumberOfPlayers() + 1; PlayerIndex++)
	{
		// Get all blocks of this player type
		TArray<AColourWarsBlock*> PlayerBlocks;
		for (int32 BlockIndex = 0; BlockIndex < Blocks.Num(); BlockIndex++)
		{
			AColourWarsBlock* block = Blocks[BlockIndex];

			// If block is same as player then add it to the player blocks
			if ((int32)block->BlockType == PlayerIndex)
			{
				PlayerBlocks.Add(block);
			}
		}

		// Randomly select a player block to be the capital block
		int32 RandomIndex = rand() % PlayerBlocks.Num();
		PlayerBlocks[RandomIndex]->SetCapitalBlock();
	}
}

/// <summary>
/// Apply the bonus for all capital blocks of the currert player block type on the grid
/// </summary>
void AColourWarsBlockGrid::ApplyCapitalBlocksBonus()
{
	for (int32 blockIndex = 0; blockIndex < Blocks.Num(); blockIndex++)
	{
		if (Blocks[blockIndex]->bIsCapitalBlock && Blocks[blockIndex]->BlockType == PlayerPawn->SelectedBlock->BlockType)
		{
			Blocks[blockIndex]->ApplyCapitalBlockBonus();
		}
	}
}

void AColourWarsBlockGrid::DeselectAllOtherBlocks()
{
	PlayerPawn->SelectedBlock = nullptr;

	for (int32 BlockIndex = 0; BlockIndex < Blocks.Num(); BlockIndex++)
	{
		AColourWarsBlock* block = Blocks[BlockIndex];
		block->Deselect();
	}
}

void AColourWarsBlockGrid::SpawnNewBlock(eBlockType BlockType, GridCoord GridCoord)
{
	const float HalfSize = ((float)Size - 1.0f) / 2.0f;
	const float XOffset = (GridCoord.X * BlockSpacing) - (HalfSize * BlockSpacing); // Divide by dimension
	const float YOffset = (GridCoord.Y * BlockSpacing) - (HalfSize * BlockSpacing); // Modulo gives remainder

	const FVector WorldLocation = FVector(XOffset, YOffset, 0.f) + GetActorLocation();

	// Spawn a block
	AColourWarsBlock* NewBlock = GetWorld()->SpawnActor<AColourWarsBlock>(WorldLocation, FRotator(0, 0, 0));
	NewBlock->GridCoord = GridCoord;
	NewBlock->GridLocation = WorldLocation;
	NewBlock->SetActorScale3D(FVector(BlocksScale, BlocksScale, BlocksScale));

	// Tell the block about its owner
	if (NewBlock != nullptr)
	{
		NewBlock->OwningGrid = this;
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
eMoveType AColourWarsBlockGrid::MoveBlock(AColourWarsBlock* StartingBlock, AColourWarsBlock* EndingBlock)
{
	try
	{
		// If blocks are not neighbours then don't move the block
		if (!AreBlocksNeighbours(StartingBlock, EndingBlock) || !IsValidMove(StartingBlock, EndingBlock))
		{
			return eMoveType::Invalid;
		}

		eMoveType moveType = eMoveType::Invalid;

		// If blocks are same type then move score over to ending block
		if (StartingBlock->BlockType == EndingBlock->BlockType)
		{
			EndingBlock->AddScore(StartingBlock->Score);
			StartingBlock->SetScore(0);
			moveType = eMoveType::Defensive;
		}
		else
		{
			int32 const attackingCost = StartingBlock->AttackingCost(EndingBlock);
			EndingBlock->SetScore(StartingBlock->Score - attackingCost);
			StartingBlock->SetScore(0);
			EndingBlock->SetBlockType(StartingBlock->BlockType);
			if (EndingBlock->bIsCapitalBlock)
			{
				EndingBlock->UnsetCapitalBlock();
			}
			moveType = eMoveType::Attacking;
			EndingBlock->BonusCheck();
		}

		// Add a score to the starting block
		StartingBlock->AddScore(1);

		// Move capital status if this block is capital block
		if (StartingBlock->bIsCapitalBlock)
		{
			StartingBlock->UnsetCapitalBlock();
			EndingBlock->SetCapitalBlock();
		}

		return moveType;
	}
	catch (...)
	{
		return eMoveType::Invalid;
	}
}

/// <summary>
/// Is the move for this block to take the other block valid?
/// </summary>
/// <param name="StartingBlock"></param>
/// <param name="EndingBlock"></param>
/// <returns></returns>
bool AColourWarsBlockGrid::IsValidMove(AColourWarsBlock* StartingBlock, AColourWarsBlock* EndingBlock)
{
	if (StartingBlock->BlockType == EndingBlock->BlockType)
	{
		return true;
	}
	else
	{
		// Check if the 'attacking' block has enough score to 'take' the other block
		if (StartingBlock->CanDefeat(EndingBlock))
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

		if (block->BlockType == BlockType)
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
	int32 const Xdiff = abs(Block1->GridCoord.X - Block2->GridCoord.X);
	int32 const Ydiff = abs(Block1->GridCoord.Y - Block2->GridCoord.Y);

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
GridCoord AColourWarsBlockGrid::ToGridCoord(int Index)
{
	GridCoord gridCoord;

	gridCoord.X = Index % GameInstance->GameGridSize;
	gridCoord.Y = std::floor((double)(Index / GameInstance->GameGridSize));

	return gridCoord;
}

/// <summary>
/// Convert an index value to a grid coordinate
/// </summary>
/// <param name="GridCoord"></param>
/// <returns></returns>
int AColourWarsBlockGrid::ToGridIndex(GridCoord GridCoord)
{
	return (GridCoord.X * GameInstance->GameGridSize) + GridCoord.Y;
}

/// <summary>
/// Get all neighbour blocks to the central block
/// </summary>
/// <param name="CentralBlock"></param>
/// <returns></returns>
TArray<AColourWarsBlock*> AColourWarsBlockGrid::GetNeighbours(AColourWarsBlock* CentralBlock)
{
	TArray<AColourWarsBlock*> neighbours;

	GridCoord newGridCoord;
	if (CentralBlock->GridCoord.X > 0)
	{
		newGridCoord = CentralBlock->GridCoord;
		newGridCoord.X = CentralBlock->GridCoord.X - 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}

	if (CentralBlock->GridCoord.X < Size - 1)
	{
		newGridCoord = CentralBlock->GridCoord;
		newGridCoord.X = CentralBlock->GridCoord.X + 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}
	
	if (CentralBlock->GridCoord.Y > 0)
	{
		newGridCoord = CentralBlock->GridCoord;
		newGridCoord.Y = CentralBlock->GridCoord.Y - 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}

	if (CentralBlock->GridCoord.Y < Size - 1)
	{
		newGridCoord = CentralBlock->GridCoord;
		newGridCoord.Y = CentralBlock->GridCoord.Y + 1;
		neighbours.Add(Blocks[ToGridIndex(newGridCoord)]);
	}

	return neighbours;
}

#undef LOCTEXT_NAMESPACE
