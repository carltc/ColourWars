// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"
#include "Components/TextRenderComponent.h"

#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

AColourWarsBlock::AColourWarsBlock()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		ConstructorHelpers::FObjectFinderOptional<UMaterial> BaseMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> BlueMaterial;
		ConstructorHelpers::FObjectFinderOptional<UMaterialInstance> OrangeMaterial;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Puzzle/Meshes/PuzzleCube.PuzzleCube"))
			, BaseMaterial(TEXT("/Game/Puzzle/Meshes/BaseMaterial.BaseMaterial"))
			, BlueMaterial(TEXT("/Game/Puzzle/Meshes/BlueMaterial.BlueMaterial"))
			, OrangeMaterial(TEXT("/Game/Puzzle/Meshes/OrangeMaterial.OrangeMaterial"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Set defaults
	Score = 0;

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
	BlueMaterial = ConstructorStatics.BlueMaterial.Get();
	OrangeMaterial = ConstructorStatics.OrangeMaterial.Get();

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
		bIsSelected = true;

		// Change material
		BlockMesh->SetMaterial(0, OrangeMaterial);
		this->AddActorLocalOffset(FVector(0.f, 0.f, 100.f));

		// Tell the Grid
		/*if (OwningGrid != nullptr)
		{
			OwningGrid->AddScore();
		}*/
	}
	else
	{
		bIsSelected = false;

		// Change material
		BlockMesh->SetMaterial(0, BlueMaterial);
		this->AddActorLocalOffset(FVector(0.f, 0.f, -100.f));
	}
}

void AColourWarsBlock::Highlight(bool bOn)
{
	// Do not highlight if the block has already been activated.
	if (bIsSelected)
	{
		return;
	}

	if (bOn)
	{
		//BlockMesh->SetMaterial(0, BaseMaterial);
	}
	else
	{
		//BlockMesh->SetMaterial(0, BlueMaterial);
	}
}
