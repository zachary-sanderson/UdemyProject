// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalmeshComponent.h"
#include "Main.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "SpecialEffect.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "SpawnVolume.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere"));
	AggroSphere->SetupAttachment(GetRootComponent());
	AggroSphere->InitSphereRadius(600.f);
	AggroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	CombatCollisionL = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollisionL"));
	CombatCollisionR = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollisionR"));
	CombatCollisionL->SetupAttachment(GetMesh(), CombatCollisionLSocket);
	CombatCollisionR->SetupAttachment(GetMesh(), CombatCollisionRSocket);

	//Particles that show when hit with a special attack if any
	StatusParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("IdleParticlesComponent"));
	StatusParticleComponent->SetupAttachment(GetRootComponent());

	//Stats
	Health = 100.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	//Combat
	bOverlappingCombatSphere = false;
	bAttacking = false;

	AttackMinTime = 0.5f;
	AttackMaxTime = 1.f;

	DeathDelay = 3.f;

	bHasValidTarget = false;

	//Status Effects, all values editable in BPs
	bStunned = false;
	bJuggled = false;
	bBurned = false;
	bFrozen = false;

	StunLength = 3.f;
	JuggleLength = 3.f;
	BurnLength = 10.f;
	FrozenLength = 3.f;

	BurnDOTLength = 2.f;
	BurnDOTDamage = 10.f;

	//Used for boss only
	RecallLocation = FVector(0.f);
	bMovingToRecall = false;
	bIsInAnimation = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AggroSphereOnOverlapBegin);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollisionL->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatLeftOnOverlapBegin);

	CombatCollisionR->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRightOnOverlapBegin);

	AggroSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AggroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CombatCollisionL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionL->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollisionL->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollisionL->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CombatCollisionR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionR->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollisionR->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollisionR->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	AIController = Cast<AAIController>(GetController());

	StatusParticleComponent->Deactivate();
}

void AEnemy::AssignAIController()
{
	AIController = Cast<AAIController>(GetController());
}

// Called every frame
//A kind of hacky implementation of the bosses phase change, works well enough for the scope of the project but would need revision if expanding
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMovingToRecall)
	{
		if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			bMovingToRecall = false;
			AIController->StopMovement();

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				bAttacking = true;
				AnimInstance->Montage_Play(RecallMontage, 1.f);
				bRecalled = true;
			}
		}
	}
	else if (Health < MaxHealth / 2)
	{
		if (RecallMontage && !bRecalled)
		{
			if (!bAttacking)
			{
				if (AIController)
				{
					AIController->StopMovement();
					AIController->MoveTo(RecallLocation);
					FAIMoveRequest MoveRequest;
					MoveRequest.SetGoalLocation(RecallLocation);
					MoveRequest.SetAcceptanceRadius(10.f);

					FNavPathSharedPtr NavPath;

					AIController->MoveTo(MoveRequest, &NavPath);
					bMovingToRecall = true;
				}
			}
		}
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//When in aggro range move towards player
void AEnemy::AggroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			CombatTarget = Main;
			if (!bIsInAnimation && NoImpairingStatusEffect() && Alive())
				MoveToTarget(Main);
		}
	}
}

//When in attacking range begin attacking, provided timer allows it
void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive() && NoImpairingStatusEffect())
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) 
		{ 
			bHasValidTarget = true;

			bOverlappingCombatSphere = true;

			float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
		}
	}
}

//Reset timer and begin moving towards target, the boss can also do a unique attack when you leave his melee range
void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) 
		{ 
			bHasValidTarget = false;
			bOverlappingCombatSphere = false;
			if (!bAttacking && Alive() && NoImpairingStatusEffect())
			{
				if (SubjugationMontage) {
					int32 SwitchInt = FMath::RandRange(0, 1);
					UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
					switch (SwitchInt)
					{
					case 0:
						if (AnimInstance)
						{
							bAttacking = true;
							AnimInstance->Montage_Play(SubjugationMontage, 1.f);
						}
						break;
					case 1:
						MoveToTarget(Main);
						break;
					}
				}
				else
					MoveToTarget(Main);
			} 
			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

void AEnemy::MoveToTarget(class AMain* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(10.f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);
	}
}

//When left weapon connects deals damage and displays a particle effect at the location
void AEnemy::CombatLeftOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			if (Main->bBlocking)
			{
				Main->TakeStaminaDamage(15.f);
				if (Main->BlockSound)
				{
					UGameplayStatics::PlaySound2D(this, Main->BlockSound);
				}
				if (HitParticles)
				{
					const USkeletalMeshSocket* ShieldSocket = Main->GetMesh()->GetSocketByName("FX_Shield_Particles");
					if (ShieldSocket)
					{
						FVector SocketLocation = ShieldSocket->GetSocketLocation(Main->GetMesh());
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), true);
					}
				}
			}
			else
			{
				if (Main->HitSound)
				{
					UGameplayStatics::PlaySound2D(this, Main->HitSound);
				}
				if (HitParticles)
				{
					const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocketL");
					if (TipSocket)
					{
						FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, SocketLocation, FRotator(0.f), true);
					}
				}
				if (DamageTypeClass)
				{
					UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
				}
			}
		}
	}
}

//When right weapon connects deals damage and displays a particle effect at the location
void AEnemy::CombatRightOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			if (Main->bBlocking)
			{
				Main->TakeStaminaDamage(15.f);
				if (Main->BlockSound)
				{
					UGameplayStatics::PlaySound2D(this, Main->BlockSound);
				}
				if (Main->HitParticles)
				{
					const USkeletalMeshSocket* ShieldSocket = Main->GetMesh()->GetSocketByName("FX_Shield_Particles");
					if (ShieldSocket)
					{
						FVector SocketLocation = ShieldSocket->GetSocketLocation(GetMesh());
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), true);
					}
				}
			}
			else
			{
				if (Main->HitSound)
				{
					UGameplayStatics::PlaySound2D(this, Main->HitSound);
				}
				if (Main->HitParticles)
				{
					const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocketR");
					if (TipSocket)
					{
						FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), true);
					}
					const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName("Weapon_01");
					if (WeaponSocket)
					{
						FVector SocketLocation = WeaponSocket->GetSocketLocation(GetMesh());
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), true);
					}
				}
				if (DamageTypeClass)
				{
					UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
				}
			}
		}
	}
}

void AEnemy::ActivateCollisionL()
{
	CombatCollisionL->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCollisionL()
{
	CombatCollisionL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateCollisionR()
{
	CombatCollisionR->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCollisionR()
{
	CombatCollisionR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//Play weapon swing sound
void AEnemy::PlaySwingSound()
{
	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

/*
	**Stop moving and attack!**

	Deals with the varying attacks for the enemy and boss, 
	this is a sloppy implementation that would obviously not scale for multiple enemies/bosses,
	however serves it's purpose given the scope of this game
*/
void AEnemy::Attack()
{
	if (Alive() && NoImpairingStatusEffect() && bHasValidTarget)
	{
		if (SoulSiphonMontage && CombatMontage)
		{
			if (AIController)
			{
				AIController->StopMovement();
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			}
			if (!bAttacking)
			{
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					int32 SwitchInt = FMath::RandRange(0, 1);
					switch (SwitchInt)
					{
					case 0:
						bAttacking = true;
						AnimInstance->Montage_Play(CombatMontage, 1.f);
						AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage);
						break;
					case 1:
						bAttacking = true;
						AnimInstance->Montage_Play(SoulSiphonMontage, 1.f);
						break;
					default:
						MoveToTarget(CombatTarget);
						break;
					}
					
				}
			}
		}
		else if (CombatMontage)
		{
			if (AIController)
			{
				AIController->StopMovement();
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			}
			if (!bAttacking)
			{
				bAttacking = true;
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					AnimInstance->Montage_Play(CombatMontage, 1.f);
					AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage);
				}
			}
		}
	}
}

/*
	**Play the correct follow up attack in the combo or stop attacking!**

	Deals with the varying attacks for the enemy and boss,
	this is a sloppy implementation that would obviously not scale for multiple enemies/bosses,
	however serves it's purpose given the scope of this game
*/
void AEnemy::AttackEnd()
{
	bAttacking = false;

	if (!NoImpairingStatusEffect())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	FName CurrentSection;
	if (AnimInstance->GetActiveMontageInstance())
		CurrentSection = AnimInstance->GetActiveMontageInstance()->GetCurrentSection();
	else
	{
		MoveToTarget(CombatTarget);
		return;
	}


	int AttackNum = 1;

	if (CurrentSection == FName("Attack1"))
	{
		AttackNum = 1;
	}
	else if (CurrentSection == FName("Attack2"))
	{
		AttackNum = 2;
	}
	else if (CurrentSection == FName("Attack3"))
	{
		AttackNum = 3;
	}
	else if (CurrentSection == FName("Attack4"))
	{
		AttackNum = 4;
	}
	else if (CurrentSection == FName("Attack5"))
	{
		AttackNum = 5;
	}
	else 
	{
		return;
	}


	switch (AttackNum)
	{
	case 1:
		bAttacking = true;
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
		break;
	case 2:
		bAttacking = true;
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
		break;
	case 3:
		bAttacking = true;
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Attack4"), CombatMontage);
		break;
	case 4:
		bAttacking = true;
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Attack5"), CombatMontage);
		break;
	default:
		if (bOverlappingCombatSphere)
		{
			float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
		}
		else
		{
			MoveToTarget(CombatTarget);
		}
		break;
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.0f;
		Die(DamageCauser);
	}
	else { Health -= DamageAmount; }
	return DamageAmount;
}

//Death and cleanup
void AEnemy::Die(AActor* Causer)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.35f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	CombatCollisionL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollisionR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AggroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	AMain* Main = Cast<AMain>(Causer);
	if (Main /*&& Main->CombatTarget == this*/)
	{
		UE_LOG(LogTemp, Warning, TEXT("Removing lock on"));
		Main->SetCombatTarget(nullptr);
		Main->SetHasCombatTarget(false);
		Main->RemoveLockOn();
	}
	if (RecallSpawningBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempting to destroy Recall Box"));
		if (RSB)
		{
			RSB->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("Destroying Recall Box"));
		}
	}
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

bool AEnemy::NoImpairingStatusEffect()
{
	return (!bStunned && !bJuggled && !bFrozen && !bIsInAnimation && !bMovingToRecall);
}

void AEnemy::FinishIntro()
{
	bIsInAnimation = false;
	if (CombatTarget)
	{
		MoveToTarget(CombatTarget);
	}
	else
	{
		UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}
}

void AEnemy::Disappear()
{
	Destroy();
}

//Spawns a damaging effect at the players location
void AEnemy::Subjugation()
{
	AIController->StopMovement();
	if (CombatTarget && SubjugationSFXToSpawn)
	{
		FVector Location = CombatTarget->GetActorLocation();
		SpawnSFX(SubjugationSFXToSpawn, Location);
	}
}

void AEnemy::NotAttacking()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
	else
		MoveToTarget(CombatTarget);
}

//Drains all the players stamina
void AEnemy::SoulSiphon()
{
	const USkeletalMeshSocket* SiphonSocket = GetMesh()->GetSocketByName(FName("Cast_L"));
	if (SiphonSocket)
	{
		FVector SocketLocation = SiphonSocket->GetSocketLocation(GetMesh());
		SpawnSFX(SoulSiphonSFXToSpawn, SocketLocation);
	}
}

//Spawns Subjugations at random locations in the boss chamber on a timer
void AEnemy::Recall()
{
	SpawnSFX(RecallSpawningBox, RecallLocation);
}

void AEnemy::SpawnSFX(TSubclassOf<AActor> ActorToSpawn, FVector Location)
{
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (World)
	{
		AActor* Actor = World->SpawnActor<AActor>(ActorToSpawn, Location, FRotator(0.f), SpawnParams);
		ASpecialEffect* SpecialEffect = Cast<ASpecialEffect>(Actor);
		if (SpecialEffect)
		{
			SpecialEffect->Parent = this;
		}
		ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(Actor);
		if (SpawnVolume)
			RSB = SpawnVolume;
	}
}

/**************** STATUS EFFECTS **************/

void AEnemy::Stunned()
{
	bStunned = true;
	DeactivateCollisionL();
	DeactivateCollisionR();
	StatusParticleComponent->SetTemplate(StunParticles);
	StatusParticleComponent->Activate();
	if (AIController)
	{
		AIController->StopMovement();
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.25f);
	}
	GetWorldTimerManager().SetTimer(StunTimer, this, &AEnemy::NotStunned, StunLength);
}
void AEnemy::NotStunned()
{
	bStunned = false;
	StatusParticleComponent->Deactivate();
	Recover();
}

void AEnemy::Juggled()
{
	bJuggled = true;
	DeactivateCollisionL();
	DeactivateCollisionR();
	if (AIController)
	{
		AIController->StopMovement();
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->StopAllMontages(0.25f);
	}
	GetWorldTimerManager().SetTimer(JuggleTimer, this, &AEnemy::NotJuggled, JuggleLength);
}
void AEnemy::NotJuggled()
{
	bJuggled = false;
	Recover();
}

void AEnemy::Burned()
{
	if (!bBurned)
	{
		bBurned = true;
		StatusParticleComponent->SetTemplate(BurnParticles);
		StatusParticleComponent->Activate();
		GetWorldTimerManager().SetTimer(BurnTimer, this, &AEnemy::NotBurned, BurnLength);
		BurnDOT();
	}
}
void AEnemy::NotBurned()
{
	bBurned = false;
	StatusParticleComponent->Deactivate();
}

void AEnemy::BurnDOT()
{
	if (bBurned)
	{
		if (Health - BurnDOTDamage <= 0)
			CombatTarget->RemoveLockOn();
		UGameplayStatics::ApplyDamage(this, BurnDOTDamage, GetController(), this, DamageTypeClass);
		GetWorldTimerManager().SetTimer(BurnTimer, this, &AEnemy::BurnDOT, BurnLength);
	}
}

void AEnemy::Frozen()
{
	bFrozen = true;
	DeactivateCollisionL();
	DeactivateCollisionR();

	if (AIController)
	{
		AIController->StopMovement();
		GetMesh()->bPauseAnims = true;
		GetMesh()->bNoSkeletonUpdate = true;
	}
	StatusParticleComponent->SetTemplate(FreezeParticles);
	StatusParticleComponent->Activate();
	GetWorldTimerManager().SetTimer(FreezeTimer, this, &AEnemy::NotFrozen, FrozenLength);
}
void AEnemy::NotFrozen()
{
	bFrozen = false;
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
	StatusParticleComponent->Deactivate();
	Recover();
}

void AEnemy::Recover()
{
	bAttacking = false;
	if (bOverlappingCombatSphere) Attack();

	else if (CombatTarget) MoveToTarget(CombatTarget);
}