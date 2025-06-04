// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ammo.h"

#include "Character/ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo()
{
	PrimaryActorTick.bCanEverTick = true;

	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>("Ammo Static Mesh");
	SetRootComponent(AmmoMesh);

	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	PickupWidget->SetupAttachment(RootComponent);
	AreaSphere->SetupAttachment(RootComponent);
	
	
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::OnBoxBeginOverlap);
}

void AAmmo::SetItemStateProperties(EItemState State)
{
	Super::SetItemStateProperties(State);

	switch (State)
	{
	case EItemState::EIS_PickUp:
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		
		break;

	case EItemState::EIS_EquipInterping:
		PickupWidget->SetVisibility(false);
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_PickedUp:
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetVisibility(false);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_Equipped:
		
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		break;

	case EItemState::EIS_Falling:
		AmmoMesh->SetSimulatePhysics(true);
		AmmoMesh->SetEnableGravity(true);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		AmmoMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		
		break;

	default:
		break;
	}
	
}

void AAmmo::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(OtherActor);
	if (OtherActor == ShooterChar)
	{
		StartInterpItem(ShooterChar);
	}
}

void AAmmo::EnableCustomDepth()
{
	Super::EnableCustomDepth();
	AmmoMesh->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth()
{
	Super::DisableCustomDepth();
	AmmoMesh->SetRenderCustomDepth(false);
}
