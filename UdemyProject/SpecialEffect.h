// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpecialEffect.generated.h"

UENUM(BlueprintType)
enum class EStatusEffect : uint8
{
	ESE_Stunned UMETA(DisplayName = "Stunned"),
	ESE_Juggled UMETA(DisplayName = "Juggled"),
	ESE_Burned UMETA(DisplayName = "Burned"),
	ESE_Frozen UMETA(DisplayName = "Frozen"),
	ESE_Damage UMETA(DisplayName = "Damage"),
	ESE_Stamina UMETA(DisplayName = "Stamina"),

	ESE_None UMETA(DisplayName = "None"),

	ESE_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class UDEMYPROJECT_API ASpecialEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpecialEffect();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SFX | Collision")
	class UBoxComponent* CollisionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX | Particles")
	class UParticleSystemComponent* SFXTest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX | Particles")
	class UParticleSystem* SFXParticlesToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX | Sounds")
	class USoundCue* SFXSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SFX | Sounds")
	class UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	AActor* Parent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float StaminaDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX | Combat")
	EStatusEffect SFXStatusEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX | Sounds")
	float SFXSoundStartTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SFX | Collision")
	bool bHitTarget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void Die();
};
