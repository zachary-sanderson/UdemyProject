// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Main.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Main = Cast<AMain>(Pawn);
		}
	}
}

void UMainAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		//Set the MovementSpeed of the player when not locked on

		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		if (Main)
		{
			if (Main->MovementStatus != EMovementStatus::EMS_Sprinting && LateralSpeed.Size() > 187.5f)
				MovementSpeed = 187.5f;
			else
				MovementSpeed = LateralSpeed.Size();
		}
		else
			MovementSpeed = LateralSpeed.Size();


		//Set the movement speed of the player when locked on as well as animation related booleans for locomotion

		if (Main)
		{
			MovementSpeedForward = Main->MoveForwardSpeed;
			MovementSpeedRight = Main->MoveRightSpeed;
			bLockedOn = Main->bWasLockedOn;
			bBlocking = Main->bRMBDown;
			bLanded = Main->bLanding;
		}

		if (bLockedOn) { RootMotionMode = ERootMotionMode::RootMotionFromEverything; }
		else { RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly; }

		bIsInAir = Pawn->GetMovementComponent()->IsFalling();

		if (Main == nullptr)
		{
			Main = Cast<AMain>(Pawn);
		}
	}
}