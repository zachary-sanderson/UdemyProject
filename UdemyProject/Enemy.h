// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UENUM(BlueprintType)
enum class EEnemyMovementStatus : uint8
{
	EMS_Idle UMETA(DisplayName = "Idle"),
	EMS_MoveToTarget UMETA(DisplayName = "MoveToTarget"),
	EMS_Attacking UMETA(DisplayName = "Attacking"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class UDEMYPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EEnemyMovementStatus EnemyMovementStatus;

	FORCEINLINE void SetEnemyMovementStatus(EEnemyMovementStatus Status) { EnemyMovementStatus = Status;  }
	FORCEINLINE EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AggroSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController;

	UFUNCTION(BlueprintCallable)
	void AssignAIController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* SwingSound;

	bool bHasValidTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollisionL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName CombatCollisionLSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UBoxComponent* CombatCollisionR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		FName CombatCollisionRSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CombatMontage;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMinTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMaxTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		AMain* CombatTarget;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void AggroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void CombatLeftOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void CombatRightOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMain* Target);

	UFUNCTION(BlueprintCallable)
	void ActivateCollisionL();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollisionL();

	UFUNCTION(BlueprintCallable)
	void ActivateCollisionR();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollisionR();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void NotAttacking();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void DeathEnd();

	UFUNCTION(BlueprintCallable)
		bool Alive();

	//Just calls Destroy() had to implement to work around a bug
	void Disappear();



	/*	****BOSS STUFF**** */

	UFUNCTION(BlueprintCallable)
	void Subjugation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* SubjugationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<class AActor> SubjugationSFXToSpawn;

	UFUNCTION(BlueprintCallable)
	void SoulSiphon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* SoulSiphonMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<class AActor> SoulSiphonSFXToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	FVector RecallLocation;

	UFUNCTION(BlueprintCallable)
	void Recall();

	bool bRecalling;

	bool bMovingToRecall;

	bool bRecalled;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<class AActor> RecallSpawningBox;

	class ASpawnVolume* RSB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* RecallMontage;

	UFUNCTION(BlueprintCallable)
		void FinishIntro();

	void SpawnSFX(TSubclassOf<AActor> ActorToSpawn, FVector Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		bool bIsInAnimation;

	/*	****BOSS STUFF END***** */
	




	/* *****STATUS EFFECTS****** */


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	class UParticleSystemComponent* StatusParticleComponent;

	/* STUN */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	float StunLength;

	FTimerHandle StunTimer;

	bool bStunned;
	void Stunned();
	void NotStunned();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	UParticleSystem* StunParticles;

	/* JUGGLE */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	float JuggleLength;

	FTimerHandle JuggleTimer;

	bool bJuggled;
	void Juggled();
	void NotJuggled();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	UParticleSystem* JuggleParticles;

	/* BURN */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	float BurnLength;

	FTimerHandle BurnTimer;

	bool bBurned;
	void Burned();
	void NotBurned();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	UParticleSystem* BurnParticles;

	/* BURN DOT */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	float BurnDOTLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	float BurnDOTDamage;

	FTimerHandle BurnDOTTimer;

	void BurnDOT();

	AMain* StatusInflicter;

	FORCEINLINE void SetStatusInflicter(AMain* Main) { StatusInflicter = Main; }

	/* FREEZE */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	float FrozenLength;

	FTimerHandle FreezeTimer;

	bool bFrozen;
	void Frozen();
	void NotFrozen();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Status")
	UParticleSystem* FreezeParticles;

	void Recover();

	UFUNCTION(BlueprintCallable)
		bool NoImpairingStatusEffect();
};
