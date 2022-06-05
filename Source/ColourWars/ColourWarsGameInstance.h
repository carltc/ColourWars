// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ColourWarsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COLOURWARS_API UColourWarsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Game", EditAnywhere, BlueprintReadWrite)
		int32 NumberOfPlayers = 2;

	UPROPERTY(Category = "Game", EditAnywhere, BlueprintReadWrite)
		int32 GameGridSize = 5;

};
