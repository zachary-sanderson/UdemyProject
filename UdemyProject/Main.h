// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Lightning UMETA(DisplayName = "Lightning"),
	EWT_Wind UMETA(DisplayName = "Wind"),
	EWT_Fire UMETA(DisplayName = "Fire"),
	EWT_Ice UMETA(DisplayName = "Ice"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class UDEMYPROJECT_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MinSprintStamina;

	FRotator GetLookAtRotationYaw(FVector Target);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;

	bool bShiftKeyDown;

	//ToggleSprinting
	void ShiftKeyDown();
	void ShiftKeyUp();

	//Toggle Pause
	bool bESCDown;
	void ESCDown();
	void ESCUp();

	/* Set movement status and running speed*/
	void SetMovementStatus(EMovementStatus Status);

	// Camera boom positioning camera behind player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// Follow Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Mesh")
	class UStaticMeshComponent* HelmetMesh;

	// Base turn rate to scale turning function for camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	// Base lookup rate to scale lookup function for camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;


	/**
	/*
	/* Player Stats
	/*
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	void Die();

	virtual void Jump() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called for fwd and bwd
	void MoveForward(float Value);

	// Called for yaw rotation
	void MoveRight(float Value);

	void Turn(float Value);

	// Called for pitch rotation
	void LookUp(float Value);

	bool bMovingForward;
	bool bMovingRight;

	bool CanMove(float Value);

	float MoveForwardSpeed;
	float MoveRightSpeed;

	/** Called via input to turn at a given rate
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** Called via input to lookup at a given rate
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired lookup rate
	*/
	void LookUpAtRate(float Rate);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	bool bLanding;

	UFUNCTION(BlueprintCallable)
		void Landing();



	/*  
		COMBAT BEGIN
	*/

	//Attack
	bool bLMBDown;
	void LMBDown();
	void LMBUp();

	//LockOn
	void LockOn();
	void RemoveLockOn();

	//ToggleBlock
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Block)
		bool bRMBDown;
	void RMBDown();
	void RMBUp();

	UPROPERTY(VisibleAnywhere, BlueprintreadOnly, Category = "Combat")
		bool bWasLockedOn;

	UPROPERTY(VisibleAnywhere, BlueprintreadOnly, Category = "Combat")
		bool bLanded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		bool bHasCombatTarget;

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		FVector CombatTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UParticleSystemComponent* ShieldParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		UParticleSystemComponent* SwordParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		USoundCue* SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		USoundCue* SwordSwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		USoundCue* BlockSound;

	UFUNCTION()
		virtual void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
		void ActivateCollision();

	UFUNCTION(BlueprintCallable)
		void DeactivateCollision();

	UFUNCTION(BlueprintCallable)
		void PlaySwingSound();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
		class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
		class AItem* ActiveOverlappingItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
		class UBoxComponent* CombatCollision;

	void SetEquippedWeapon(AWeapon* WeaponToSet);
	FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
		bool bAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		float Damage;

	bool bInvulnerable;

	void NotInvulnerable();

	FTimerHandle InvulnerabilityTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
		float InvulnerabilityDelay;

	void TakeStaminaDamage(float DamageAmount);

	void Attack();

	UFUNCTION(BlueprintCallable)
		void AttackEnd();

	UFUNCTION(BlueprintCallable)
		void AttackFinish();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
		bool bBlocking;

	void Block();

	UFUNCTION(BlueprintCallable)
		void BlockStart();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		class UAnimMontage* CombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* CombatMontage1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* CombatMontage2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* BlockMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* DeathMontage;

	UFUNCTION(BlueprintCallable)
		void DeathEnd();

	void FindCombatTarget();

	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		EWeaponType WeaponType;

	//Current lock on target
	UPROPERTY(VisibleAnywhere, BlueprintreadOnly, Category = "Combat")
		class AEnemy* CombatTarget;

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	/*  
		COMBAT END   
	*/




	/*
		SPECIAL START
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* SpecialMontage;

	void SwitchWeaponType();

	bool bSpecialDown;
	void SpecialDown();
	void SpecialUp();

	void Special();

	UFUNCTION(BlueprintCallable)
	void SpecialEnd();


	UFUNCTION(BlueprintCallable)
	void WeaponSpecialAttack();

	void WindSpecial();
	void FireSpecial();
	void LightningSpecial();
	void IceSpecial();

	FVector SpecialStartLocation;
	FVector SpecialForwardVector;
	FVector SpecialRightVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
		float PillarBaseDistanceFromPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
		float PillarMaxDistanceFromPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
		float PillarCurrentDistanceFromPlayer;

	FTimerHandle PillarTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
		float PillarDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
		TSubclassOf<class AActor> LightningSFXToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
		TSubclassOf<class AActor> WindSFXToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
		TSubclassOf<class AActor> FireSFXToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
		TSubclassOf<class AActor> IceSFXToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		UParticleSystem* LightningParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		UParticleSystem* WindParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		UParticleSystem* FireParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		UParticleSystem* IceParticles;

	UFUNCTION()
		void SpawnOurSFX(const FVector& Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		TSubclassOf<UDamageType> DamageTypeClass;

	/*
		SPECIAL END
	*/



	/* Saving/Loading/Switching levels */

	void SwitchLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	UFUNCTION(BlueprintCallable)
	void LoadGameNoSwitch();

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
		TSubclassOf<class AItemStorage> WeaponStorage;
};
