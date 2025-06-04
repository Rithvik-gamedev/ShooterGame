// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "Enum/AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AAmmo : public AItem
{
	GENERATED_BODY()

public:

	AAmmo();

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	virtual void SetItemStateProperties(EItemState State) override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	//UFUNCTION()
	//void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	TObjectPtr<UStaticMeshComponent> AmmoMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	EAmmoType AmmoType = EAmmoType::EAT_9MM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	TObjectPtr<UTexture2D> AmmoIcon;

public:

	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }


	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;
};
