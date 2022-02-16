// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsGameMode.h"
#include "ColourWarsPlayerController.h"
#include "ColourWarsPawn.h"

AColourWarsGameMode::AColourWarsGameMode()
{
	// no pawn by default
	DefaultPawnClass = AColourWarsPawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = AColourWarsPlayerController::StaticClass();
}
