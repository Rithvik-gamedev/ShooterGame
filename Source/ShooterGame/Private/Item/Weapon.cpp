// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Weapon.h"

#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Kismet/GameplayStatics.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemType =  EItemType::EIT_Weapon;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ItemState == EItemState::EIS_Falling && bIsFalling)
	{
		const FRotator MeshRotation = FRotator(0.f, ItemMesh->GetComponentRotation().Yaw, 0.f);
		ItemMesh->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	UpdatePistolSlideDisplacement();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	//CurrentAmmo = MaxAmmo;
	if (BoneNameToHide != TEXT(""))
	{
		ItemMesh->HideBoneByName(BoneNameToHide, PBO_None);
	}
	
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FString WeaponDataPath = TEXT("/Script/Engine.DataTable'/Game/_Game/DataTables/WeaponTypeDataTable.WeaponTypeDataTable'");
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponDataPath));
	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponTableRow = nullptr;

		switch (WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:
			WeaponAmmoType = EAmmoType::EAT_9MM;
			WeaponTableRow = WeaponTableObject->FindRow<FWeaponDataTable>(TEXT("SubmachineGun"), TEXT(""));
			break;
		case EWeaponType::EWT_AssaultRifle:
			WeaponAmmoType = EAmmoType::EAT_AssaultRifle;
			WeaponTableRow = WeaponTableObject->FindRow<FWeaponDataTable>(TEXT("AssaultRifle"), TEXT(""));
			break;
		case EWeaponType::EWT_Pistol:
			WeaponAmmoType = EAmmoType::EAT_9MM;
			WeaponTableRow = WeaponTableObject->FindRow<FWeaponDataTable>(TEXT("Pistol"), TEXT(""));
			break;
		}

		if (WeaponTableRow)
		{
			CurrentAmmo = WeaponTableRow->CurrentAmmo;
			MaxAmmo = WeaponTableRow->MaxAmmo;
			AmmoIconInventory = WeaponTableRow->AmmoIcon;
			ItemIcon = WeaponTableRow->InventoryIcon;
			SetPickupSound(WeaponTableRow->PickupSound);
			SetEquipSound(WeaponTableRow->EquipSound); 
			ItemName = WeaponTableRow->ItemName;
			ItemMesh->SetSkeletalMesh(WeaponTableRow->ItemMesh);
			
			MaterialInstance = WeaponTableRow->MaterialInstance;
			PreviousMaterialIndex = MaterialIndex;
			ItemMesh->SetMaterial(PreviousMaterialIndex, nullptr);
			MaterialIndex = WeaponTableRow->MaterialIndex;

			ReloadMontageSectionName = WeaponTableRow->ReloadMontageSectionName;
			ClipBoneName = WeaponTableRow->ClipBoneName;

			ItemMesh->SetAnimInstanceClass(WeaponTableRow->WeaponAnimInstanceClass);

			CrosshairMiddle = WeaponTableRow->CrosshairMiddle;
			CrosshairTop = WeaponTableRow->CrosshairTop;
			CrosshairBottom = WeaponTableRow->CrosshairBottom;
			CrosshairLeft = WeaponTableRow->CrosshairLeft;
			CrosshairRight = WeaponTableRow->CrosshairRight;

			FireRate = WeaponTableRow->FireRate;
			MuzzleFlash = WeaponTableRow->MuzzleFlash;
			FireSound = WeaponTableRow->FireSound;
			BoneNameToHide = WeaponTableRow->BoneNameToHide;

			bAutomatic = WeaponTableRow->bAutomatic;

			Damage = WeaponTableRow->Damage;
			HeadShotDamage = WeaponTableRow->HeadShotDamage;
		}

		switch (ItemRarity)
		{
		case EItemRarity::EWR_Damaged:
			Damage -= 2.f;
			HeadShotDamage -= 4.f;
			break;
		case EItemRarity::EWR_Uncommon:
			Damage += 2.f;
			HeadShotDamage += 4.f;
			break;
		case EItemRarity::EWR_Rare:
			Damage += 4.f;
			HeadShotDamage += 6.f;
			break;

		case EItemRarity::EWR_Legendary:
			Damage += 6.f;
			HeadShotDamage += 10.f;
			break;
			
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

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation = FRotator (0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward =  GetItemMesh()->GetForwardVector() ;
	const FVector MeshRight = GetItemMesh()->GetRightVector() ;
	// Direction in which we throw the Weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation = 30.f ;
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ItemMesh->AddImpulse(ImpulseDirection * ThrowWeaponImpulse);
	
	bIsFalling = true;
	GetWorldTimerManager().SetTimer(ThrowTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
	EnableGlowMaterial();
}

void AWeapon::StopFalling()
{
	bIsFalling = false;
	SetItemState(EItemState::EIS_PickUp);
	StartPulseTimer();
}

void AWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, SlideDisplacementTime);
}

void AWeapon::FinishMovingSlide()
{
	bMovingSlide = false;
}

void AWeapon::UpdatePistolSlideDisplacement()
{
	if (SlideDisplacementCurve && bMovingSlide)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(SlideTimer);
		const float CurveValue = SlideDisplacementCurve->GetFloatValue(ElapsedTime);
		PistolSlideDisplacement = CurveValue * MaxSlideDistance;
		RecoilRotation = CurveValue * MaxRecoilRotation;
	}
}

void AWeapon::DecrementAmmo()
{
	if (CurrentAmmo > MaxAmmo)
	{
		CurrentAmmo = MaxAmmo;
	}
	if (CurrentAmmo - 1 <= 0 )
	{
		bHasAmmo = true;
		CurrentAmmo = 0;
	}
	else
	{
		--CurrentAmmo;
	}
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(CurrentAmmo + Amount <= MaxAmmo, TEXT("Attempted to reload with more than max capacity"))
	CurrentAmmo += Amount;
}



