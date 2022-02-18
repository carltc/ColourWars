// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ColourWarsBlock.generated.h"

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

	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CollisionMeshUp;

	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CollisionMeshDown;

	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CollisionMeshLeft;

	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CollisionMeshRight;

public:
	AColourWarsBlock();

	// Block type
	enum eBlockType
	{
		Red,
		Green,
		Blue
	};

	eBlockType BlockType;

	/** Has this block been selected */
	bool bIsSelected;
	
	/** Score of this block */
	int32 Score;
	
	/** Grid location of this block */
	FVector GridLocation;

	/** Pointer to player pawn */
	UPROPERTY()
	class AColourWarsPawn* PlayerPawn;
	
	/** Pointer to white material used on the focused block */
	UPROPERTY()
	class UMaterial* BaseMaterial;

	/** Pointer to red material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* RedMaterial;
	
	/** Pointer to green material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* GreenMaterial;
	
	/** Pointer to blue material used on inactive blocks */
	UPROPERTY()
	class UMaterialInstance* BlueMaterial;

	/** Pointer to orange material used on active blocks */
	UPROPERTY()
	class UMaterialInstance* OrangeMaterial;

	/** Grid that owns us */
	UPROPERTY()
	class AColourWarsBlockGrid* OwningGrid;

	/** Handle the block being clicked */
	UFUNCTION()
	void BlockClicked(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);

	/** Handle the block being touched  */
	UFUNCTION()
	void OnFingerPressedBlock(ETouchIndex::Type FingerIndex, UPrimitiveComponent* TouchedComponent);
	
	// declare overlap begin function
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// declare overlap end function
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	void SetBlockMaterial();
	
	void HandleClicked();

	void Highlight(bool bOn);

	void Select();

	void Deselect();

	void AddScore(int32 ScoreToAdd);
	
	bool ValidMove(AColourWarsBlock* OtherBlock);
	
	void MakeMove(AColourWarsBlock* OtherBlock);
	
	bool CanDefeat(AColourWarsBlock* DefendingBlock);
	
	int32 AttackingCost(AColourWarsBlock* DefendingBlock);

private:
	/** Array of all blocks in grid */
	TArray<AColourWarsBlock*> NeighbouringBlocks;

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
	/** Returns BlockMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBlockMesh() const { return BlockMesh; }
};



