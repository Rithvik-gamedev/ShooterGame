// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterCharacter.h"

#include "ShooterGame/ShooterGame.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Enemy/Enemy.h"
#include "Enemy/EnemyController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interfaces/HitInterface.h"
#include "Item/Ammo.h"
#include "Item/Item.h"
#include "Item/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/ShooterPlayerController.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.f;
	SpringArm->SocketOffset = FVector(0.f, 50.f, 50.f);
	SpringArm->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritRoll = true;
	SpringArm->bInheritYaw = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>("Camera");
	FollowCamera->SetupAttachment(SpringArm);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 90.f;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	WeaponClipSceneComponent = CreateDefaultSubobject<USceneComponent>("WeaponClip");

	InitializeItemInterpComp();
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<AShooterPlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(ShooterMappingContext, 0);
		}
	}
	CameraDefaultFOV = FollowCamera->FieldOfView;
	CameraCurrentFOV = CameraDefaultFOV;
	BaseMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;

	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	InitializeAmmoMap();
	//Create FInterpLocations Struct for each interp location. Add to Array
	InitializeInterpLocations();
}


void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetSensitivityAndFOV(DeltaTime);
	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();
	AutoReload();
	InterpCapsuleHeight(DeltaTime);
	
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceHitResult;
		CrosshairScreenTrace(ItemTraceHitResult);
		if (ItemTraceHitResult.bBlockingHit)
		{
			TracedHitItem = Cast<AItem>(ItemTraceHitResult.GetActor());
			const AWeapon* TraceHitWeapon = Cast<AWeapon>(TracedHitItem);
			if (TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
				{
					//No Currently highlighting slot; highlight one
					HighlightInventorySlot();	
				}
			}
			else
			{
				// is a Slot is being highlighted
				if (HighlightedSlot != -1)
				{
					//UnHighlightSlot
					UnHighlightInventorySlot();
				}
			}
			if (TracedHitItem && TracedHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TracedHitItem = nullptr;
			}
			
			if (TracedHitItem && OverlappedItem.Contains(TracedHitItem) && TracedHitItem->GetPickupWidget())
			{
				TracedHitItem->GetPickupWidget()->SetVisibility(true);
				TracedHitItem->EnableCustomDepth();
			}

			if (TracedHitItemLastFrame && TracedHitItem != TracedHitItemLastFrame)
			{
				TracedHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				TracedHitItemLastFrame->DisableCustomDepth();
			}
			
			TracedHitItemLastFrame = TracedHitItem;
		}
	}
	else if (TracedHitItemLastFrame)
	{
		TracedHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TracedHitItemLastFrame->DisableCustomDepth();
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInput)
	{
		EnhancedInput->BindAction(MoveIA, ETriggerEvent::Triggered, this, &AShooterCharacter::MovePlayer);
		EnhancedInput->BindAction(LookIA, ETriggerEvent::Triggered, this, &AShooterCharacter::MouseLook);
		EnhancedInput->BindAction(JumpIA, ETriggerEvent::Triggered, this, &AShooterCharacter::JumpChar);
		
		EnhancedInput->BindAction(ShootIA, ETriggerEvent::Started, this, &AShooterCharacter::Shoot);
		EnhancedInput->BindAction(ShootIA, ETriggerEvent::Completed, this, &AShooterCharacter::ShootComplete);

		EnhancedInput->BindAction(AimingIA, ETriggerEvent::Triggered, this, &AShooterCharacter::AimPressed);
		EnhancedInput->BindAction(AimingIA, ETriggerEvent::Completed, this, &AShooterCharacter::AimReleased);

		EnhancedInput->BindAction(EquipItemIA, ETriggerEvent::Triggered, this, &AShooterCharacter::EquipButton);
		EnhancedInput->BindAction(ReloadIA, ETriggerEvent::Started, this, &AShooterCharacter::Reload);

		EnhancedInput->BindAction(CrouchIA, ETriggerEvent::Triggered, this, &AShooterCharacter::CrouchChar);

		EnhancedInput->BindAction(DefaultWeaponQuickSlotIA, ETriggerEvent::Triggered, this, &AShooterCharacter::DefaultWeaponQuickSlot);
		EnhancedInput->BindAction(OneQuickSlotIA, ETriggerEvent::Triggered, this, &AShooterCharacter::OneQuickSlot);
		EnhancedInput->BindAction(TwoQuickSlotIA, ETriggerEvent::Triggered, this, &AShooterCharacter::TwoQuickSlot);
		EnhancedInput->BindAction(ThreeQuickSlotIA, ETriggerEvent::Triggered, this, &AShooterCharacter::ThreeQuickSlot);
		EnhancedInput->BindAction(FourQuickSlotIA, ETriggerEvent::Triggered, this, &AShooterCharacter::FourQuickSlot);
		EnhancedInput->BindAction(FiveQuickSlotIA, ETriggerEvent::Triggered, this, &AShooterCharacter::FiveQuickSlot);
	}
}

void AShooterCharacter::SetSensitivityAndFOV(float DeltaTime)
{
	if (bIsAiming)
	{
		CameraCurrentFOV = UKismetMathLibrary::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, CameraAimingSpeed);
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		CameraCurrentFOV = UKismetMathLibrary::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, CameraAimingSpeed);
		BaseTurnRate = HipFireTurnRate;
		BaseLookUpRate = HipFireLookUpRate;
	}
	FollowCamera->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::MovePlayer(const FInputActionValue& Value)
{
	const FVector2D InputAxisVector = Value.Get<FVector2D>();
	
	FRotator Rotation = GetControlRotation();
	FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InputAxisVector.Y);
	AddMovementInput(RightDirection, InputAxisVector.X);
	
}

void AShooterCharacter::MouseLook(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();

	AddControllerYawInput(InputVector.X * BaseTurnRate);
	AddControllerPitchInput(-InputVector.Y * BaseLookUpRate);
	
}

void AShooterCharacter::JumpChar(const FInputActionValue& Value)
{
	const bool bInput = Value.Get<bool>();
	if (bInput)
	{
		Jump();
	}
}

void AShooterCharacter::DefaultWeaponQuickSlot(const FInputActionValue& Value)
{
	const bool KeyInput = Value.Get<bool>();
	if (KeyInput)
	{
		if (EquippedWeapon->GetSlotIndex() == 0) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);	
	}
}

void AShooterCharacter::OneQuickSlot(const FInputActionValue& value)
{
	const bool KeyInput = value.Get<bool>();
	if (KeyInput)
	{
		if (EquippedWeapon->GetSlotIndex() == 1) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);	
	}
}

void AShooterCharacter::TwoQuickSlot(const FInputActionValue& value)
{
	const bool KeyInput = value.Get<bool>();
	if (KeyInput)
	{
		if (EquippedWeapon->GetSlotIndex() == 2) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);	
	}
}

void AShooterCharacter::ThreeQuickSlot(const FInputActionValue& value)
{
	const bool KeyInput = value.Get<bool>();
	if (KeyInput)
	{
		if (EquippedWeapon->GetSlotIndex() == 3) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);	
	}
}

void AShooterCharacter::FourQuickSlot(const FInputActionValue& value)
{
	const bool KeyInput = value.Get<bool>();
	if (KeyInput)
	{
		if (EquippedWeapon->GetSlotIndex() == 4) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);	
	}
}

void AShooterCharacter::FiveQuickSlot(const FInputActionValue& value)
{
	const bool KeyInput = value.Get<bool>();
	if (KeyInput)
	{
		if (EquippedWeapon->GetSlotIndex() == 5) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);	
	}
}

void AShooterCharacter::ShootWeapon()
{
	if (!WeaponHasAmmo()) return;
	if (IsDead()) return;
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;
	CharacterState = ECharacterState::ECS_Shooting;
	const USkeletalMeshSocket* MuzzleSocket = EquippedWeapon->GetItemMesh()->GetSocketByName(TEXT("WeaponMuzzleSocket"));
	if (MuzzleSocket)
	{
		const FTransform SocketTransform = MuzzleSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		PlayGunFireMontage();
		WeaponLineTrace();
		bFiringBullet = true;
		GetWorldTimerManager().SetTimer(ShootingCrosshairSpreadTimer, this, &AShooterCharacter::FinishShootingCrosshairSpread, ShootingCrosshairSpreadTime);
		EquippedWeapon->DecrementAmmo();

		if (EquippedWeapon->GeMuzzleFlash() && EquippedWeapon->GetFireSound())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GeMuzzleFlash() , SocketTransform);
			UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
		}
	}
	StartFireTimer();

	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
	{
		EquippedWeapon->StartSlideTimer();
	}
	
}

void AShooterCharacter::Shoot(const FInputActionValue& Value)
{
	const bool bInputShoot = Value.Get<bool>();
	
	if (bInputShoot)
	{
		if (CharacterState == ECharacterState::ECS_Unoccupied)
		{
			bIsShootButtonPressed = true;
			ShootWeapon();
		}
	}
	
}

void AShooterCharacter::ShootComplete()
{
	bIsShootButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (EquippedWeapon == nullptr) return;

	GetWorldTimerManager().SetTimer(ShootTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetFireRate());
}

void AShooterCharacter::AutoFireReset()
{
	if (CharacterState == ECharacterState::ECS_Stunned) return;
	if (EquippedWeapon == nullptr) return;
	
	CharacterState = ECharacterState::ECS_Unoccupied;
	if (WeaponHasAmmo())
	{
		if (bIsShootButtonPressed && EquippedWeapon->GetAutomatic())
		{
			ShootWeapon();
		}
	}
	else
	{
		Reload(true);
	}
}

void AShooterCharacter::EndStun()
{
	CharacterState = ECharacterState::ECS_Unoccupied;
	if (bIsAiming)
	{
		Aim();
	}
}


void AShooterCharacter::PlayGunFireMontage()
{
	if (GunFireMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(GunFireMontage);
		//AnimInstance->Montage_JumpToSection(TEXT("HipFire"), GunFireMontage);
	}
}


void AShooterCharacter::WeaponLineTrace()
{
	FHitResult WeaponHitResult;
	const USkeletalMeshSocket* MuzzleSocket = EquippedWeapon->GetItemMesh()->GetSocketByName(TEXT("WeaponMuzzleSocket"));
	const FTransform SocketTransform = MuzzleSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
	const FVector WeaponStartLocation = SocketTransform.GetLocation();
	const FQuat SocketRotation = SocketTransform.GetRotation();
	const FVector WeaponEndLocation = WeaponStartLocation + SocketRotation.GetAxisX() * 1000.f;
	FVector BeamEndPoint = WeaponEndLocation;

	/*ScreenTrace*/
	FHitResult ScreenTraceHit;
	if (CrosshairScreenTrace(ScreenTraceHit))
	{
		if (ScreenTraceHit.bBlockingHit)
		{
			const FVector BarrelStartToEnd = ScreenTraceHit.Location - WeaponStartLocation;
			BeamEndPoint = WeaponStartLocation + BarrelStartToEnd * 1.25f;
		}
	}
	/*ScreenTrace*/

	/* WeaponTrace*/
	GetWorld()->LineTraceSingleByChannel(WeaponHitResult, WeaponStartLocation, BeamEndPoint,ECC_Visibility);
	//DrawDebugLine(GetWorld(), WeaponStartLocation, BeamEndPoint ,  FColor::Orange, false, 5.f, 0, 1.f);
	if (WeaponHitResult.bBlockingHit)
	{
		if (WeaponHitResult.GetActor()) //Call BulletHit if it hits an enemy
		{
			if (IHitInterface* HitInterface = Cast<IHitInterface>(WeaponHitResult.GetActor()))
			{
				HitInterface->BulletHit_Implementation(WeaponHitResult, GetController(), this);

				if (AEnemy* HitEnemy = Cast<AEnemy>(WeaponHitResult.GetActor()))
				{
					int32 Damage{};
					bool bHeadshot = false;
					if (WeaponHitResult.BoneName == HitEnemy->GetHeadBone())
					{
						Damage = EquippedWeapon->GetHeadShotDamage();
						bHeadshot = true;
					}
					else
					{
						Damage = EquippedWeapon->GetDamage();
						bHeadshot = false;
					}
					UGameplayStatics::ApplyDamage(HitEnemy, Damage, GetController(), this, UDamageType::StaticClass());
					HitEnemy->ShowHitNumbers(Damage, WeaponHitResult.Location, bHeadshot);
				}
			}
			else if(HitParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(this, HitParticle, WeaponHitResult.Location);
				//GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Emerald, TEXT("Default Hit Particles"));
			}
		}
		//DrawDebugSphere(GetWorld(), WeaponHitResult.Location, 25.f, 12, FColor::Orange, false, 5.f);
	}
	if (BeamParticle)
	{
		UParticleSystemComponent* BeamPoint = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticle, SocketTransform);
		if (BeamPoint)
		{
			//BeamPoint->SetVectorParameter(FName("Target"), BeamEndPoint);
			BeamPoint->SetVectorParameter(FName("Target"), BeamEndPoint);
		}
	}
	
}

bool AShooterCharacter::CrosshairScreenTrace(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation = FVector2D(ViewportSize.X/2.f, ViewportSize.Y/2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	if (PlayerController == nullptr) return false;
	const bool bScreenToWorld =  UGameplayStatics::DeprojectScreenToWorld(PlayerController, CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	const FVector Start = CrosshairWorldPosition;
	const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 10000.f;
	if (bScreenToWorld)
	{
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 5.f);
		if (HitResult.bBlockingHit)
		{
			//DrawDebugSphere(GetWorld(),HitResult.Location, 25.f, 12, FColor::Green, false, 5.f);
			return true;
		}
	}
	return false;
}


void AShooterCharacter::EquipButton(const FInputActionValue& Value)
{
	bool bInput = Value.Get<bool>();
	if (PlayerController && bInput)
	{
		if (CharacterState != ECharacterState::ECS_Unoccupied) return;
		if (TracedHitItem)
		{
			TracedHitItem->StartInterpItem(this);
			TracedHitItem = nullptr;
		}
	}
}

void AShooterCharacter::Reload(const FInputActionValue& Value)
{
	bool bInput = Value.Get<bool>();
	if (bInput)
	{
		if (CharacterState != ECharacterState::ECS_Unoccupied) return;
		if (EquippedWeapon == nullptr) return;
		if (EquippedWeapon->GetCurrentAmmo() >= EquippedWeapon->GetMaxAmmo()) return;
		if (CarriedAmmo())
		{
			PlayReloadMontage();
			CharacterState = ECharacterState::ECS_Reloading;
		}
		
	}
}

void AShooterCharacter::CrouchChar(const FInputActionValue& Value)
{
	bool bInput = Value.Get<bool>();

	if (bInput)
	{
		if (!GetCharacterMovement()->IsFalling())
		{
			bIsCrouching = !bIsCrouching;
			if (bIsCrouching)
			{
				GetCharacterMovement()->MaxWalkSpeed = CrouchMoveSpeed;
				GetCharacterMovement()->GroundFriction = 8.f;
				GetCharacterMovement()->BrakingDecelerationWalking = 1024.f;
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
				GetCharacterMovement()->GroundFriction = 2.f;
				GetCharacterMovement()->BrakingDecelerationWalking = 85.f;
			}
		}
	}
}

void AShooterCharacter::PlayReloadMontage()
{
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;
	if (ReloadMontage && EquippedWeapon)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(ReloadMontage);
		AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSectionName());
	}
}

void AShooterCharacter::PlayEquipMontage()
{
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		CharacterState = ECharacterState::ECS_Equipping;
	}
}

void AShooterCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AShooterCharacter::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;

	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.f, 0.f, -400.f);

	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());;
}

void AShooterCharacter::FinishReloading()
{
	if (CharacterState == ECharacterState::ECS_Stunned) return;
	CharacterState = ECharacterState::ECS_Unoccupied;
	if (EquippedWeapon == nullptr) return;
	EAmmoType WeaponAmmoType = EquippedWeapon->GetWeaponAmmoType();

	if (AmmoMap.Contains(WeaponAmmoType))
	{
		int32 CarriedAmmo = AmmoMap[WeaponAmmoType];

		const int32 MagSpace = EquippedWeapon->GetMaxAmmo() - EquippedWeapon->GetCurrentAmmo();
		if (MagSpace > CarriedAmmo)
		{
			//Dump all carried ammo into magazine
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(WeaponAmmoType, CarriedAmmo);
		}
		else
		{
			//fill the magazine
			EquippedWeapon->ReloadAmmo(MagSpace);
			CarriedAmmo -= MagSpace;
			AmmoMap.Add(WeaponAmmoType, CarriedAmmo);
		}
	}
	if (bIsShootButtonPressed && CharacterState == ECharacterState::ECS_Unoccupied)
	{
		StartFireTimer();
	}
}

void AShooterCharacter::FinishEquipping()
{
	if (CharacterState == ECharacterState::ECS_Stunned) return;
	CharacterState = ECharacterState::ECS_Unoccupied;
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (WeaponClipSceneComponent == nullptr) return;
	
	int32 ClipBoneIndex = EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName());
	FTransform ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules TransformRules(EAttachmentRule::KeepRelative, true);
	WeaponClipSceneComponent->AttachToComponent(GetMesh(),TransformRules, FName(TEXT("Hand_L")));
	WeaponClipSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetIsClipMoving(true);
	
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetIsClipMoving(false);
}

bool AShooterCharacter::CarriedAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	EAmmoType WeaponAmmoType = EquippedWeapon->GetWeaponAmmoType();
	if (AmmoMap.Contains(WeaponAmmoType))
	{
		return AmmoMap[WeaponAmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::AutoReload()
{
	if (EquippedWeapon == nullptr || CharacterState != ECharacterState::ECS_Unoccupied) return;

	if (EquippedWeapon->GetCurrentAmmo() <= 0)
	{
		Reload(true);
	}
}

void AShooterCharacter::InterpCapsuleHeight(float DeltaTime)
{
	float TargetHeight;
	if (bIsCrouching) 
	{
		TargetHeight = CrouchingCapsuleHalfHeight;
	}
	else TargetHeight = StandingCapsuleHalfHeight;
	
	const float InterpHalfHeight = UKismetMathLibrary::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetHeight, DeltaTime, 20.f);
	const float DeltaHeight = InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetMesh()->AddLocalOffset(FVector(0.f, 0.f, -DeltaHeight));
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::FinishShootingCrosshairSpread()
{
	bFiringBullet = false;
	GetWorldTimerManager().ClearTimer(ShootingCrosshairSpreadTimer);
}


void AShooterCharacter::Aim()
{
	bIsAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = AimMoveSpeed;
	GetCharacterMovement()->GroundFriction = 8.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1024.f;
	if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMoveSpeed;
		SpringArm->SocketOffset = FVector(0.f, 50.f, 20.f);
	}
	else
	{
		SpringArm->SocketOffset = FVector(0.f, 50.f, 50.f);
	}
}

void AShooterCharacter::AimPressed(const FInputActionValue& Value)
{
	bool bValue = Value.Get<bool>();
	if (PlayerController && bValue)
	{
		if (CharacterState != ECharacterState::ECS_Stunned)
		{
			Aim();
		}
	}
}

void AShooterCharacter::AimReleased()
{
	bIsAiming = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
	GetCharacterMovement()->GroundFriction = 2.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 85.f;
	if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMoveSpeed;
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return  GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;
	if (WeaponToEquip)
	{
		const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("WeaponSocket"));

		if (WeaponSocket)
		{
			WeaponSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		
		if (EquippedWeapon == nullptr)
		{
			// -1 == no EquippedWeapon yet. No need to reverse the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToEquip)
{
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;

	if (Inventory.Num() -1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToEquip;
		WeaponToEquip->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	
	DropWeapon();
	EquipWeapon(WeaponToEquip);
	TracedHitItem = nullptr;
	TracedHitItemLastFrame = nullptr;
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetCurrentAmmo() > 0;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9MM, Default9MM_Ammo);
	AmmoMap.Add(EAmmoType::EAT_AssaultRifle, DefaultAR_Ammo);
}

void AShooterCharacter::DropWeapon()
{
	if (CharacterState != ECharacterState::ECS_Unoccupied) return;
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
		
	}
	
}

void AShooterCharacter::IncrementOverlapItemCount(const int32 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}


void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	const FVector2D WalkSpeedRange = FVector2D(0, GetCharacterMovement()->MaxWalkSpeed);
	const FVector2D VelocityMultiplyRange = FVector2D(0.f, 1.f);
	CrosshairVelocityFactor =  FMath::GetMappedRangeValueClamped(WalkSpeedRange,VelocityMultiplyRange,GetVelocity().Size2D());

	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = UKismetMathLibrary::FInterpTo(CrosshairInAirFactor, 2.f, DeltaTime, 25.f);
	}
	else
	{
		CrosshairInAirFactor = UKismetMathLibrary::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if (bIsAiming)
	{
		CrosshairAimFactor = UKismetMathLibrary::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 10.f);
	}
	else
	{
		CrosshairAimFactor = UKismetMathLibrary::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	if (bFiringBullet)
	{
		CrosshairShootingFactor = UKismetMathLibrary::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = UKismetMathLibrary::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 60.f);
	}
	

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

// No longer needed; AItem has GetInterpLocation
  
FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraLocation = FollowCamera->GetComponentLocation();
	const FVector CameraForward = FollowCamera->GetForwardVector();

	FVector DesiredLocation = CameraLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);

	return DesiredLocation;
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount = AmmoMap[Ammo->GetAmmoType()];
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetWeaponAmmoType() == Ammo->GetAmmoType())
	{
		if (EquippedWeapon->GetCurrentAmmo() == 0)
		{
			Reload(true);
		}
	}
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentSlotIndex, int32 NewSlotIndex)
{
	bool bCanExchange = (CurrentSlotIndex != NewSlotIndex) && (NewSlotIndex < Inventory.Num()) && (CharacterState == ECharacterState::ECS_Unoccupied || CharacterState == ECharacterState::ECS_Equipping);
	if (bCanExchange)
	{
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewSlotIndex]);
		EquipWeapon(NewWeapon);
		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);
		PlayEquipMontage();
	}
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}
	
	if (Inventory.Num() < InventoryCapacity)
	{
		return Inventory.Num();
	}
	return -1; //inventory is full
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot = GetEmptyInventorySlot();
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

float AShooterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();

		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("CharacterDead"), true);
		}
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AShooterCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	if (IsDead()) return;
	const FVector ForwardVector = GetActorForwardVector();
	const FVector HitVector = (ImpactPoint - GetActorLocation()).GetSafeNormal2D();
	float CosTheta = FVector::DotProduct(ForwardVector, HitVector);

	float Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	FVector CrossProduct =  FVector::CrossProduct(ForwardVector, HitVector);
	if (CrossProduct.Z < 0) Theta *= -1.f;
	UE_LOG(LogTemp, Warning, TEXT("Theta : %f"), Theta);

	FName SectionName("FromFront");
	

	if (Theta >= 45.f && Theta < 135.f) SectionName = FName("FromRight");
	if (Theta <= -45.f && Theta > -135.f) SectionName = FName("FromLeft");
	if (Theta >= 135.f || Theta <= -135.f) SectionName = FName("FromBack");

	PlayHitReactMontage(SectionName);
}

void AShooterCharacter::Die()
{
	PlayDeathMontage();
	
	//AShooterPlayerController* PlayerControllerShooter = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

bool AShooterCharacter::IsDead()
{
	return Health == 0.f;
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

void AShooterCharacter::Stun()
{
	if (IsDead()) return;
	CharacterState = ECharacterState::ECS_Stunned;
	PlayHitReactMontage(TEXT("FromFront"));
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	if (AWeapon* Weapon = Cast<AWeapon>(Item))
	{
		if (Inventory.Num() < InventoryCapacity)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else // Inventory is full and swap weapon
		{
			SwapWeapon(Weapon);
		}
		
	}
	else if (AAmmo* Ammo = Cast<AAmmo>(Item))
	{
		PickupAmmo(Ammo);
	}
	
}



void AShooterCharacter::AddToOverlapItemArray(AItem* Item)
{
	OverlappedItem.AddUnique(Item);
}

void AShooterCharacter::RemoveFromOverlappedItemArray(AItem* Item)
{
	if (OverlappedItem.Contains(Item))
	{
		OverlappedItem.Remove(Item);
	}
}

void AShooterCharacter::InitializeItemInterpComp()
{
	WeaponInterpComponent = CreateDefaultSubobject<USceneComponent>("Weapon Interp Component");
	WeaponInterpComponent->SetupAttachment(FollowCamera);

	InterpComponents.SetNum(6);
	for (int i = 0; i < InterpComponents.Num(); i++)
	{
		InterpComponents[i] = CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("Interp Component %d"), i));
		InterpComponents[i]->SetupAttachment(FollowCamera);
	}
}
FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation(WeaponInterpComponent, 0);
	InterpLocations.Add(WeaponLocation);

	for (int32 i = 0; i < InterpComponents.Num(); i++)
	{
		FInterpLocation ItemInterpLoc(InterpComponents[i], 0);
		InterpLocations.AddUnique(ItemInterpLoc);
	}
}

int32 AShooterCharacter::GetItemInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount  = INT_MAX;

	for (int i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemInterpCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemInterpCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemInterpCount += Amount;
	}
}
