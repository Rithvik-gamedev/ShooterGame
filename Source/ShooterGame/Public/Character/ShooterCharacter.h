// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Enum/AmmoType.h"
#include "ShooterCharacter.generated.h"

class AAmmo;
class AWeapon;
class AItem;
class AShooterPlayerController;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UNiagaraSystem;
struct FInputActionValue;


UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Shooting UMETA(DisplayName = "Shooting"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),

	ECS_DefaultMAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemInterpCount = 0;

	bool operator==(const FInterpLocation& Other) const
	{
		return SceneComponent == Other.SceneComponent && ItemInterpCount == Other.ItemInterpCount;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class SHOOTERGAME_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	
protected:
	
	virtual void BeginPlay() override;
	

	/* Input */
	void MovePlayer(const FInputActionValue& Value);
	void MouseLook(const FInputActionValue& Value);
	void JumpChar(const FInputActionValue& Value);
	void Shoot(const FInputActionValue& Value);
	void ShootComplete();
	void AimPressed(const FInputActionValue& Value);
	void AimReleased();
	void EquipButton(const FInputActionValue& Value);
	void Reload(const FInputActionValue& Value);
	void CrouchChar(const FInputActionValue& Value);
	void DefaultWeaponQuickSlot(const FInputActionValue& Value);
	void OneQuickSlot(const FInputActionValue& value);
	void TwoQuickSlot(const FInputActionValue& value);
	void ThreeQuickSlot(const FInputActionValue& value);
	void FourQuickSlot(const FInputActionValue& value);
	void FiveQuickSlot(const FInputActionValue& value);
	void ShootWeapon();
	/*Input*/

	/*Aiming*/
	void WeaponLineTrace();
	void SetSensitivityAndFOV(float DeltaTime);
	void CalculateCrosshairSpread(float DeltaTime);
	void FinishShootingCrosshairSpread();
	void Aim();
	bool CrosshairScreenTrace(FHitResult& HitResult);
	/*Aiming*/

	/*Montages*/
	void PlayGunFireMontage();
	void PlayReloadMontage();
	void PlayEquipMontage();
	void PlayHitReactMontage(const FName& SectionName);
	void PlayDeathMontage();
	/*Montages*/

	/*Footsteps*/

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();
	
	/*Weapon*/
	AWeapon* SpawnDefaultWeapon();
	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwapWeapon(AWeapon* WeaponToEquip);
	void DropWeapon();
	bool WeaponHasAmmo();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void GrabClip();

	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void StartFireTimer();

	void AutoFireReset();

	UFUNCTION(BlueprintCallable)
	void EndStun();
	/*Weapon*/

	/*Item*/

	void InitializeItemInterpComp();
	/*Item*/

	/*Character*/
	void InitializeAmmoMap();
	void TraceForItems();
	bool CarriedAmmo();
	void AutoReload();
	void InterpCapsuleHeight(float DeltaTime);

	void PickupAmmo(AAmmo* Ammo);
	void ExchangeInventoryItems(int32 CurrentSlotIndex, int32 NewSlotIndex);
	
	int32 GetEmptyInventorySlot();
	void HighlightInventorySlot();

	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	void DirectionalHitReact(const FVector& ImpactPoint);
	void Die();
	/*Character*/

	UPROPERTY(EditAnywhere, Category = "Spring")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Spring")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<AShooterPlayerController> PlayerController;

	/* Input */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> ShooterMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShootIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AimingIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> EquipItemIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ReloadIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> CrouchIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> DefaultWeaponQuickSlotIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> OneQuickSlotIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> TwoQuickSlotIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ThreeQuickSlotIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FourQuickSlotIA;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FiveQuickSlotIA;
	/* Input */

	/* Cosmetic */

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UParticleSystem> HitParticle;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UParticleSystem> BeamParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	TObjectPtr<USoundBase> MeleeImpactSound;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UParticleSystem> BloodParticles;
	/* Cosmetic */

	/* Montages */
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TObjectPtr<UAnimMontage> GunFireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TObjectPtr<UAnimMontage> DeathMontage;
	/* Montages */

	/*Aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsShootButtonPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bFiringBullet = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	float CameraDefaultFOV;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	float CameraCurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CameraZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CameraAimingSpeed = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming Sensitivity", meta = (ClampMin = 0.0f, ClampMax = 1.2f, UIMin = 0.0f, UIMax = 1.2f))
	float BaseTurnRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming Sensitivity", meta = (ClampMin = 0.0f, ClampMax = 1.2f, UIMin = 0.0f, UIMax = 1.2f))
	float BaseLookUpRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming Sensitivity", meta = (ClampMin = 0.0f, ClampMax = 1.2f, UIMin = 0.0f, UIMax = 1.2f))
	float HipFireTurnRate = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming Sensitivity", meta = (ClampMin = 0.0f, ClampMax = 1.2f, UIMin = 0.0f, UIMax = 1.2f))
	float HipFireLookUpRate = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming Sensitivity", meta = (ClampMin = 0.0f, ClampMax = 1.2f, UIMin = 0.0f, UIMax = 1.2f))
	float AimingTurnRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming Sensitivity", meta = (ClampMin = 0.0f, ClampMax = 1.2f, UIMin = 0.0f, UIMax = 1.2f))
	float AimingLookUpRate = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairSpreadMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairAimFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairShootingFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float ShootingCrosshairSpreadTime = 0.1f;

	FTimerHandle ShootingCrosshairSpreadTimer;
	
	/*Aiming*/

	
	/*Items*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	bool bShouldTraceForItems = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	int32 OverlappedItemCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	TObjectPtr<AItem> TracedHitItemLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	TObjectPtr<AItem> TracedHitItem;
	/*Items*/

	/*Item Interp*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	TObjectPtr<USceneComponent> WeaponInterpComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	TArray<USceneComponent*> InterpComponents;
	/*Item Interp*/
	
	/*Weapon*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	float CameraInterpDistance = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	float CameraInterpElevation = 65.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	EAmmoType AmmoType = EAmmoType::EAT_9MM;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	int32 Default9MM_Ammo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	int32 DefaultAR_Ammo = 45;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	TObjectPtr<USceneComponent> WeaponClipSceneComponent;

	
	FTimerHandle ShootTimer;
	/*Weapon*/

	/*Character*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Properties")
	ECharacterState CharacterState = ECharacterState::ECS_Unoccupied;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Properties")
	float BaseMoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Properties")
	float AimMoveSpeed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Properties")
	float CrouchMoveSpeed = 270.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Properties")
	float StandingCapsuleHalfHeight = 88.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Properties")
	float CrouchingCapsuleHalfHeight = 55.f;

	// Delegate for Sending slot information to InventoryBar when Equipping
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FEquipItemDelegate EquipItemDelegate;

	//Delegate for sending slot info for playing icon animation
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FHighlightIconDelegate HighlightIconDelegate;

	// Index for the currently highlighted slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 HighlightedSlot = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float StunChance = 0.25f;

	
	/*Character*/

private:

	//Array of no. of items that are overlapping with item's area sphere
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = true))
	TArray<AItem*> OverlappedItem;

	//Array of Item Interp Location Struct 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FInterpLocation> InterpLocations;

	//Array of AItems for our Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = true))
	TArray<AItem*> Inventory;

	const int32 InventoryCapacity = 6;
	
public:

	UFUNCTION(BlueprintPure)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int32 GetOverlappedItemCount() const { return OverlappedItemCount;}

	FORCEINLINE int32 SetOverlappedItemCount(const int32 Amount) { return OverlappedItemCount += Amount;}

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera;}

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState;}

	FORCEINLINE USoundBase* GetMeleeImpactSound() const { return MeleeImpactSound; }
	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }
	FORCEINLINE float GetStunChance() const {return StunChance; }

	bool IsAiming() const { return bIsAiming; }

	bool IsCrouching() const { return bIsCrouching; }

	//No longer needed; AItem has GetInterpLocation
	FVector GetCameraInterpLocation();

	void GetPickupItem (AItem* Item);

	void IncrementOverlapItemCount(int32 Amount);

	void AddToOverlapItemArray(AItem* Item);
	void RemoveFromOverlappedItemArray(AItem* Item);

	FInterpLocation GetInterpLocation(int32 Index);
	int32 GetItemInterpLocationIndex();
	void InitializeInterpLocations();
	void IncrementInterpLocItemCount(int32 Index, int32 Amount);
	void UnHighlightInventorySlot();
	void Stun();

	UFUNCTION(BlueprintCallable)
	bool IsDead();
};

