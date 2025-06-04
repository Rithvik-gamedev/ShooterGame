// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

enum class EWeaponType : uint8;
class AShooterCharacter;
/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:

	virtual void NativeInitializeAnimation() override;

	void TurnInPlace();

	void Lean(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	TObjectPtr<AShooterCharacter> ShooterCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	bool bIsinAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	bool bIsAccelerating;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	bool bIsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	bool bIsCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	float Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	float DirectionLastFrame;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter Character")
	float RootYawOffset = 0.f;

	float TIPCharacterYaw = 0.f;

	float TIPCharacterYawLastFrame = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator CharacterRotation = FRotator(0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator CharacterRotationLastFrame = FRotator(0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LeanYawDelta = 0.f;
	
	float DistanceCurve = 0.f;

	float DistanceCurveLastFrame = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EWeaponType WeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bShoudlUseFABRIK = false;
	
	
};
