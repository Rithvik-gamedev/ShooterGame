// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Enum/AmmoType.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponType : uint8 
{
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"),
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),

	EWT_DefaultMAX UMETA(DisplayName = "DefaultMAX"),
};

USTRUCT()
struct FWeaponDataTable: public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWidgetComponent> PickupWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstance> MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> WeaponAnimInstanceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneNameToHide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotDamage = 25.f;
};

UCLASS()
class SHOOTERGAME_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;
	
protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	void StopFalling();

	void FinishMovingSlide();

	void UpdatePistolSlideDisplacement();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
	int32 CurrentAmmo = 0;

	//MagazineCapacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
	int32 MaxAmmo = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	EWeaponType WeaponType = EWeaponType::EWT_SubmachineGun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	EAmmoType WeaponAmmoType = EAmmoType::EAT_9MM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	FName ReloadMontageSectionName = TEXT("ReloadSMG");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reload Properties")
	FName ClipBoneName = TEXT("smg_clip");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reload  Properties")
	bool bIsClipMoving = false;

	FTimerHandle ThrowTimer;
	float ThrowWeaponTime = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	double ThrowWeaponImpulse = 5000.f;
	
	bool bIsFalling = false;

	bool bHasAmmo = false;

	//Data Table for weapon Properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category ="Data Table")
	TObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY()
	int32 PreviousMaterialIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Crosshair Textures")
	TObjectPtr<UTexture2D> CrosshairMiddle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Crosshair Textures")
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Crosshair Textures")
	TObjectPtr<UTexture2D> CrosshairBottom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Crosshair Textures")
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Crosshair Textures")
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FireRate = 0.08f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	FName BoneNameToHide;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	bool bAutomatic;

	/*Pistol*/
	
	//Amount that the slide is pushed back during fire
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol")
	float PistolSlideDisplacement = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol")
	TObjectPtr<UCurveFloat> SlideDisplacementCurve;
	
	FTimerHandle SlideTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol")
	float SlideDisplacementTime = 0.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol")
	bool bMovingSlide = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol")
	float MaxSlideDistance = 4.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol")
	float RecoilRotation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol")
	float MaxRecoilRotation = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	float Damage = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	float HeadShotDamage = 25.f;

private:

	
public:
	
	void ThrowWeapon();
	void DecrementAmmo();
	void ReloadAmmo(int32 Amount);
	
	
	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
	FORCEINLINE EAmmoType GetWeaponAmmoType() const {return WeaponAmmoType;}
	FORCEINLINE FName GetReloadMontageSectionName() const { return ReloadMontageSectionName;}
	FORCEINLINE bool GetIsClipMoving() const { return bIsClipMoving; }
	FORCEINLINE void SetIsClipMoving(bool IsMoving) { bIsClipMoving = IsMoving; }
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName;}
	FORCEINLINE float GetFireRate() const { return FireRate; }
	FORCEINLINE UParticleSystem* GeMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundBase* GetFireSound() const { return FireSound; }
	FORCEINLINE bool GetAutomatic() const {return bAutomatic;}
	FORCEINLINE float GetDamage() const { return Damage;}
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage;}

	void StartSlideTimer();
	
	
};
