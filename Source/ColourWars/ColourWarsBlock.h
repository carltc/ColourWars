// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCoord.h"
#include "ColourWarsBlock.generated.h"

// Neighbour Check Type
UENUM(BlueprintType)
enum class eNeighbourCheckType : uint8
{
	Vertical   UMETA(DisplayName = "Vertical"),
	Horizontal UMETA(DisplayName = "Horizontal")
};

// Block type
UENUM(BlueprintType)
enum class eBlockType : uint8
{
	None     UMETA(DisplayName = "None"),
	Red      UMETA(DisplayName = "Red"),
	Green    UMETA(DisplayName = "Green"),
	Blue     UMETA(DisplayName = "Blue"),
	Purple   UMETA(DisplayName = "Purple")
};

// Move type
UENUM(BlueprintType)
enum class eMoveType : uint8
{
	Invalid     UMETA(DisplayName = "Invalid"),
	Attacking   UMETA(DisplayName = "Attacking"),
	Defensive   UMETA(DisplayName = "Defensive")
};

/** A block that can be clicked */
UCLASS(minimalapi)
class AColourWarsBlock : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	/** StaticMesh component for the clickable block */
	UPROPERTY(Category = Block, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BlockMesh;

	/** Text component for the score */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UTextRenderComponent* ScoreText;
	
	/** Capital block visual component */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UTextRenderComponent* CapitalBlockVisual;

	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* NeighbourCheck_CollisionBox;

public:
	AColourWarsBlock();

	eBlockType BlockType;

	/** Has this block been selected */
	bool bIsSelected;
	
	/** Is this block the capital block of the owning player */
	bool bIsCapitalBlock;
	
	/** Score of this block */
	int32 Score;
	
	/** Grid coordinate of this block */
	GridCoord GridCoord;
	
	/** World location of block in grid */
	FVector GridLocation;

	/** Pointer to player pawn */
	UPROPERTY()
	class AColourWarsPawn* PlayerPawn;
	
	/** Pointer to game mode */
	UPROPERTY()
	class AColourWarsGameMode* GameMode;
	
	/** Grid that owns us */
	UPROPERTY()
	class AColourWarsBlockGrid* OwningGrid;

	/** Handle the block being clicked */
	UFUNCTION()
	void BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);

	/** Handle the block being touched  */
	UFUNCTION()
	void OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);
	
	UFUNCTION(BlueprintCallable)
		void IncreaseThisBlock();
	
	UFUNCTION(BlueprintCallable)
		void CombineNeighbourBlocks();

	void SetBlockType(eBlockType newBlockType);
	
	void HandleClicked();

	bool NeighbourCheck(AColourWarsBlock* OtherBlock, eNeighbourCheckType CheckType);
	
	void BonusCheck();

	void Select();

	void Deselect();

	void AddScore(int32 ScoreToAdd);
	
	void SetScore(int32 ScoreToSet);
	
	void SetCapitalBlock();
	
	void UnsetCapitalBlock();

	void ApplyCapitalBlockBonus();

	bool ValidMove(AColourWarsBlock* OtherBlock);
	
	eMoveType MakeMove(AColourWarsBlock* OtherBlock);
	
	bool CanDefeat(AColourWarsBlock* DefendingBlock);
	
	int32 AttackingCost(AColourWarsBlock* DefendingBlock);

	/** Get Neighbouring blocks */
	TArray<AColourWarsBlock*> GetNeighbouringBlocks();

private:
	/** Array of all blocks in grid */
	TArray<AColourWarsBlock*> NeighbouringBlocks;

	void SetBlockMaterial();

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns BlockMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBlockMesh() const { return BlockMesh; }

};



