// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive/Explosive.h"

#include "Components/SphereComponent.h"
#include "Enemy/Enemy.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AExplosive::AExplosive()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>("ExplosiveMesh");
	SetRootComponent(ExplosiveMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>("OverlapSphere");
	OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult& HitResult, AController* ActorController, AActor* DamageCauserActor)
{
	//IHitInterface::BulletHit_Implementation(HitResult);

	if (ImpactParticle && ImpactSound)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticle, HitResult.Location);
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	TArray<AActor*> OverlappingActors;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (auto Actor : OverlappingActors)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Actor Damaged : %s"), *Actor->GetName());
		//UGameplayStatics::ApplyRadialDamageWithFalloff(this, ExplosiveDamage, ExplosiveMinimumDamage, HitResult.Location, 150.f, OverlapSphere->GetScaledSphereRadius(), 10.f, UDamageType::StaticClass(), ActorsToIgnore);
		UGameplayStatics::ApplyDamage(Actor, ExplosiveDamage, ActorController, DamageCauserActor, UDamageType::StaticClass());
		//DrawDebugSphere(GetWorld(), HitResult.Location, OverlapSphere->GetScaledSphereRadius(), 24, FColor::Green);

		/*
		FHitResult SphereHit;
		UKismetSystemLibrary::SphereTraceSingle(this, HitResult.Location, (HitResult.Location * 200.f), OverlapSphere->GetScaledSphereRadius(), ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, SphereHit, true);
		if (SphereHit.bBlockingHit)
		{
			if (AEnemy* SphereEnemy = Cast<AEnemy>(SphereHit.GetActor()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Sphere Trace Hit Actor: %s"), *SphereEnemy->GetName())
			}
		} */
	}
	
	Destroy();
}

