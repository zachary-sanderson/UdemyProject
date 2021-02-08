// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UDEMYPROJECT_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable, Category = AnimationProperties)
	void UpdateAnimationProperties();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class APawn* Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class AEnemy* Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
	bool bStunned;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bJuggled;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bBurned;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bFrozen;
};
