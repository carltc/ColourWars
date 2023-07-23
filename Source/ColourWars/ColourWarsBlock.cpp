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
	bIsCapitalBlock = false;

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
	
	// Add text static mesh component
	CapitalBlockVisual = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CaptialVisual0"));
	CapitalBlockVisual->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	CapitalBlockVisual->SetRelativeRotation(FRotator(90.f, 0.f, 180.f));
	CapitalBlockVisual->SetRelativeScale3D(FVector(10.f, 10.f, 10.f));
	CapitalBlockVisual->VerticalAlignment = EVRTA_TextCenter;
	CapitalBlockVisual->HorizontalAlignment = EHTA_Center;
	CapitalBlockVisual->SetText("O");
	CapitalBlockVisual->SetVisibility(false);
	CapitalBlockVisual->SetupAttachment(DummyRoot);

	// Setup the collision meshes
	NeighbourCheck_CollisionBox = CreateDefaultSubobject<UBoxComponent>(FName("Neighbour Checker Collision Box"));
	NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	NeighbourCheck_CollisionBox->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	NeighbourCheck_CollisionBox->SetupAttachment(DummyRoot);
}

/// <summary>
/// Set the material of this block
/// </summary>
void AColourWarsBlock::SetBlockMaterial()
{
	BlockMesh->SetMaterial(0, GameMode->GetPlayerColour(BlockType));
}

/// <summary>
/// Perform a click when a mouse click on this block is performed.
/// </summary>
/// <param name="ClickedComp"></param>
/// <param name="ButtonClicked"></param>
void AColourWarsBlock::BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
	HandleClicked();
}

/// <summary>
/// Perform click on this block when a touch input from finger is detected.
/// </summary>
/// <param name="FingerIndex"></param>
/// <param name="TouchedComponent"></param>
void AColourWarsBlock::OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	if (!GameMode->GameOver)
	{
		HandleClicked();
	}
}

/// <summary>
/// Act on the block being clicked.
/// 
/// If it has already been selected, then it will deselect.
/// If another block has been selected then check if this block is a neighbour and if so then combine them, otherwise select this block.
/// If no other block has been selected then select this block.
/// </summary>
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
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Making block move."));

					eMoveType MoveType = MakeMove(PlayerPawn->SelectedBlock);

					// If this was an attacking move then a bonus might be available
					if (MoveType == eMoveType::Attacking)
					{
						// Check if a bonus should be awarded and if so do it
						BonusCheck(PlayerPawn->SelectedBlock);
					}

					// Deselect the selected block
					PlayerPawn->SelectedBlock->Deselect();

					// Apply Capital Block bonus to current player
					GameMode->ApplyCapitalBlockBonus();

					// Set gamemode to next player turn
					GameMode->NextTurn();
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Invalid moved attempted."));
				}
				return;
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("This block is not a neighbour of the selected block."));
			}
		}

		// If nothing selected then select this block
		// Check if this block is the correct one for the current player
		if (GameMode->CurrentPlayer == BlockType)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Unselecting all and seleting this block."));
			OwningGrid->DeselectAllOtherBlocks();
			this->Select();
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Deselecting block."));
		this->Deselect();
	}
}

/// <summary>
/// Increase the score of this block based by 1
/// </summary>
void AColourWarsBlock::IncreaseThisBlock()
{
	// Add 1 score as part of this move
	this->AddScore(1);

	// Deselect the selected block
	PlayerPawn->SelectedBlock->Deselect();

	// Set gamemode to next player turn
	GameMode->NextTurn();
}

/// <summary>
/// Combine all of the scores of neighbour blocks into this block
/// </summary>
void AColourWarsBlock::CombineNeighbourBlocks()
{
	bool moveMade = false;

	// Find all neighbouring blocks
	TSet<AActor*> OverlappingActors;
	this->GetOverlappingActors(OverlappingActors);

	for (AActor* actor : OverlappingActors)
	{
		AColourWarsBlock* block = Cast<AColourWarsBlock>(actor);
		if (block != nullptr)
		{
			// Check if this block type is the same type
			if (block->BlockType == this->BlockType && block->Score > 1)
			{
				this->AddScore(block->Score - 1);
				block->SetScore(1);
				moveMade = true;
			}
		}
	}

	if (moveMade)
	{
		// Deselect the selected block
		PlayerPawn->SelectedBlock->Deselect();

		// Set gamemode to next player turn
		GameMode->NextTurn();
	}
}

/// <summary>
/// Check if the other block is a neighbour of this block based on the check direction (check type)
/// </summary>
/// <param name="OtherBlock"></param>
/// <param name="CheckType"></param>
/// <returns></returns>
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

TArray<AColourWarsBlock*> AColourWarsBlock::GetNeighbouringBlocks()
{
	TSet<AActor*> OverlappingActors;
	TSet<AActor*> HorizontalOverlappingActors;
	TArray<AColourWarsBlock*> OverlappingBlocks;

	// Reset the collision box back to the centre and normal size
	NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));

	// Make the collision box large vertically
	NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(2.f, 1.f, 1.f));

	// Get all actors that the selected block overlaps
	GetOverlappingActors(OverlappingActors);
	
	// Make the collision box large horizontally
	NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 2.f, 1.f));

	// Get all actors that the selected block overlaps
	GetOverlappingActors(HorizontalOverlappingActors);
	OverlappingActors.Append(HorizontalOverlappingActors);

	for (AActor* actor : OverlappingActors)
	{
		AColourWarsBlock* block = Cast<AColourWarsBlock>(actor);
		
		OverlappingBlocks.Add(block);
	}

	return OverlappingBlocks;
}

/// <summary>
/// Check if this block should have a bonus applied based on the changed block (ie. if it is a neighbour or not)
/// </summary>
/// <param name="ChangedBlock"></param>
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
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(100.f, 100.f, 0.f));
			break;
			case 1:
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(-100.f, 100.f, 0.f));
			break;
			case 2:
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(-100.f, -100.f, 0.f));
			break;
			case 3:
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(100.f, -100.f, 0.f));
			break;
		}

		// Get all actors that the selected block overlaps
		ChangedBlock->GetOverlappingActors(OverlappingActors);

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

			// And add 1 to changed block too
			ChangedBlock->AddScore(1);
		}
		else
		{
			FString base = "Only found '";
			base.Append(FString::FromInt(blocksFound));
			base.Append("' neighbouring blocks of same type.");
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, base);
		}
	}

	// Reset the collision box back to the centre and normal size
	ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ChangedBlock->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
}

/// <summary>
/// Select this block
/// </summary>
void AColourWarsBlock::Select()
{
	PlayerPawn->SelectedBlock = this;
	bIsSelected = true;

	this->AddActorLocalOffset(FVector(0.f, 0.f, 100.f));
}

/// <summary>
/// Deselect this block
/// </summary>
void AColourWarsBlock::Deselect()
{
	PlayerPawn->SelectedBlock = nullptr;
	bIsSelected = false;

	this->SetActorLocation(GridLocation);
}

/// <summary>
/// Add a score value to this block
/// </summary>
/// <param name="ScoreToAdd"></param>
void AColourWarsBlock::AddScore(int32 ScoreToAdd)
{
	// Add Score
	Score += ScoreToAdd;

	SetScore(Score);
}

/// <summary>
/// Set the score of this block
/// </summary>
/// <param name="ScoreToSet"></param>
void AColourWarsBlock::SetScore(int32 ScoreToSet)
{
	// Add Score
	Score = ScoreToSet;

	// Update text
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "{0}"), FText::AsNumber(Score)));
}

/// <summary>
/// Set this block as a capital block
/// </summary>
void AColourWarsBlock::SetCapitalBlock()
{
	bIsCapitalBlock = true;
	CapitalBlockVisual->SetVisibility(true);
}

/// <summary>
/// Unset this block as a capital block
/// </summary>
void AColourWarsBlock::UnsetCapitalBlock()
{
	bIsCapitalBlock = false;
	CapitalBlockVisual->SetVisibility(false);
}

/// <summary>
/// Set the Capital block bonus based on this block as the capital block
/// </summary>
void AColourWarsBlock::ApplyCapitalBlockBonus()
{
	
}

/// <summary>
/// Is the move for this block to take the other block valid?
/// </summary>
/// <param name="OtherBlock"></param>
/// <returns></returns>
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

/// <summary>
/// Make the move for this block to take the other block
/// </summary>
/// <param name="OtherBlock"></param>
/// <returns></returns>
eMoveType AColourWarsBlock::MakeMove(AColourWarsBlock* OtherBlock)
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
	OwningGrid->SpawnNewBlock(OtherBlock->BlockType, OtherBlock->GridCoord);

	// Place this block in location of other block
	OtherBlock->SetActorLocation(this->GridLocation);
	OtherBlock->GridLocation = this->GridLocation;

	// Move capital status if this block is capital block
	if (this->bIsCapitalBlock)
	{
		OtherBlock->SetCapitalBlock();
	}

	OwningGrid->RemoveBlock(this);
	// Finally destroy this block as it was 'taken'
	this->Destroy();

	return MoveType;
}

/// <summary>
/// Check if this block can take the defending block
/// </summary>
/// <param name="DefendingBlock"></param>
/// <returns></returns>
bool AColourWarsBlock::CanDefeat(AColourWarsBlock* DefendingBlock)
{
	if (this->Score > this->AttackingCost(DefendingBlock))
	{
		return true;
	}

	return false;
}

/// <summary>
/// Get the cost that is required for an attacker to take the defending block
/// </summary>
/// <param name="DefendingBlock"></param>
/// <returns></returns>
int32 AColourWarsBlock::AttackingCost(AColourWarsBlock* DefendingBlock)
{
	if (GameMode->GetNumberOfPlayers() == 2)
	{
		return DefendingBlock->Score;
	}
	else if (GameMode->GetNumberOfPlayers() == 3)
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
	}
	else if (GameMode->GetNumberOfPlayers() == 4)
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
			if (this->BlockType == eBlockType::Purple)
			{
				return DefendingBlock->Score * 2;
			}
			return DefendingBlock->Score;
		case eBlockType::Purple:
			if (this->BlockType == eBlockType::Red)
			{
				return DefendingBlock->Score * 2;
			}
			return DefendingBlock->Score;
		}
	}

	return false;
}
