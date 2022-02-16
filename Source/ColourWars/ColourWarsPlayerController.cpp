// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsPlayerController.h"

AColourWarsPlayerController::AColourWarsPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}
