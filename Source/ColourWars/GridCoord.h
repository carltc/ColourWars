// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class COLOURWARS_API GridCoord
{
public:
	GridCoord();
	GridCoord(int X, int Y);
	~GridCoord();

	int32 X;

	int32 Y;
};
