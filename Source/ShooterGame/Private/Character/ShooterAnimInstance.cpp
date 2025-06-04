// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterAnimInstance.h"

#include "Character/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Weapon.h"
#include "Kismet/KismetMathLibrary.h"


void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr ) return;
	if (Speed > 0 || bIsinAir)
	{
		RootYawOffset = UKismetMathLibrary::FInterpTo(RootYawOffset, 0.f, GetWorld()->GetDeltaSeconds(),20.f);
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		DistanceCurveLastFrame = 0.f;
		DistanceCurve = 0.f;
		
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);
		
		//RootOffsetYaw > 0 means turning left
		const float Turning = GetCurveValue(TEXT("Turning"));
		if (Turning > 0)
		{
			DistanceCurveLastFrame = DistanceCurve;
			DistanceCurve = GetCurveValue(TEXT("RotationCurve"));
			const float DeltaDistanceCurve = DistanceCurve - DistanceCurveLastFrame;

			RootYawOffset > 0 ? RootYawOffset -= DeltaDistanceCurve : RootYawOffset += DeltaDistanceCurve;

			const float AbsRootYawOffset = FMath::Abs(RootYawOffset);
			if (AbsRootYawOffset > 90.f)
			{
				const float YawExcess = AbsRootYawOffset - 90.f;
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = (Delta.Yaw)/DeltaTime;
	const float Interp = FMath::FInterpTo(LeanYawDelta, Target, DeltaTime, 20.f);
	LeanYawDelta = FMath::Clamp(Interp, -90.f, 90.f);
	
}

void UShooterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		const FVector Velocity = ShooterCharacter->GetVelocity(); 
		Speed = Velocity.Size2D();

		//Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, ShooterCharacter->GetActorRotation());

		
		FRotator VelocityRotFromX = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		FRotator NormalizedRot = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotFromX, ShooterCharacter->GetBaseAimRotation());
		Direction = NormalizedRot.Yaw;

		if (Speed > 0.f)
		{
			DirectionLastFrame = Direction;
		}

		bIsCrouching = ShooterCharacter->IsCrouching();
		bIsinAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		bIsAiming = ShooterCharacter->IsAiming();
		bShoudlUseFABRIK = ShooterCharacter->GetCharacterState() == ECharacterState::ECS_Unoccupied || ShooterCharacter->GetCharacterState() == ECharacterState::ECS_Shooting;

		if (ShooterCharacter->GetEquippedWeapon())
		{
			WeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
		}
		
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}
	}

	TurnInPlace();
	Lean(DeltaSeconds);
}


