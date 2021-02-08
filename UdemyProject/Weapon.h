// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Pickup UMETA(DisplayName = "Pickup"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

/*
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Lightning UMETA(DisplayName = "Lightning"),
	EWT_Rock UMETA(DisplayName = "Rock"),
	EWT_Fire UMETA(DisplayName = "Fire"),
	EWT_Ice UMETA(DisplayName = "Ice"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};
*/

/**
 * 
 */
UCLASS()
class UDEMYPROJECT_API AWeapon : public AItem
{
	GENERATED_BODY()
public:

	/*
	AWeapon();

	UPROPERTY(EditDefaultsOnly, Category ="SavedData")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
		EWeaponState WeaponState;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
		//EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
		bool bWeaponParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
		class UParticleSystem* WeaponSpecialStartParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
		UParticleSystem* WeaponSpecialAttackParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
		class USoundCue* OnEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
		USoundCue* SwingSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
		class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
		class UBoxComponent* CombatCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
		class AMain* MainChar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
		float Damage;

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

protected:

	virtual void BeginPlay() override;

public:

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void Equip(class AMain* Char);

	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; }
	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }

	UFUNCTION()
		virtual void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
		void ActivateCollision();

	UFUNCTION(BlueprintCallable)
		void DeactivateCollision();

	UFUNCTION(BlueprintCallable)
		void WeaponSpecialStart();

	UFUNCTION(BlueprintCallable)
		void WeaponSpecialAttack();

	void RockSpecial();
	void FireSpecial();
	void LightningSpecial();
	void IceSpecial();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<class AActor> SFXToSpawn;

	UFUNCTION()
	void SpawnOurSFX(const FVector& Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	AController* WeaponInstigator;

	FORCEINLINE void SetInstigator(AController* Inst) { WeaponInstigator = Inst; }
	FORCEINLINE void SetMain(AMain* Char) { MainChar = Char; }
	*/
};
