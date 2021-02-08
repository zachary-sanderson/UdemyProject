// Fill out your copyright notice in the Description page of Project Settings.


#include "SpecialEffect.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Main.h"
#include "Enemy.h"

// Sets default values
// A class for unique attacks or effects, has a collision volume, particle effect and sound effect attached
ASpecialEffect::ASpecialEffect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionVolume"));
	RootComponent = CollisionVolume;

	SFXTest = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("IdleParticlesComponent"));
	SFXTest->SetupAttachment(GetRootComponent());

	SFXStatusEffect = EStatusEffect::ESE_None;

	bHitTarget = false;

	SFXSoundStartTime = 0.f;
}

// Called when the game starts or when spawned
void ASpecialEffect::BeginPlay()
{
	Super::BeginPlay();

	CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &ASpecialEffect::OnOverlapBegin);
	if (SFXSound)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, SFXSound, GetActorLocation());
		AudioComponent->Play(SFXSoundStartTime);
	}
}

// Called every frame
void ASpecialEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/* Inflict enemy with Status effect based on special type */
void ASpecialEffect::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		AMain* Main = Cast<AMain>(OtherActor);

		switch (SFXStatusEffect)
		{
		case EStatusEffect::ESE_Stunned:
			if (Enemy)
				Enemy->Stunned();
			break;
		case EStatusEffect::ESE_Juggled:
			bHitTarget = true;
			if (Enemy)
				Enemy->Juggled();
			break;
		case EStatusEffect::ESE_Burned:
			if (Enemy)
				Enemy->Burned();
			break;
		case EStatusEffect::ESE_Frozen:
			if (Enemy)
				Enemy->Frozen();
			break;
		case EStatusEffect::ESE_Damage:
			if (Main)
			{
				if (Main->bBlocking)
					Main->TakeStaminaDamage(StaminaDamage);
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("SFX Attempting to Damage Player"))
					UGameplayStatics::ApplyDamage(Main, Damage, Main->GetController(), this, DamageTypeClass);
				}
			}
			break;
		case EStatusEffect::ESE_Stamina:
			
			if (Main)
				Main->TakeStaminaDamage(StaminaDamage);
			break;
		default:
			break;
		}
	}
}

// Fade out audio to sound less jarring
void ASpecialEffect::Die()
{
	if (AudioComponent && SFXStatusEffect != EStatusEffect::ESE_Stunned)
		AudioComponent->FadeOut(0.2f, 0.3f);
	else if (AudioComponent)
		AudioComponent->FadeOut(1.f, 0.3f);

	Destroy();
}