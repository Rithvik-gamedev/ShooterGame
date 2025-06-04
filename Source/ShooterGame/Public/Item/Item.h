// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UCurveVector;
class AShooterCharacter;
class USphereComponent;
class UWidgetComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EWR_Damaged UMETA(DisplayName = "Damaged"),
	EWR_Common UMETA(DisplayName = "Common"),
	EWR_Uncommon UMETA(DisplayName = "Uncommon"),
	EWR_Rare UMETA(DisplayName = "Rare"),
	EWR_Legendary UMETA(DisplayName = "Legendary"),

	EWR_DefaultMAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_PickUp UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_DefaultMaX UMETA(DisplayName = "DefaultMaX")
	
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),

	EIT_DefaultMaX UMETA(DisplayName = "DefaultMaX")
	
};

USTRUCT()
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
	
};



UCLASS()
class SHOOTERGAME_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();
	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	void SetItemRarity();
	
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void SetItemStateProperties(EItemState State);

	void FinishInterpItem();

	void ItemInterp(float DeltaTime);
	
	FVector GetInterpLocation();

	void StartPickupSoundTimer();
	void EndPickupSoundTimer();

	void StartEquipSoundTimer();
	void EndEquipSoundTimer();

	void InitializeCustomDepth();

	void EnableGlowMaterial();
	void DisableGlowMaterial();

	void UpdatePulse();
	void ResetPulseTimer();
	void StartPulseTimer();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<USkeletalMeshComponent> ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<UWidgetComponent> PickupWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<USphereComponent> AreaSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	EItemState ItemState = EItemState::EIS_PickUp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	EItemType ItemType = EItemType::EIT_DefaultMaX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	EItemRarity ItemRarity = EItemRarity::EWR_Common;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties")
	TArray<bool> ItemRarityArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	FString ItemName = TEXT("DefaultName");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	int32 ItemCount = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<UCurveFloat> ItemZCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<UCurveFloat> ItemScaleCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	FVector ItemInterpLocation = FVector(0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	FVector CameraInterpTargetLocation = FVector(0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	bool bIsInterping;
	
	FTimerHandle StartInterpTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	float ZCurveTime = 0.7f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<AShooterCharacter> Character;

	UPROPERTY(EditAnywhere, Category = "Item Sounds")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, Category = "Item Sounds")
	TObjectPtr<USoundBase> EquipSound;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	//Index of the interp location this item is Interping to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	int32 InterpLocIndex = 0;

	
	//Index for Material to change at runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties")
	int32 MaterialIndex = 0;

	//Dynamic Instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;

	//Material Instance used with Dynamic Material Instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties")
	TObjectPtr<UMaterialInstance> MaterialInstance;
	
	bool bCanChangeCustomDepth = true;

	//Curve to drive the dynamic material parameters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<UCurveVector> PulseCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
	TObjectPtr<UCurveVector> InterpPulseCurve;

	FTimerHandle PulseTimer;

	//Time for the pulse Timer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
	float PulseCurveTime = 5.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	float GlowAmount = 150.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	float FresnelExponent = 3.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties")
	float FresnelReflectFraction = 4.f;

	//Icon for the item in inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTexture2D> ItemIcon;

	//ammo Icon for the item in inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTexture2D> AmmoIconInventory;

	//slot in the inventory array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Table")
	TObjectPtr<UDataTable> ItemRarityDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity")
	FLinearColor GlowColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity")
	FLinearColor LightColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity")
	FLinearColor DarkColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity")
	int32 NumberOfStars;

	//Background for this item in inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity")
	TObjectPtr<UTexture2D> IconBackground;
	
public:

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget;}
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const {return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE USoundBase* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundBase* GetEquipSound() const { return EquipSound; }
	FORCEINLINE void SetPickupSound(USoundBase* Sound) { PickupSound = Sound; }
	FORCEINLINE void SetEquipSound(USoundBase* Sound) { EquipSound = Sound; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex;}
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	void SetItemState(const EItemState State);

	void StartInterpItem(AShooterCharacter* ShooterCharacter);

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	

};
