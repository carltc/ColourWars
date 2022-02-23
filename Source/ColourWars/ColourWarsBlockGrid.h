// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ColourWarsBlock.h"
#include "ColourWarsBlockGrid.generated.h"

/** Class used to spawn blocks and manage score */
UCLASS(minimalapi)
class AColourWarsBlockGrid : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	/** Text component for the score */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UTextRenderComponent* ScoreText;


public:
	AColourWarsBlockGrid();

	/** How many blocks have been clicked */
	int32 Score;

	/** Number of blocks along each side of grid */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	int32 Size;

	/** Spacing of blocks */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	float BlockSpacing;

	/** Pointer to player pawn */
	UPROPERTY()
		class AColourWarsPawn* PlayerPawn;

	/** Pointer to game mode */
	UPROPERTY()
		class AColourWarsGameMode* GameMode;

	/** StaticMesh component for the clickable block */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* PlayerTurnMesh;

private:
	/** Array of all blocks in grid */
	TArray<AColourWarsBlock*> Blocks;


protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

public:

	/** Handle the block being clicked */
	void AddScore();

	/** Handle the block being clicked */
	void DeselectAllOtherBlocks();
	
	/** Handle the block being clicked */
	void SpawnNewBlock(AColourWarsBlock::eBlockType BlockType, FVector Location);

	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns ScoreText subobject **/
	FORCEINLINE class UTextRenderComponent* GetScoreText() const { return ScoreText; }
};



