// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColourWarsPawn.h"
#include "ColourWarsBlock.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AColourWarsPawn::AColourWarsPawn(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AColourWarsPawn::Tick(float DeltaSeconds)
{
	/*Super::Tick(DeltaSeconds);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{

	}*/
}

void AColourWarsPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", EInputEvent::IE_Pressed, this, &AColourWarsPawn::OnResetVR);
}

void AColourWarsPawn::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);

	OutResult.Rotation = FRotator(-90.0f, -90.0f, 0.0f);
}

void AColourWarsPawn::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}
