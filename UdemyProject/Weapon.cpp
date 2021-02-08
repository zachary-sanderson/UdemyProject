// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/World.h"
#include "SpecialEffect.h"

// Depriciated class from when the game used a multi weapon system rather than one weapon that can change special effect

/*
AWeapon::AWeapon()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());
	

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());

	bWeaponParticles = true;

	WeaponState = EWeaponState::EWS_Pickup;
	WeaponType = EWeaponType::EWT_Lightning;

	Damage = 25.f;

	//Distance and timing variables for Main->SpecialAttack
	SpecialStartLocation = FVector(0.f);
	SpecialForwardVector = FVector(0.f);
	SpecialRightVector = FVector(0.f);

	PillarBaseDistanceFromPlayer = 100.f;
	PillarMaxDistanceFromPlayer = 700.f;
	PillarCurrentDistanceFromPlayer = 100.f;
	PillarDelay = 0.1f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (WeaponState == EWeaponState::EWS_Pickup && OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main && Main->EquippedWeapon != this)
		{
			Main->SetActiveOverlappingItem(this);
		}
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->SetActiveOverlappingItem(nullptr);
		}
	}
}

void AWeapon::Equip(class AMain* Char)
{
	if (Char)
	{
		SetInstigator(Char->GetController());
		SetMain(Char);

		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);

		const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName("RightHandSocket");
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, Char->GetMesh());
			bRotate = false;

			Char->SetEquippedWeapon(this);
			Char->SetActiveOverlappingItem(nullptr);
		}
		if (OnEquipSound) UGameplayStatics::PlaySound2D(this, OnEquipSound);
		if (!bWeaponParticles)
		{
			IdleParticleComponent->Deactivate();
		}
	}
}

void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (Enemy->HitParticles)
			{
				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
				if (WeaponSocket)
				{
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, GetActorLocation(), FRotator(0.f), true);
				}
			}
			if (Enemy->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound);
			}
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, MainChar, DamageTypeClass);
			}
		}
	}
}

void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UGameplayStatics::PlaySound2D(this, SwingSound);
}

void AWeapon::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::WeaponSpecialStart()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSpecialStartParticles, IdleParticleComponent->GetComponentLocation(), FRotator(0.f), true);
}

void AWeapon::WeaponSpecialAttack()
{
	SpecialStartLocation = MainChar->GetActorLocation();
	SpecialForwardVector = MainChar->GetActorForwardVector();
	SpecialRightVector = MainChar->GetActorRightVector();
	switch (WeaponType)
	{
	case EWeaponType::EWT_Lightning:
		LightningSpecial();
		break;
	case EWeaponType::EWT_Rock:
		RockSpecial();
		break;
	case EWeaponType::EWT_Fire:
		FireSpecial();
		break;
	case EWeaponType::EWT_Ice:
		IceSpecial();
		break;
	}
}

void AWeapon::RockSpecial()
{
	UE_LOG(LogTemp, Warning, TEXT("ROCK"));
	SpawnOurSFX(SpecialStartLocation + (SpecialForwardVector * PillarCurrentDistanceFromPlayer));
}
void AWeapon::FireSpecial()
{
	FVector FwdRight = (SpecialForwardVector + SpecialRightVector) * 0.5f;
	FVector FwdLeft = (SpecialForwardVector - SpecialRightVector) * 0.5f;
	if (PillarCurrentDistanceFromPlayer <= PillarMaxDistanceFromPlayer)
	{
		SpawnOurSFX(SpecialStartLocation + (SpecialForwardVector * PillarCurrentDistanceFromPlayer));
		SpawnOurSFX(SpecialStartLocation + (FwdLeft * PillarCurrentDistanceFromPlayer));
		SpawnOurSFX(SpecialStartLocation + (FwdRight * PillarCurrentDistanceFromPlayer));
		PillarCurrentDistanceFromPlayer += PillarBaseDistanceFromPlayer;
		GetWorldTimerManager().SetTimer(PillarTimer, this, &AWeapon::FireSpecial, PillarDelay);
	}
	else { PillarCurrentDistanceFromPlayer = PillarBaseDistanceFromPlayer; }
}
void AWeapon::LightningSpecial()
{
	if (MainChar->bHasCombatTarget)
	{
		SpawnOurSFX(MainChar->CombatTargetLocation);
	}
	else
	{
		FVector FwdVec = MainChar->GetActorForwardVector();
		SpawnOurSFX(FwdVec * 500);
	}
}
void AWeapon::IceSpecial()
{
	if (PillarCurrentDistanceFromPlayer <= PillarMaxDistanceFromPlayer)
	{
		SpawnOurSFX(SpecialStartLocation + (SpecialForwardVector * PillarCurrentDistanceFromPlayer));
		PillarCurrentDistanceFromPlayer += PillarBaseDistanceFromPlayer;
		GetWorldTimerManager().SetTimer(PillarTimer, this, &AWeapon::IceSpecial, PillarDelay);
	}
	else { PillarCurrentDistanceFromPlayer = PillarBaseDistanceFromPlayer; }
}

void AWeapon::SpawnOurSFX(const FVector& Location)
{
	if (SFXToSpawn)
	{
		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		FVector Spawnlocation = Location + FVector(0.f, 0.f, -50.f);

		if (World)
		{
			AActor* Actor = World->SpawnActor<AActor>(SFXToSpawn, Spawnlocation, FRotator(0.f), SpawnParams);
		}
	}
}
*/