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

const TMap<eBlockType, FVector> AColourWarsBlock::BlockColours
{
	{eBlockType::None,   FVector( 1.0f  , 0.9f , 0.2f )},
	{eBlockType::Red,    FVector( 1.0f  , 0    , 0    )},
	{eBlockType::Green,  FVector( 0     , 1.0f , 0    )},
	{eBlockType::Blue,   FVector( 0     , 0    , 1.0f )},
	{eBlockType::Purple, FVector( 0.25f , 0    , 1.0f )}
};

AColourWarsBlock::AColourWarsBlock()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlockMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> TextMaterial;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			, BlockMaterial(TEXT("/Game/Puzzle/Meshes/BlockMaterial_Inst.BlockMaterial_Inst"))
			, TextMaterial(TEXT("/Game/Puzzle/Meshes/TextMaterial_Inst.TextMaterial_Inst"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Set defaults
	Score = 0;
	bIsCapitalBlock = false;
	BlockType = eBlockType::None;
	BlockMaterial = ConstructorStatics.BlockMaterial.Get();
	TextMaterial = UMaterialInstanceDynamic::Create(ConstructorStatics.TextMaterial.Get(), NULL);

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
	BlockMesh->SetMaterial(0, BlockMaterial);

	// Add text static mesh component
	ScoreText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreText0"));
	ScoreText->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	ScoreText->SetRelativeRotation(FRotator(90.f, 0.f, 180.f));
	ScoreText->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
	ScoreText->VerticalAlignment = EVRTA_TextCenter;
	ScoreText->HorizontalAlignment = EHTA_Center;
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "{0}"), FText::AsNumber(Score)));
	ScoreText->SetupAttachment(DummyRoot);
	ScoreText->SetMaterial(0, TextMaterial);
	
	// Add capital visual
	CapitalBlockVisual = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CaptialVisual0"));
	CapitalBlockVisual->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	CapitalBlockVisual->SetRelativeRotation(FRotator(90.f, 0.f, 180.f));
	CapitalBlockVisual->SetRelativeScale3D(FVector(1.f, 20.f, 15.f));
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
/// Get the BlockType of this block
/// </summary>
/// <returns></returns>
eBlockType AColourWarsBlock::GetBlockType()
{
	return BlockType;
}

/// <summary>
/// Set the type of this Block and update it to reflect the change
/// </summary>
/// <param name="BlockType"></param>
void AColourWarsBlock::SetBlockType(eBlockType newBlockType)
{
	BlockType = newBlockType;

	SetBlockColour();
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
	if (!GameMode->GetGameState()->GetGameOver())
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
	// Check if this block can be selected
	if (!bIsSelectable)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("This block cannot be selected."));
		return;
	}

	GameMode->GetGameState()->ToggleBlockSelection(this);
}

/// <summary>
/// Check if this block has created a square of same blocks and if so apply a completion bonus
/// </summary>
void AColourWarsBlock::BonusCheck()
{
	TSet<AActor*> OverlappingActors;

	// Reset the collision box back to the centre and normal size
	this->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	this->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));

	for (int i = 0; i < 4; i++)
	{
		// Set the collision box to 1 of the 4 directions
		switch (i)
		{
			case 0:
				this->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				this->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(100.f, 100.f, 0.f));
			break;
			case 1:
				this->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				this->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(-100.f, 100.f, 0.f));
			break;
			case 2:
				this->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				this->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(-100.f, -100.f, 0.f));
			break;
			case 3:
				this->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
				this->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(100.f, -100.f, 0.f));
			break;
		}

		// Get all actors that the selected block overlaps
		this->GetOverlappingActors(OverlappingActors);

		int32 blocksFound = 0;
		bool AllBlocksSame = true;
		for (AActor * actor : OverlappingActors)
		{
			AColourWarsBlock* block = Cast<AColourWarsBlock>(actor);
			if (block != nullptr)
			{
				blocksFound++;

				// Check if this block type is the same as others
				if (block->BlockType != this->BlockType)
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
			this->AddScore(1);
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
	this->NeighbourCheck_CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	this->NeighbourCheck_CollisionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
}

/// <summary>
/// Set this block as selected
/// </summary>
void AColourWarsBlock::SetBlockSelected()
{
	bIsSelected = true;
	this->AddActorLocalOffset(FVector(0.f, 0.f, 100.f));
}

void AColourWarsBlock::SetBlockDeselected()
{
	bIsSelected = false;
	this->SetActorLocation(GridLocation);
}

/// <summary>
/// Get the Score of this block
/// </summary>
/// <returns></returns>
int32 AColourWarsBlock::GetScore()
{
	return Score;
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
/// Is this block a Capital Block
/// </summary>
/// <returns></returns>
bool AColourWarsBlock::IsCapitalBlock()
{
	return bIsCapitalBlock;
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

IntVector AColourWarsBlock::GetGridCoord()
{
	return GridCoord;
}

void AColourWarsBlock::SetGridCoord(IntVector gridCoord)
{
	GridCoord = gridCoord;
}

FVector AColourWarsBlock::GetGridLocation()
{
	return GridLocation;
}

void AColourWarsBlock::SetGridLocation(FVector gridLocation)
{
	GridLocation = gridLocation;
}

AColourWarsBlockGrid* AColourWarsBlock::GetOwningGrid()
{
	return OwningGrid;
}

void AColourWarsBlock::SetOwningGrid(AColourWarsBlockGrid* grid)
{
	OwningGrid = grid;
}

/// <summary>
/// Set the Capital block bonus based on this block as the capital block
/// </summary>
void AColourWarsBlock::ApplyCapitalBlockBonus()
{
	TArray<AColourWarsBlock*> neighbours = OwningGrid->GetNeighbours(this, false);

	for (int32 blockIndex = 0; blockIndex < neighbours.Num(); blockIndex++)
	{
		if (neighbours[blockIndex]->BlockType == this->BlockType)
		{
			neighbours[blockIndex]->AddScore(1);
		}
	}
}

/// <summary>
/// Get the cost that is required for this block to take the defending block
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

void AColourWarsBlock::SetBlockSelectable(bool Selectable)
{
	bIsSelectable = Selectable;

	if (bIsSelectable)
	{
		BlockMesh->SetScalarParameterValueOnMaterials("GreyingOut", 0);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Block set as Selectable."));
	}
	else
	{
		BlockMesh->SetScalarParameterValueOnMaterials("GreyingOut", 0.5);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Block set as Unselectable."));
	}
}

void AColourWarsBlock::SetBlockColour()
{
	BlockMesh->SetVectorParameterValueOnMaterials("Colour", BlockColours[BlockType]);
}

void AColourWarsBlock::SetBlockTextColour(FVector colour)
{
	TextMaterial->SetVectorParameterValue("Colour", colour);
}

void AColourWarsBlock::SetBlockTextGreen()
{
	SetBlockTextColour(FVector(0, 0.5, 0));
}

void AColourWarsBlock::SetBlockTextRed()
{
	SetBlockTextColour(FVector(0.5, 0, 0));
}

void AColourWarsBlock::SetBlockTextWhite()
{
	SetBlockTextColour(FVector(1, 1, 1));
}

void AColourWarsBlock::SetBlockScoreText(int32 score)
{
	ScoreText->SetText(FText::Format(LOCTEXT("ScoreFmt", "{0}"), FText::AsNumber(score)));

	if (score > Score)
	{
		SetBlockTextGreen();
	}
	else if (score < Score)
	{
		SetBlockTextRed();
	}
	else
	{
		SetBlockTextWhite();
	}

}


