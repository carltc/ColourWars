// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsBlockGrid.h"
#include "ColourWarsBlock.h"
#include "ColourWarsPawn.h"
#include "ColourWarsGameMode.h"
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
	ScoreText->SetRelativeLocation(FVector(300.f, -200.f, -200.f));
	ScoreText->SetRelativeRotation(FRotator(90.f,0.f,0.f));
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "Score: {0}"), FText::AsNumber(0)));
	ScoreText->SetupAttachment(DummyRoot);

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
	const int32 NumBlocks = Size * Size;

	// Set starting block type
	int32 BlockTypeInt = 1;

	// Create list to be used to randomly assign block types
	TArray<AColourWarsBlock::eBlockType> BlockTypes;
	for (int32 BlockIndex = 0; BlockIndex < NumBlocks; BlockIndex++)
	{
		// Get block type
		AColourWarsBlock::eBlockType BlockType = static_cast<AColourWarsBlock::eBlockType>(BlockTypeInt);
		BlockTypes.Add(BlockType);

		// Increment the block type
		BlockTypeInt++;
		if (BlockTypeInt > 3)
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
}


void AColourWarsBlockGrid::AddScore()
{
	// Increment score
	Score++;

	// Update text
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "Score: {0}"), FText::AsNumber(Score)));
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

void AColourWarsBlockGrid::SpawnNewBlock(AColourWarsBlock::eBlockType BlockType, FVector Location)
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


#undef LOCTEXT_NAMESPACE
