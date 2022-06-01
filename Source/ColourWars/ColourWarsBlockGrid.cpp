// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsBlockGrid.h"
#include "ColourWarsBlock.h"
#include "ColourWarsPawn.h"
#include "ColourWarsGameMode.h"
#include "ColourWarsGameInstance.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
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
	BlockSpacing = 300.f;

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
		const float HalfSize = ((float)Size-1.0f)/2.0f;
		const float XOffset = ((BlockIndex/Size) * BlockSpacing) - (HalfSize * BlockSpacing); // Divide by dimension
		const float YOffset = ((BlockIndex%Size) * BlockSpacing) - (HalfSize * BlockSpacing); // Modulo gives remainder

		// Make position vector, offset from Grid location
		const FVector BlockLocation = FVector(XOffset, YOffset, 0.f) + GetActorLocation();

		// Get random blocktype
		max = BlockTypes.Num();
		int32 RandomIndex = rand() % max;

		// Spawn a block
		SpawnNewBlock(BlockTypes[RandomIndex], BlockLocation);

		// Remove block type just used
		BlockTypes.RemoveAt(RandomIndex);
	}

	this->UpdateScore();
}


void AColourWarsBlockGrid::UpdateScore()
{
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

void AColourWarsBlockGrid::DeselectAllOtherBlocks()
{
	PlayerPawn->SelectedBlock = nullptr;

	for (int32 BlockIndex = 0; BlockIndex < Blocks.Num(); BlockIndex++)
	{
		AColourWarsBlock* block = Blocks[BlockIndex];
		block->Deselect();
	}
}

void AColourWarsBlockGrid::SpawnNewBlock(eBlockType BlockType, FVector Location)
{
	// Spawn a block
	AColourWarsBlock* NewBlock = GetWorld()->SpawnActor<AColourWarsBlock>(Location, FRotator(0, 0, 0));
	NewBlock->GridLocation = Location;
	NewBlock->BlockType = BlockType;

	// Tell the block about its owner
	if (NewBlock != nullptr)
	{
		NewBlock->OwningGrid = this;
	}

	// Set the block material
	NewBlock->SetBlockMaterial();

	// Add the block to the array of blocks
	Blocks.Add(NewBlock);
}

void AColourWarsBlockGrid::RemoveBlock(AColourWarsBlock* BlockToRemove)
{
	Blocks.Remove(BlockToRemove);
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

#undef LOCTEXT_NAMESPACE
