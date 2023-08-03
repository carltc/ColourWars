// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class COLOURWARS_API IntVector
{
public:
	IntVector();
	IntVector(int X, int Y);
	~IntVector();

	int32 X;

	int32 Y;
};
