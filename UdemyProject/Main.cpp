// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "UdemySaveGame.h"
#include "ItemStorage.h"
#include "SpecialEffect.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("sword_top"));

	HelmetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(GetMesh(), FName("FX_Head"));

	ShieldParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ShieldParticles"));
	ShieldParticles->SetupAttachment(GetMesh(), FName("FX_Shield_Particles"));

	SwordParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SwordParticles"));
	SwordParticles->SetupAttachment(GetMesh(), FName("FX_Sword_Particles"));

	// Create Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(34.f, 88.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match
	// the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	// Set turn/lookup rate
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Don't Rotate when the controller rotates
	// Let that just effect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 450.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Stats
	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;
	Damage = 10.f;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	MoveForwardSpeed = 0.f;
	MoveRightSpeed = 0.f;

	//Input booleans
	bShiftKeyDown = false;
	bLMBDown = false;
	bRMBDown = false;
	bESCDown = false;
	bSpecialDown = false;

	//Movement
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;
	bMovingForward = false;
	bMovingRight = false;
	bLanding = false;

	//Combat
	InterpSpeed = 15.f;
	bInterpToEnemy = false;
	bHasCombatTarget = false;

	//Damage Invulnerability Variables
	bInvulnerable = false;
	InvulnerabilityDelay = 0.2f;

	//Distance and timing variables for Special Attack
	SpecialStartLocation = FVector(0.f);
	SpecialForwardVector = FVector(0.f);
	SpecialRightVector = FVector(0.f);

	PillarBaseDistanceFromPlayer = 100.f;
	PillarMaxDistanceFromPlayer = 700.f;
	PillarCurrentDistanceFromPlayer = 100.f;
	PillarDelay = 0.1f;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	//Level loading
	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (Map != "SunTemple")
	{
		LoadGameNoSwitch();
		UE_LOG(LogTemp, Warning, TEXT("Loaded"));
		if (MainPlayerController)
		{
			UE_LOG(LogTemp, Warning, TEXT("HAS MPC"));
			MainPlayerController->GameModeOnly();
		}
	}

	//Set Collision preferences
	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AMain::CombatOnOverlapBegin);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	//Double check block animation particles are off
	if (ShieldParticles) ShieldParticles->Deactivate();
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bMovingForward) { MoveForwardSpeed = 0.f; }
	if (!bMovingRight) { MoveRightSpeed = 0.f; }
	if (Stamina < 0.1f) { RMBUp(); }

	float DeltaStamina;
	if (bAttacking || bBlocking)
		DeltaStamina = 0.f;
	else
		DeltaStamina = StaminaDrainRate * DeltaTime;


	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }

	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
				Stamina -= DeltaStamina;
			}
			if (bMovingForward || bMovingRight)
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else
			{
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else // Shift Key up
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
		}
		else
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		else
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	default:
		;
	}

	//When locked on interp player to enemy
	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
		SetActorRotation(InterpRotation);
	}

	//Interp Camera to face target as well
	if (bWasLockedOn && !bAttacking)
	{
		FVector CamLoc; FRotator CamRot;
		Controller->GetPlayerViewPoint(CamLoc, CamRot);
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CamLoc, CombatTarget->GetActorLocation());
		FRotator CameraInterpRotation = FMath::RInterpTo(Controller->GetControlRotation(), LookAtRotation, DeltaTime, InterpSpeed*3);
		FRotator ControlRotation = Controller->GetControlRotation();
		FRotator NewRotation;
		if (CameraInterpRotation.Pitch <= -23.f)
			NewRotation = FRotator(CameraInterpRotation.Pitch, CameraInterpRotation.Yaw, CameraInterpRotation.Roll);
		else
			NewRotation = FRotator(-23.f, CameraInterpRotation.Yaw, CameraInterpRotation.Roll);
		Controller->SetControlRotation(NewRotation);
	}

	//Pass the location the health bar should be rendered at to the Main Player Controller
	if (CombatTarget)
	{
		if (MainPlayerController && CombatTarget->GetMesh())
		{
			MainPlayerController->EnemyLocation = CombatTarget->GetMesh()->GetSocketLocation(FName("HealthBar"));
		}
	}
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("RMB", IE_Pressed, this, &AMain::RMBDown);
	PlayerInputComponent->BindAction("RMB", IE_Released, this, &AMain::RMBUp);

	PlayerInputComponent->BindAction("Special", IE_Pressed, this, &AMain::SpecialDown);
	PlayerInputComponent->BindAction("Special", IE_Released, this, &AMain::SpecialUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AMain::LockOn);

	PlayerInputComponent->BindAction("SwitchWeaponType", IE_Pressed, this, &AMain::SwitchWeaponType);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);
}

//Check booleans to see if a player is Dead or locked in animation and should not be able to move.
bool AMain::CanMove(float Value)
{
	if (MainPlayerController)
	{
		return (Value != 0.0f) && !bAttacking &&
			(MovementStatus != EMovementStatus::EMS_Dead) &&
			MainPlayerController->bPauseMenuVisible == false && bLanding;
	}
	return false;
}

void AMain::Landing()
{ 
	bLanding = true;
}

void AMain::Turn(float Value)
{
	if (!bWasLockedOn)
		AddControllerYawInput(Value);
}

void AMain::LookUp(float Value)
{
	if (!bWasLockedOn)
		AddControllerPitchInput(Value);
}

// Called for forwards movement
void AMain::MoveForward(float Value)
{
	bMovingForward = false;
	if (CanMove(Value))
	{
		MoveForwardSpeed = Value;
		
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}
}
// Called for sideways movement
void AMain::MoveRight(float Value)
{
	bMovingRight = false;
	if (CanMove(Value))
	{
		MoveRightSpeed = Value;
		
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;
	}
}

void AMain::TurnAtRate(float Rate)
{
	if (bInterpToEnemy == false && bWasLockedOn == false)
	{
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AMain::LookUpAtRate(float Rate)
{
	if (bInterpToEnemy == false && bWasLockedOn == false) {
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

//Calls Attack() provided the player can attack
void AMain::LMBDown()
{
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) { return; }

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	else if (!bRMBDown)
	{
		Attack();
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

//Calls Block() provided the player can attack
void AMain::RMBDown()
{
	bRMBDown = true;

	if (bAttacking || MovementStatus == EMovementStatus::EMS_Dead) { return; }

	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	Block();
}

//Cleanly ends blocking status/animation
void AMain::RMBUp()
{
	bRMBDown = false;
	bBlocking = false;
	if (ShieldParticles)
		ShieldParticles->Deactivate();

	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && BlockMontage)
		{
			if (AnimInstance->GetCurrentActiveMontage())
			{
				if (AnimInstance->GetCurrentActiveMontage()->GetFName() == BlockMontage->GetFName())
				{
					AnimInstance->Montage_Stop(0.2f);
				}
			}
		}
	}
}

void AMain::Block()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && BlockMontage)
		{
			AnimInstance->Montage_Play(BlockMontage, 1.35f);
			AnimInstance->Montage_JumpToSection(FName("StartUp"), BlockMontage);
		}
	}
}

void AMain::BlockStart()
{
	bBlocking = true;
	ShieldParticles->Activate();
}

void AMain::SpecialDown()
{
	bSpecialDown = true;

	Special();
}

void AMain::SpecialUp()
{
	bSpecialDown = false;
}

void AMain::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::ESCUp()
{
	bESCDown = false;
}

void AMain::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Amount;
	}
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}

	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMain::Jump()
{
	if (GetMovementComponent()->IsFalling()) return;
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
		bLanding = false;
	}
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}
void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

//Locks on to the nearest enemy
void AMain::LockOn()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (bWasLockedOn == false && !bAttacking)
	{
		FindCombatTarget();

		if (bHasCombatTarget && CombatTarget->Alive())
		{
			bWasLockedOn = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;

			if (!bAttacking) { SetInterpToEnemy(true); }
		}
	}
	else
	{
		RemoveLockOn();
	}
}

void AMain::RemoveLockOn()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	bWasLockedOn = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	SetInterpToEnemy(false);

	if (MainPlayerController)
	{
		MainPlayerController->RemoveEnemyHealthBar();
		MainPlayerController->RemoveEnemyLockOn();
	}
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

//Depreciated
//Leftover from the original project which had different weapons, removed due to change in assets.
void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
	EquippedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	if (/*EquippedWeapon &&*/ !bAttacking && MovementStatus != EMovementStatus::EMS_Dead && Stamina > 15.f)
	{
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			if (SwingSound) UGameplayStatics::PlaySound2D(this, SwingSound);
			AnimInstance->Montage_Play(CombatMontage, 1.2f);
			TakeStaminaDamage(15.f);
		}
	}
}

void AMain::AttackEnd()
{
	if (bLMBDown && Stamina > 15.f)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance->GetCurrentActiveMontage()->GetFName() == CombatMontage->GetFName())
		{
			if (SwingSound) UGameplayStatics::PlaySound2D(this, SwingSound);
			AnimInstance->Montage_Play(CombatMontage1, 1.2f);
			TakeStaminaDamage(15.f);
		}
		else if (AnimInstance->GetCurrentActiveMontage()->GetFName() == CombatMontage1->GetFName())
		{
			if (SwingSound) UGameplayStatics::PlaySound2D(this, SwingSound);
			AnimInstance->Montage_Play(CombatMontage2, 1.2f);
			TakeStaminaDamage(15.f);
		}
	}
	else
	{
		AttackFinish();
	}
}

void AMain::AttackFinish()
{
	bAttacking = false;
}

//When attack collides with enemy spawn particle effects and damage them
void AMain::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (Enemy->HitParticles)
			{
				const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName("FX_Sword_Top");
				if (WeaponSocket)
				{
					FVector SocketLocation = WeaponSocket->GetSocketLocation(GetMesh());
					
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, Enemy->GetControlRotation(), true);
				}
			}
			if (Enemy->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
			}
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Enemy, Damage, GetController(), this, DamageTypeClass);
			}
		}
	}
}

//Special Attack that has a unique form for each element.
void AMain::Special()
{
	if (/*EquippedWeapon &&*/ !bAttacking && MovementStatus != EMovementStatus::EMS_Dead && Stamina >= 50.f)
	{
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && SpecialMontage)
		{
			AnimInstance->Montage_Play(SpecialMontage, 1.f);
			switch (WeaponType)
			{
			case EWeaponType::EWT_Lightning:
				AnimInstance->Montage_JumpToSection(FName("CardCast"), SpecialMontage);
				break;
			case EWeaponType::EWT_Wind:
				AnimInstance->Montage_JumpToSection(FName("Cast"), SpecialMontage);
				break;
			case EWeaponType::EWT_Fire:
				AnimInstance->Montage_JumpToSection(FName("CardCast"), SpecialMontage);
				break;
			case EWeaponType::EWT_Ice:
				AnimInstance->Montage_JumpToSection(FName("Cast"), SpecialMontage);
				break;
			}
		}
	}
}

void AMain::SpecialEnd()
{
	bAttacking = false;
}

void AMain::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMain::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMain::PlaySwingSound()
{
	if (SwordSwingSound) UGameplayStatics::PlaySound2D(this, SwordSwingSound);
}

//Cycles through different weapon types
void AMain::SwitchWeaponType()
{
	if (bAttacking) return;

	switch (WeaponType)
	{
	case EWeaponType::EWT_Lightning:
		WeaponType = EWeaponType::EWT_Wind;
		SwordParticles->SetTemplate(WindParticles);
		break;
	case EWeaponType::EWT_Wind:
		WeaponType = EWeaponType::EWT_Fire;
		SwordParticles->SetTemplate(FireParticles);
		break;
	case EWeaponType::EWT_Fire:
		WeaponType = EWeaponType::EWT_Ice;
		SwordParticles->SetTemplate(IceParticles);
		break;
	case EWeaponType::EWT_Ice:
		WeaponType = EWeaponType::EWT_Lightning;
		SwordParticles->SetTemplate(LightningParticles);
		break;
	}
}

//Calls unqiue special attack based on current weapon type
void AMain::WeaponSpecialAttack()
{
	TakeStaminaDamage(50.f);
	SpecialStartLocation = GetActorLocation();
	SpecialForwardVector = GetActorForwardVector();
	SpecialRightVector = GetActorRightVector();
	switch (WeaponType)
	{
	case EWeaponType::EWT_Lightning:
		LightningSpecial();
		break;
	case EWeaponType::EWT_Wind:
		WindSpecial();
		break;
	case EWeaponType::EWT_Fire:
		WindSpecial();
		break;
	case EWeaponType::EWT_Ice:
		IceSpecial();
		break;
	}
}

void AMain::WindSpecial()
{
	SpawnOurSFX(SpecialStartLocation + (SpecialForwardVector * PillarCurrentDistanceFromPlayer));
}

//Spawn a cone of fire
void AMain::FireSpecial()
{
	FVector FwdRight = (SpecialForwardVector + SpecialRightVector) * 0.5f;
	FVector FwdLeft = (SpecialForwardVector - SpecialRightVector) * 0.5f;
	if (PillarCurrentDistanceFromPlayer <= PillarMaxDistanceFromPlayer)
	{
		SpawnOurSFX(SpecialStartLocation + (SpecialForwardVector * PillarCurrentDistanceFromPlayer));
		SpawnOurSFX(SpecialStartLocation + (FwdLeft * PillarCurrentDistanceFromPlayer));
		SpawnOurSFX(SpecialStartLocation + (FwdRight * PillarCurrentDistanceFromPlayer));
		PillarCurrentDistanceFromPlayer += PillarBaseDistanceFromPlayer;
		GetWorldTimerManager().SetTimer(PillarTimer, this, &AMain::FireSpecial, PillarDelay);
	}
	else { PillarCurrentDistanceFromPlayer = PillarBaseDistanceFromPlayer; }
}

//Call a bolt of lightning that stuns
void AMain::LightningSpecial()
{
	if (CombatTarget)
	{
		SpawnOurSFX(CombatTarget->GetActorLocation());
	}
	else
	{
		SpawnOurSFX(SpecialForwardVector * 500);
	}
}

//Pillars of ice that freeze enemy in place and mid animation
void AMain::IceSpecial()
{
	if (PillarCurrentDistanceFromPlayer <= PillarMaxDistanceFromPlayer)
	{
		SpawnOurSFX(SpecialStartLocation + (SpecialForwardVector * PillarCurrentDistanceFromPlayer));
		PillarCurrentDistanceFromPlayer += PillarBaseDistanceFromPlayer;
		GetWorldTimerManager().SetTimer(PillarTimer, this, &AMain::IceSpecial, PillarDelay);
	}
	else { PillarCurrentDistanceFromPlayer = PillarBaseDistanceFromPlayer; }
}

//Takes in a location and based on the current weapon type spawns the correct Special Effect
void AMain::SpawnOurSFX(const FVector& Location)
{
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FVector Spawnlocation = Location + FVector(0.f, 0.f, -50.f);

	TSubclassOf<AActor> SFXtoSpawn = nullptr;

	if (World)
	{
		switch (WeaponType)
		{
		case EWeaponType::EWT_Lightning:
			if (LightningSFXToSpawn)
			{
				SFXtoSpawn = LightningSFXToSpawn;
			}
			break;
		case EWeaponType::EWT_Wind:
			if (WindSFXToSpawn)
			{
				SFXtoSpawn = WindSFXToSpawn;
			}
			break;
		case EWeaponType::EWT_Fire:
			if (FireSFXToSpawn)
			{
				SFXtoSpawn = FireSFXToSpawn;
			}
			break;
		case EWeaponType::EWT_Ice:
			if (IceSFXToSpawn)
			{
				SFXtoSpawn = IceSFXToSpawn;
			}
			break;
		default:
			SFXtoSpawn = LightningSFXToSpawn;
			break;
		}

		if (SFXtoSpawn) {
			AActor* Actor = World->SpawnActor<AActor>(SFXtoSpawn, Spawnlocation, FRotator(0.f), SpawnParams);
			ASpecialEffect* SpecialEffect = Cast<ASpecialEffect>(Actor);
			if (SpecialEffect)
				SpecialEffect->Parent = this;
		}
	}
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (!bInvulnerable)
	{
		if (Health - DamageAmount <= 0.f)
		{
			Health -= DamageAmount;
			Die();
			if (DamageCauser)
			{
				AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
				if (Enemy)
				{
					Enemy->bHasValidTarget = false;
				}
			}
		}
		else
		{
			Health -= DamageAmount;
			bInvulnerable = true;
			GetWorldTimerManager().SetTimer(InvulnerabilityTimer, this, &AMain::NotInvulnerable, InvulnerabilityDelay);
		}
	}

	return DamageAmount;
}

void AMain::NotInvulnerable()
{
	bInvulnerable = false;
	GetWorldTimerManager().ClearTimer(InvulnerabilityTimer);
}

void AMain::TakeStaminaDamage(float DamageAmount)
{
	if (Stamina - DamageAmount <= 0.f)
	{
		Stamina = 0.0f;
	}
	else
	{
		Stamina -= DamageAmount;
	}
}

//Find nearest enemy, used for LockOn
void AMain::FindCombatTarget()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), FoundActors);

	const FVector Location = GetActorLocation();
	float MinDistance = 200000.f;
	AEnemy* ClosestEnemy = nullptr;

	for (auto Actor : FoundActors)
	{
		AEnemy* Enemy = Cast<AEnemy>(Actor);
		if (Enemy->Alive())
		{
			float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
			{
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
	}

	if (ClosestEnemy && ClosestEnemy->Alive())
	{
		SetCombatTarget(ClosestEnemy);
		SetHasCombatTarget(true);
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
			MainPlayerController->DisplayEnemyLockOn();
		}
	}
	else
	{
		SetCombatTarget(nullptr);
		SetHasCombatTarget(false);
		RemoveLockOn();
	}
}

void AMain::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame()
{
	UUdemySaveGame* SaveGameInstance = Cast<UUdemySaveGame>(UGameplayStatics::CreateSaveGameObject(UUdemySaveGame::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;

	UE_LOG(LogTemp, Warning, TEXT("MapName: %s"), *MapName)

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMain::LoadGame(bool SetPosition)
{
	UUdemySaveGame* LoadGameInstance = Cast<UUdemySaveGame>(UGameplayStatics::CreateSaveGameObject(UUdemySaveGame::StaticClass()));

	LoadGameInstance = Cast<UUdemySaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;

	//Leftover code from the weapon system, keeping as an example
	/*
	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}
	*/

	if (SetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);
		SwitchLevel(LevelName);
	}
}

void AMain::LoadGameNoSwitch()
{
	UUdemySaveGame* LoadGameInstance = Cast<UUdemySaveGame>(UGameplayStatics::CreateSaveGameObject(UUdemySaveGame::StaticClass()));

	LoadGameInstance = Cast<UUdemySaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;

	//Leftover code from the weapon system, keeping as an example
	/*
	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}
	*/

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}