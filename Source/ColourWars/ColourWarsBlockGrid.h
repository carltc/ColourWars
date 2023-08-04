// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ColourWarsBlock.h"
#include "IntVector.h"
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

	/** Spacing of blocks */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	float BlockSpacing;

	/** Pointer to player pawn */
	UPROPERTY()
		class AColourWarsPawn* PlayerPawn;

	/** Pointer to game mode */
	UPROPERTY()
		class AColourWarsGameMode* GameMode;

	/** Pointer to game instance */
	UPROPERTY()
		class UColourWarsGameInstance* GameInstance;

	/** StaticMesh component for the clickable block */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* PlayerTurnMesh;

private:
	/** Array of all blocks in grid */
	TArray<AColourWarsBlock*> Blocks;

	/** Number of blocks along each side of grid */
	int32 Size;
	
	/** Block sizes */
	float BlocksScale = 1.f;

protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

public:
	/** Update the player scores */
	void UpdateScore();
	
	/** Set the starting capital blocks for each player */
	void SetCapitalBlocks();
	
	/** Apply the bonus for all capital blocks of the currert player block type on the grid */
	void ApplyCapitalBlocksBonus();

	/** Deselect all blocks */
	void DeselectAllBlocks();
	
	/** Add a new block to the grid */
	void SpawnNewBlock(eBlockType BlockType, IntVector GridCoord, int32 startingScore);

	/** Remove this block from the grid */
	void RemoveBlock(AColourWarsBlock* BlockToRemove);
	
	/** Move the StartingBlock onto the EndingBlock */
	eMoveType MoveBlock(AColourWarsBlock* StartingBlock, AColourWarsBlock* EndingBlock);

	void CombineNeighbourBlocks(AColourWarsBlock* block);

	void AddOneToBlock(AColourWarsBlock* block);

	/** Check if player has blocks left */
	bool HasBlocks(eBlockType BlockType);

	/** Check if these 2 blocks are neighbours, either vertically or horizontally. */
	bool AreBlocksNeighbours(AColourWarsBlock* Block1, AColourWarsBlock* Block2);

	/** Is the move for this block to take the other block valid? */
	bool IsValidMove(AColourWarsBlock* StartingBlock, AColourWarsBlock* EndingBlock);

	/** Convert an index value to a grid coordinate */
	IntVector ToGridCoord(int Index);
	
	/** Convert an index value to a grid coordinate */
	int ToGridIndex(IntVector GridCoord);

	void SetSelectableBlocks(eMoveType MoveType, TArray<AColourWarsBlock*> SelectedBlocks);

	void UnsetAllSelectableBlocks();

	void SetPlayerTurnMeshColour();

	bool CanDefeat(AColourWarsBlock* AttackingBlock, AColourWarsBlock* DefendingBlock);

	bool ValidMove(AColourWarsBlock* Block, AColourWarsBlock* OtherBlock);

	/** Get all neighbour blocks to the central block */
	TArray<AColourWarsBlock*> GetNeighbours(AColourWarsBlock* CentralBlock, bool diagonals);

	UFUNCTION(BlueprintCallable)
		int32 GetGameGridSize();

	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns ScoreText subobject **/
	FORCEINLINE class UTextRenderComponent* GetScoreText() const { return ScoreText; }
};



