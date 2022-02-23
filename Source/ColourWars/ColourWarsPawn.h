// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ColourWarsPawn.generated.h"

UCLASS(config=Game)
class AColourWarsPawn : public APawn
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		class AColourWarsBlock* SelectedBlock;

protected:
	void OnResetVR();

};
