// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntVector.h"
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
	Move        UMETA(DisplayName = "Move"),
	Combine     UMETA(DisplayName = "Combine"),
	AddOne      UMETA(DisplayName = "AddOne")
};

/** A block that can be clicked */
UCLASS(minimalapi)
class AColourWarsBlock : public AActor
{
	GENERATED_BODY()

private:
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

	AColourWarsBlock();

	eBlockType BlockType;

	/** Has this block been selected */
	bool bIsSelected;
	
	/** Is it possible to select this block? */
	bool bIsSelectable;

	/** Is this block the capital block of the owning player */
	bool bIsCapitalBlock;
	
	/** Score of this block */
	int32 Score;
	
	/** Grid coordinate of this block */
	IntVector GridCoord;
	
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

	/** Pointer to grey material used on block */
	UPROPERTY()
		class UMaterialInstance* BlockMaterial;
	
	/** Pointer to basic material used on score */
	UPROPERTY()
		class UMaterialInstanceDynamic* TextMaterial;

	/** Handle the block being clicked */
	UFUNCTION()
	void BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);

	/** Handle the block being touched  */
	UFUNCTION()
	void OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);
	
	void HandleClicked();

	/** Array of all blocks in grid */
	TArray<AColourWarsBlock*> NeighbouringBlocks;

	void SetBlockColour();
	
	void SetBlockTextColour(FVector colour);

	void SetBlockTextGreen();

	void SetBlockTextRed();

	void SetBlockTextWhite();

public:

	const static TMap<eBlockType, FVector> BlockColours;

	/** Set this block as selected */
	void SetBlockSelected();

	/** Set this block as deselected */
	void SetBlockDeselected();

	int32 GetScore();

	void AddScore(int32 ScoreToAdd);
	
	void SetScore(int32 ScoreToSet);

	eBlockType GetBlockType();
	
	void SetBlockType(eBlockType newBlockType);

	bool IsCapitalBlock();

	void SetCapitalBlock();
	
	void UnsetCapitalBlock();

	IntVector GetGridCoord();

	void SetGridCoord(IntVector gridCoord);

	FVector GetGridLocation();

	void SetGridLocation(FVector gridLocation);

	AColourWarsBlockGrid* GetOwningGrid();

	void SetOwningGrid(AColourWarsBlockGrid* grid);

	void ApplyCapitalBlockBonus();

	void BonusCheck();

	int32 AttackingCost(AColourWarsBlock* DefendingBlock);

	void SetBlockSelectable(bool Selectable);

	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns BlockMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBlockMesh() const { return BlockMesh; }

	void SetBlockScoreText(int32 score);
};



