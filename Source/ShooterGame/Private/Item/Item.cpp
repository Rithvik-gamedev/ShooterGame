// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Item.h"

#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Item Mesh");
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>("Collision");
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>("Pickup Widget");
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>("Area Trace Sphere");
	AreaSphere->SetupAttachment(GetRootComponent());
	
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//load the data in ItemRarityTable
	//Path to ItemRarityDataTable
	FString RarityTablePath(TEXT("/Script/Engine.DataTable'/Game/_Game/DataTables/ItemRarityDataTable.ItemRarityDataTable'"));
	UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath));
	if (RarityTableObject)
	{
		FItemRarityTable* RarityRow = nullptr;

		switch (ItemRarity)
		{
		case EItemRarity::EWR_Damaged:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(TEXT("Damaged"), TEXT(""));
			break;
		case EItemRarity::EWR_Common:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(TEXT("Common"), TEXT(""));
			break;
		case EItemRarity::EWR_Uncommon:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(TEXT("Uncommon"), TEXT(""));
			break;
		case EItemRarity::EWR_Rare:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(TEXT("Rare"), TEXT(""));
			break;
		case EItemRarity::EWR_Legendary:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(TEXT("Legendary"), TEXT(""));
			break;
		}

		if (RarityRow)
		{
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStars = RarityRow->NumberOfStars;
			IconBackground = RarityRow->IconBackground;
			if (ItemMesh)
			{
				ItemMesh->SetCustomDepthStencilValue(RarityRow->CustomDepthStencil);
			}
		}
	}

	if (MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);
		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
		EnableGlowMaterial();
	}
}

void AItem::SetItemRarity()
{
	// Using static cast to do enum as int, it will follow the order from top to bottom assuming enums are in order
	// Note: be vary of array size as it can crash the editor due to out of bounds or array size
	uint8 RarityCount = static_cast<uint8>(EItemRarity::EWR_DefaultMAX) + 1;
	ItemRarityArray.SetNum(RarityCount);
	
	for (uint8 i = 0; i <= ItemRarityArray.Num() - 1; i++)
	{
		ItemRarityArray[i] = false;
	}

	for (uint8 i = 0; i <= static_cast<int32>(ItemRarity); i++)
	{
		ItemRarityArray[i] = true;
	}
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	SetItemRarity();
	SetItemStateProperties(ItemState);
	
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereBeginOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	InitializeCustomDepth();

	StartPulseTimer();
}



// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);

	//Get Curve Values from Pulse curve and Set Dynamic Curve Values
	UpdatePulse();
}

void AItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter && OtherActor == ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlapItemCount(1);
			ShooterCharacter->AddToOverlapItemArray(this);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter && OtherActor == ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlapItemCount(-1);
			ShooterCharacter->RemoveFromOverlappedItemArray(this);
			ShooterCharacter->UnHighlightInventorySlot();
		}
	}
	
}

void AItem::SetItemStateProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_PickUp:
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Overlap);

		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

	case EItemState::EIS_EquipInterping:
		PickupWidget->SetVisibility(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);

		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		break;

	case EItemState::EIS_PickedUp:
		PickupWidget->SetVisibility(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		break;

	case EItemState::EIS_Equipped:

		PickupWidget->SetVisibility(false);
		
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);

		DisableCustomDepth();
		DisableGlowMaterial();
		break;

	case EItemState::EIS_Falling:
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		
		
		break;

	default:
		break;
	}
}

void AItem::SetItemState(const EItemState State)
{
	ItemState = State;
	SetItemStateProperties(State);
}

void AItem::StartInterpItem(AShooterCharacter* ShooterCharacter)
{
	Character = ShooterCharacter;

	//Get array Index in InterpLocations with the lowest item count
	InterpLocIndex = Character->GetItemInterpLocationIndex();
	//Add 1 to ItemInterpCount fot this interp location struct
	Character->IncrementInterpLocItemCount(InterpLocIndex, 1);
	
	if (PickupSound)
	{
		//UGameplayStatics::PlaySound2D(this, PickupSound);
		StartPickupSoundTimer();
	}
	ItemInterpLocation = GetActorLocation();
	/*
	if (Character)
	{
		CameraInterpTargetLocation = Character->GetCameraInterpLocation();
	}
	*/
	bIsInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);
	GetWorldTimerManager().ClearTimer(PulseTimer);

	GetWorldTimerManager().SetTimer(StartInterpTimer, this, &AItem::FinishInterpItem, ZCurveTime);

	bCanChangeCustomDepth = false;
	
}



void AItem::FinishInterpItem()
{
	if (EquipSound)
	{
		//UGameplayStatics::PlaySound2D(this, EquipSound);
		StartEquipSoundTimer();
	}
	bIsInterping = false;
	if (Character)
	{
		//Subtract -1 from the InterpItemCount for the interp location struct
		Character->IncrementInterpLocItemCount(InterpLocIndex, -1);
		Character->GetPickupItem(this);
		Character->UnHighlightInventorySlot();
	}
	SetActorScale3D(FVector(1.f));
	DisableGlowMaterial();
	bCanChangeCustomDepth = true;
	DisableCustomDepth();
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bIsInterping) return;

	if (Character && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(StartInterpTimer);
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		FVector ItemLocation = ItemInterpLocation;
		const FVector CameraLocation = GetInterpLocation();
		const FVector ItemToCamera = FVector(0.f, 0.f, (CameraLocation - ItemLocation).Z);
		ItemLocation.Z += CurveValue * ItemToCamera.Size();
		
		const FRotator ItemRotation = GetActorRotation();
		const FRotator CameraRotation = Character->GetFollowCamera()->GetComponentRotation();
		//InterpRotationYawOffset = ItemRotation.Yaw - CameraRotation.Yaw;
		//FRotator ItemRotationYaw = FRotator(0.f, CameraRotation.Yaw + InterpRotationYawOffset, 0.f);
		//SetActorRotation(ItemRotationYaw, ETeleportType::TeleportPhysics);
		const double RotationYaw = UKismetMathLibrary::FInterpTo(ItemRotation.Yaw, CameraRotation.Yaw, DeltaTime,40.f);
		SetActorRotation(FRotator(0.f, RotationYaw, 0.f), ETeleportType::TeleportPhysics);
		
		const FVector CurrentLocation = GetActorLocation();
		ItemLocation.X = UKismetMathLibrary::FInterpTo(CurrentLocation.X,CameraLocation.X, DeltaTime, 30.f);
		ItemLocation.Y = UKismetMathLibrary::FInterpTo(CurrentLocation.Y,CameraLocation.Y, DeltaTime, 30.f);
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue));
		}
	}
}

FVector AItem::GetInterpLocation()
{
	if (Character == nullptr) return FVector(0.f);

	switch (ItemType)
	{
	case EItemType::EIT_Ammo:
		return Character->GetInterpLocation(InterpLocIndex).SceneComponent->GetComponentLocation();
		break;

	case EItemType::EIT_Weapon:
		return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
		break;

	case EItemType::EIT_DefaultMaX:
		return Character->GetCameraInterpLocation();
		
	default:
		return FVector(0.f);
		break;
	}
}

void AItem::StartPickupSoundTimer()
{
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AItem::EndPickupSoundTimer, 0.2f);
}

void AItem::EndPickupSoundTimer()
{
	if (PickupSound)
	{
		UGameplayStatics::PlaySound2D(this,PickupSound);
		GetWorldTimerManager().ClearTimer(PickupSoundTimer);
	}
}

void AItem::StartEquipSoundTimer()
{
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AItem::EndEquipSoundTimer, 0.2f);
}

void AItem::EndEquipSoundTimer()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySound2D(this,EquipSound);
		GetWorldTimerManager().ClearTimer(EquipSoundTimer);
	}
}

void AItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}

void AItem::EnableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"),0.f);
	}

}

void AItem::DisableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"),1.f);
	}
}

void AItem::UpdatePulse()
{
	float ElapsedTime;
	FVector CurveValue = FVector(0.f);
	switch (ItemState)
	{
	case EItemState::EIS_PickUp:
		if (PulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
			CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	case EItemState::EIS_EquipInterping:
		if (InterpPulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(StartInterpTimer);
			CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	}

	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelReflectFraction"), CurveValue.Z * FresnelReflectFraction);
	}
	
	
}

void AItem::EnableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(true);
	}
}

void AItem::DisableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(false);
	}
}

void AItem::ResetPulseTimer()
{
	GetWorldTimerManager().ClearTimer(PulseTimer);
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if (ItemState == EItemState::EIS_PickUp)
	{
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}
}
