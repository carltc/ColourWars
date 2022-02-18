// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.h"
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
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> RedMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> GreenMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> OrangeMaterial;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			, BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
			, RedMaterial(TEXT("/Game/Puzzle/Meshes/RedMaterial.RedMaterial"))
			, GreenMaterial(TEXT("/Game/Puzzle/Meshes/GreenMaterial.GreenMaterial"))
			, BlueMaterial(TEXT("/Game/Puzzle/Meshes/BlueMaterial.BlueMaterial"))
			, OrangeMaterial(TEXT("/Game/Puzzle/Meshes/OrangeMaterial.OrangeMaterial"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Set defaults
	Score = 1;

	// Set the player pawn
	PlayerPawn = Cast<AColourWarsPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	// Create static mesh component
	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh0"));
	BlockMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());
	BlockMesh->SetRelativeScale3D(FVector(1.f,1.f,0.25f));
	BlockMesh->SetRelativeLocation(FVector(0.f,0.f,25.f));
	BlockMesh->SetMaterial(0, ConstructorStatics.BlueMaterial.Get());
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

	// Save a pointer to the orange material
	BaseMaterial = ConstructorStatics.BaseMaterial.Get();
	RedMaterial = ConstructorStatics.RedMaterial.Get();
	GreenMaterial = ConstructorStatics.GreenMaterial.Get();
	BlueMaterial = ConstructorStatics.BlueMaterial.Get();
	OrangeMaterial = ConstructorStatics.OrangeMaterial.Get();

	// Setup the collision meshes
	CollisionMeshUp = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh Up"));
	CollisionMeshUp->AddRelativeLocation(FVector(100.f, 0.f, 0.f));
	CollisionMeshUp->SetRelativeScale3D(FVector(3.f, 1.f, 2.f));
	CollisionMeshUp->SetupAttachment(DummyRoot);
	CollisionMeshUp->OnComponentBeginOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapBegin);
	CollisionMeshUp->OnComponentEndOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapEnd);

	CollisionMeshDown = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh Down"));
	CollisionMeshDown->AddRelativeLocation(FVector(-100.f, 0.f, 0.f));
	CollisionMeshDown->SetRelativeScale3D(FVector(3.f, 1.f, 2.f));
	CollisionMeshDown->SetupAttachment(DummyRoot);
	CollisionMeshDown->OnComponentBeginOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapBegin);
	CollisionMeshDown->OnComponentEndOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapEnd);

	CollisionMeshLeft = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh Left"));
	CollisionMeshLeft->AddRelativeLocation(FVector(0.f, 100.f, 0.f));
	CollisionMeshLeft->SetRelativeScale3D(FVector(1.f, 3.f, 2.f));
	CollisionMeshLeft->SetupAttachment(DummyRoot);
	CollisionMeshLeft->OnComponentBeginOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapBegin);
	CollisionMeshLeft->OnComponentEndOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapEnd);

	CollisionMeshRight = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh Right"));
	CollisionMeshRight->AddRelativeLocation(FVector(0.f, -100.f, 0.f));
	CollisionMeshRight->SetRelativeScale3D(FVector(1.f, 3.f, 2.f));
	CollisionMeshRight->SetupAttachment(DummyRoot);
	CollisionMeshRight->OnComponentBeginOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapBegin);
	CollisionMeshRight->OnComponentEndOverlap.AddDynamic(this, &AColourWarsBlock::OnOverlapEnd);
}

void AColourWarsBlock::SetBlockMaterial()
{
	switch (BlockType)
	{
		case eBlockType::Red:
			BlockMesh->SetMaterial(0, RedMaterial);
			break;
		case eBlockType::Green:
			BlockMesh->SetMaterial(0, GreenMaterial);
			break;
		case eBlockType::Blue:
			BlockMesh->SetMaterial(0, BlueMaterial);
			break;
	}
}

void AColourWarsBlock::BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
	HandleClicked();
}


void AColourWarsBlock::OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent)
{
	HandleClicked();
}

void AColourWarsBlock::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AColourWarsBlock* overlapBlock = Cast<AColourWarsBlock>(OtherActor);

	if (overlapBlock != nullptr)
	{
		// Add the block to the array of blocks
		NeighbouringBlocks.Add(overlapBlock);

	}
}

void AColourWarsBlock::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AColourWarsBlock* overlapBlock = Cast<AColourWarsBlock>(OtherActor);

	if (overlapBlock != nullptr)
	{
		// Remove the block to the array of blocks
		NeighbouringBlocks.Remove(overlapBlock);

	}
}

void AColourWarsBlock::HandleClicked()
{
	// Check we are not already active
	if (!bIsSelected)
	{
		if (PlayerPawn->SelectedBlock != nullptr)
		{
			TSet<AActor*> OverlappingActors;
			PlayerPawn->SelectedBlock->GetOverlappingActors(OverlappingActors);

			if (OverlappingActors.Contains(this))
			{
				if (ValidMove(PlayerPawn->SelectedBlock))
				{
					MakeMove(PlayerPawn->SelectedBlock);
				}
				return;
			}
		}

		OwningGrid->DeselectAllOtherBlocks();
		this->Select();
	}
	else
	{
		this->Deselect();
	}
}

void AColourWarsBlock::Highlight(bool bOn)
{
	// Do not highlight if the block has already been activated.
	/*if (bIsSelected)
	{
		return;
	}

	if (bOn)
	{
		BlockMesh->SetMaterial(0, BaseMaterial);
	}
	else
	{
		BlockMesh->SetMaterial(0, BlueMaterial);
	}*/
}

void AColourWarsBlock::Select()
{
	PlayerPawn->SelectedBlock = this;
	bIsSelected = true;

	// Change material
	//BlockMesh->SetMaterial(0, OrangeMaterial);
	this->AddActorLocalOffset(FVector(0.f, 0.f, 100.f));
}

void AColourWarsBlock::Deselect()
{
	PlayerPawn->SelectedBlock = nullptr;
	bIsSelected = false;

	// Change material
	//BlockMesh->SetMaterial(0, BlueMaterial);
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

void AColourWarsBlock::MakeMove(AColourWarsBlock* OtherBlock)
{
	// If blocks are same type then join scores
	if (this->BlockType == OtherBlock->BlockType)
	{
		OtherBlock->AddScore(this->Score);
	}
	else
	{
		OtherBlock->AddScore(-OtherBlock->AttackingCost(this));
	}

	// Spawn a new block into the old location
	OwningGrid->SpawnNewBlock(OtherBlock->BlockType, OtherBlock->GridLocation);

	// Place this block in location of other block
	OtherBlock->SetActorLocation(this->GridLocation);
	OtherBlock->GridLocation = this->GridLocation;

	// Deselect the other block
	OtherBlock->Deselect();

	// Finally destroy this block as it was 'taken'
	this->Destroy();
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
