// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);

	if (WEnemyHealthBar)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment);
	}

	if (WEnemyLockOn)
	{
		EnemyLockOn = CreateWidget<UUserWidget>(this, WEnemyLockOn);
		if (EnemyLockOn)
		{
			EnemyLockOn->AddToViewport();
			EnemyLockOn->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyLockOn->SetAlignmentInViewport(Alignment);
	}

	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	bPauseMenuVisible = false;
}

void AMainPlayerController::DisplayEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}
void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::DisplayEnemyLockOn()
{
	if (EnemyLockOn)
	{
		EnemyLockOn->SetVisibility(ESlateVisibility::Visible);
	}
}
void AMainPlayerController::RemoveEnemyLockOn()
{
	if (EnemyLockOn)
	{
		EnemyLockOn->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyHealthBar)
	{
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
		PositionInViewport.X -= 100.f;
		PositionInViewport.Y -= 25.f;

		FVector2D SizeInViewport(300.f, 25.f);
		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}
	if (EnemyLockOn)
	{
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);

		FVector2D LockOnSize(5.f, 5.f);
		EnemyLockOn->SetPositionInViewport(PositionInViewport);
		EnemyLockOn->SetDesiredSizeInViewport(LockOnSize);
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputModeGameAndUI;

		bShowMouseCursor = true;
	}
}
void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		GameModeOnly();

		bShowMouseCursor = false;

		bPauseMenuVisible = false;
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu();
	}
	else
	{
		DisplayPauseMenu();
	}
}

void AMainPlayerController::GameModeOnly()
{
	FInputModeGameOnly InputModeGameOnly;

	SetInputMode(InputModeGameOnly);
}