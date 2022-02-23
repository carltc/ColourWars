// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.h"
#include "ColourWarsGameMode.h"
#include "ColourWarsPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"
#include "Components/TextRenderComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

AColourWarsBlock::AColourWarsBlock()
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

	// Set defaults
	Score = 1;

	// Set the player pawn
	PlayerPawn = Cast<AColourWarsPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	
	// Set the gamemode
	GameMode = Cast<AColourWarsGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Create static mesh component
	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh0"));
	BlockMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	BlockMesh->SetRelativeScale3D(FVector(1.f,1.f,0.25f));
	BlockMesh->SetRelativeLocation(FVector(0.f,0.f,25.f));
	BlockMesh->SetupAttachment(DummyRoot);
	BlockMesh->OnClicked.AddDynamic(this, &AColourWarsBlock::BlockClicked);
	BlockMesh->OnInputTouchBegin.AddDynamic(this, &AColourWarsBlock::OnFingerPressedBlock);

	// Add text static mesh component
	ScoreText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreText0"));
	ScoreText->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	ScoreText->SetRelativeRotation(FRotator(90.f, 0.f, 180.f));
	ScoreText->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
	ScoreText->VerticalAlignment = EVRTA_TextCenter;
	ScoreText->HorizontalAlignment = EHTA_Center;
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "{0}"), FText::AsNumber(Score)));
	ScoreText->SetupAttachment(DummyRoot);

	// Setup the collision meshes
	NeighbourCheck_CollisionBox = CreateDefaultSubobject<UBoxComponent>(FName("Neighbour Checker Collision Box"));
	NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	NeighbourCheck_CollisionBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	NeighbourCheck_CollisionBox->SetupAttachment(DummyRoot);
}

void AColourWarsBlock::SetBlockMaterial()
{
	BlockMesh->SetMaterial(0, GameMode->GetPlayerColour(BlockType));
}

void AColourWarsBlock::BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
	HandleClicked();
}


void AColourWarsBlock::OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	HandleClicked();
}

void AColourWarsBlock::HandleClicked()
{
	// Check we are not already active
	if (!bIsSelected)
	{
		// Check that a block has been selected
		if (PlayerPawn->SelectedBlock != nullptr)
		{
			// Check that this block, that was clicked on, is overlapped by the collision box of the selected block
			if (NeighbourCheck(PlayerPawn->SelectedBlock, eNeighbourCheckType::Horizontal)
				|| NeighbourCheck(PlayerPawn->SelectedBlock, eNeighbourCheckType::Vertical))
			{
				if (ValidMove(PlayerPawn->SelectedBlock))
				{
					eMoveType MoveType = MakeMove(PlayerPawn->SelectedBlock);

					// If this was an attacking move then a bonus might be available
					if (MoveType == eMoveType::Attacking)
					{
						// Check if a bonus should be awarded and if so do it
						BonusCheck(PlayerPawn->SelectedBlock);
					}

					// Deselect the selected block
					PlayerPawn->SelectedBlock->Deselect();

					// Set gamemode to next player turn
					GameMode->NextTurn();
				}
				return;
			}
		}

		// If nothing selected then select this block
		// Check if this block is the correct one for the current player
		if (GameMode->CurrentPlayer == BlockType)
		{
			OwningGrid->DeselectAllOtherBlocks();
			this->Select();
		}
	}
	else
	{
		this->Deselect();
	}
}

bool AColourWarsBlock::NeighbourCheck(AColourWarsBlock* OtherBlock, eNeighbourCheckType CheckType)
{
	TSet<AActor*> OverlappingActors;

	// Reset the collision box back to the centre and normal size
	OtherBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	OtherBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));

	switch (CheckType)
	{
		case eNeighbourCheckType::Vertical:
			// Make the collision box large vertically
			OtherBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(2.f, 1.f, 1.f));
			break;
		case eNeighbourCheckType::Horizontal:
			// Make the collision box large horizontally
			OtherBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 2.f, 1.f));
			break;
	}

	// Get all actors that the selected block overlaps
	OtherBlock->GetOverlappingActors(OverlappingActors);

	// Reset the collision box back to the centre and normal size
	OtherBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	OtherBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));

	// Check that this block, that was clicked on, is overlapped by the collision box of the selected block
	if (OverlappingActors.Contains(this))
	{
		return true;
	}

	return false;
}

void AColourWarsBlock::BonusCheck(AColourWarsBlock* ChangedBlock)
{
	TSet<AActor*> OverlappingActors;

	// Reset the collision box back to the centre and normal size
	ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));

	for (int i = 0; i < 4; i++)
	{
		// Set the collision box to 1 of the 4 directions
		switch (i)
		{
			case 0:
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(2.f, 2.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(150.f, 150.f, 0.f));
			break;
			case 1:
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(2.f, 2.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(-150.f, 150.f, 0.f));
			break;
			case 2:
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(2.f, 2.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(-150.f, -150.f, 0.f));
			break;
			case 3:
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(2.f, 2.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(150.f, -150.f, 0.f));
			break;
		}

		// Get all actors that the selected block overlaps
		ChangedBlock->GetOverlappingActors(OverlappingActors);

		// Check if there are exactly 4 actors overlapped
		int32 blocksFound = 0;
		bool AllBlocksSame = true;
		for (AActor * actor : OverlappingActors)
		{
			AColourWarsBlock* block = Cast<AColourWarsBlock>(actor);
			if (block != nullptr)
			{
				blocksFound++;

				// Check if this block type is the same as others
				if (block->BlockType != ChangedBlock->BlockType)
				{
					AllBlocksSame = false;
				}
			}
		}

		// Check if exactly 3 blocks were found and all were the same
		if (AllBlocksSame && blocksFound == 3)
		{
			// Add 1 to each block
			for (AActor* actor : OverlappingActors)
			{
				AColourWarsBlock* block = Cast<AColourWarsBlock>(actor);
				if (block != nullptr)
				{
					block->AddScore(1);
				}
			}
		}
	}

	// Reset the collision box back to the centre and normal size
	ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
}

void AColourWarsBlock::Select()
{
	PlayerPawn->SelectedBlock = this;
	bIsSelected = true;

	this->AddActorLocalOffset(FVector(0.f, 0.f, 100.f));
}

void AColourWarsBlock::Deselect()
{
	PlayerPawn->SelectedBlock = nullptr;
	bIsSelected = false;

	this->SetActorLocation(GridLocation);
}

void AColourWarsBlock::AddScore(int32 ScoreToAdd)
{
	// Add Score
	Score += ScoreToAdd;

	// Update text
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "{0}"), FText::AsNumber(Score)));
}

bool AColourWarsBlock::ValidMove(AColourWarsBlock* OtherBlock)
{
	if (this->BlockType == OtherBlock->BlockType)
	{
		return true;
	}
	else
	{
		// Check if the 'attacking' block has enough score to 'take' the other block
		if (OtherBlock->CanDefeat(this))
		{
			return true;
		}
	}

	return false;
}

AColourWarsBlock::eMoveType AColourWarsBlock::MakeMove(AColourWarsBlock* OtherBlock)
{
	eMoveType MoveType = eMoveType::Defensive;

	// If blocks are same type then join scores
	if (this->BlockType == OtherBlock->BlockType)
	{
		OtherBlock->AddScore(this->Score);
		MoveType = eMoveType::Defensive;
	}
	else
	{
		OtherBlock->AddScore(-OtherBlock->AttackingCost(this));
		MoveType = eMoveType::Attacking;
	}

	// Spawn a new block into the old location
	OwningGrid->SpawnNewBlock(OtherBlock->BlockType, OtherBlock->GridLocation);

	// Place this block in location of other block
	OtherBlock->SetActorLocation(this->GridLocation);
	OtherBlock->GridLocation = this->GridLocation;

	// Finally destroy this block as it was 'taken'
	this->Destroy();

	return MoveType;
}

bool AColourWarsBlock::CanDefeat(AColourWarsBlock* DefendingBlock)
{
	if (this->Score > this->AttackingCost(DefendingBlock))
	{
		return true;
	}

	return false;
}

int32 AColourWarsBlock::AttackingCost(AColourWarsBlock* DefendingBlock)
{
	switch (DefendingBlock->BlockType)
	{
		case eBlockType::Red:
			if (this->BlockType == eBlockType::Green)
			{
				return DefendingBlock->Score * 2;
			}
			return DefendingBlock->Score;
		case eBlockType::Green:
			if (this->BlockType == eBlockType::Blue)
			{
				return DefendingBlock->Score * 2;
			}
			return DefendingBlock->Score;
		case eBlockType::Blue:
			if (this->BlockType == eBlockType::Red)
			{
				return DefendingBlock->Score * 2;
			}
			return DefendingBlock->Score;
	}

	return false;
}
