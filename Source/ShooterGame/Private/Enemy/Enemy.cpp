// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Character/ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy/EnemyController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	AgroSphere = CreateDefaultSubobject<USphereComponent>("AgroSphere");
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>("CombatRangeSphere");
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>("LeftWeaponCollision");
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponSocket"));
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>("RightWeaponCollision");
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponSocket"));
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnAgroSphereBeginOverlap);
	//AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnAgroSphereEndOverlap);

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatRangeBeginOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatRangeEndOverlap);

	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponBeginOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponBeginOverlap);
	
	//Get AI controller
	EnemyController = Cast<AEnemyController>(GetController());
	
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("Patrol1"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("Patrol2"), WorldPatrolPoint2);
		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHitNumbers();

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	HideHealthBar();
	PlayDeathMontage(TEXT("Death"));
	
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
		EnemyController->StopMovement();
	}
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);

	FTimerHandle DestroyTimer;
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AEnemy::DestroySelf, EnemyDestroyTime);
}

FName AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
{
	if (IsDead()) return FName("");
	const FVector ForwardVector = GetActorForwardVector();
	const FVector HitVector = (ImpactPoint - GetActorLocation()).GetSafeNormal2D();
	float CosTheta = FVector::DotProduct(ForwardVector, HitVector);

	float Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	FVector CrossProduct =  FVector::CrossProduct(ForwardVector, HitVector);
	if (CrossProduct.Z < 0) Theta *= -1.f;
	UE_LOG(LogTemp, Warning, TEXT("Theta : %f"), Theta);

	FName SectionName("FromFront");
	

	if (Theta >= 45.f && Theta < 135.f) SectionName = FName("FromRight");
	if (Theta <= -45.f && Theta > -135.f) SectionName = FName("FromLeft");
	if (Theta >= 135.f || Theta <= -135.f) SectionName = FName("FromBack");

	return SectionName;
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	if (!bCanHitReact) return;
	UAnimInstance* AnimInstance =  GetMesh()->GetAnimInstance();
	if ( AnimInstance && HitMontage)
	{
		AnimInstance->Montage_Play(HitMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitMontage);
		bCanHitReact = false;
		const float ReactTime = FMath::RandRange(HitReactTimeMin, HitReactTimeMax);
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, ReactTime);
	}
}

void AEnemy::PlayDeathMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance =  GetMesh()->GetAnimInstance();
	if ( AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
		//AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
	}
}

FName AEnemy::PlayAttackMontage(const TArray<FName>& SectionName)
{
	if (AttackMontage == nullptr || SectionName.Num() <= 0) return FName("");
	int32 Selection = FMath::RandRange(0, SectionName.Num() -1);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(SectionName[Selection]);
	}
	return SectionName[Selection];
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, TEXT("RemoveHitNumber"), HitNumber);
	GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::UpdateHitNumbers()
{
	for (auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber = HitPair.Key;
		const FVector Location = HitPair.Value;
		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenPosition);
		HitNumber->SetPositionInViewport(ScreenPosition);
	}
}

void AEnemy::OnAgroSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr || EnemyController == nullptr) return;
	if (!EnemyController->GetBlackboardComponent()) return;

	if (const auto Character = Cast<AShooterCharacter>(OtherActor))
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
	}
}

void AEnemy::OnAgroSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//EnemyController->GetBlackboardComponent()->ClearValue(TEXT("Target"));
}

void AEnemy::OnCombatRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	if (const auto Character = Cast<AShooterCharacter>(OtherActor) && EnemyController)
	{
		bInAttackRange = true;
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		//PlayAttackMontage(AttackMontageSectionNames);
	}
}

void AEnemy::OnCombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	if (const auto Character = Cast<AShooterCharacter>(OtherActor) && EnemyController)
	{
		bInAttackRange = false;
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
	}
}

void AEnemy::OnLeftWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor))
	{
		DoDamage(Character);
		//Not Being Used - SpawnBlood(Character, LeftWeaponSocket);
	}
}

void AEnemy::OnRightWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor))
	{
		DoDamage(Character);
		//Not Being Used - SpawnBlood(Character, RightWeaponSocket);
	}
}
/*
void AEnemy::LeftWeaponTrace()
{
	FHitResult HitResult;
	const FVector Start = LeftSceneComponentStart->GetComponentLocation();
	const FVector End = LeftSceneComponentEnd->GetComponentLocation();
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	UKismetSystemLibrary::SphereTraceSingle(this, Start, End, 24.f, TraceTypeQuery1, false,ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true);
}*/

void AEnemy::DestroySelf()
{
	Destroy();
}

void AEnemy::DoDamage(AShooterCharacter* Character)
{
	if (Character == nullptr ) return;
	//LeftWeaponTrace();
	UGameplayStatics::ApplyDamage(Character, BaseDamage, EnemyController, this, UDamageType::StaticClass());
	if (Character->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Character->GetMeleeImpactSound(), Character->GetActorLocation());

		//Added this Instead of SpawnBlood
		if (Character->GetBloodParticles()) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Character->GetBloodParticles(), Character->GetActorLocation());
		StunCharacter(Character);
	}
	
}

//Not Being Used
void AEnemy::SpawnBlood(AShooterCharacter* Character, FName SocketName)
{
	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(SocketName);
	if (WeaponSocket)
	{
		const FTransform SocketTransform = WeaponSocket->GetSocketTransform(GetMesh());
		if (Character->GetBloodParticles()) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Character->GetBloodParticles(), SocketTransform);
	}
}

void AEnemy::StunCharacter(AShooterCharacter* Character)
{
	if (Character)
	{
		const float Stun = FMath::FRandRange(0.f, 1.f);
		if (Stun <= Character->GetStunChance())
		{
			Character->Stun();
		}
	}
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}
}

void AEnemy::RemoveHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent(); 
}

void AEnemy::BulletHit_Implementation(FHitResult& HitResult, AController* ActorController, AActor* DamageCauserActor)
{
	//IHitInterface::BulletHit_Implementation(HitResult);

	if (ImpactSound && ImpactParticles)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ImpactSound, GetActorLocation());
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticles, HitResult.Location);
	}
	ShowHealthBar();
	/*
	const float Stunned = FMath::RandRange(0.f, 1.f);
	if (StunChance >= Stunned)
	{
		DirectionalHitReact(HitResult.Location);
		SetStunned(true);
	}*/
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
	AActor* DamageCauser)
{
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), DamageCauser);
	}
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}
	ShowHealthBar();

	
	const float Stunned = FMath::RandRange(0.f, 1.f);
	if (StunChance >= Stunned)
	{
		PlayHitReactMontage(TEXT("FromFront"));
		SetStunned(true);
	}
	
	return DamageAmount;
	
}

