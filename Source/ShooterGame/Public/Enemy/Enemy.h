// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Enemy.generated.h"

class AShooterCharacter;
class UBoxComponent;
class USphereComponent;
class AEnemyController;
class UBehaviorTree;
class UWidget;

UCLASS()
class SHOOTERGAME_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();
	void ShowHealthBar_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	void Die();

	FName DirectionalHitReact(const FVector& ImpactPoint);

	void PlayHitReactMontage(const FName& SectionName);
	void PlayDeathMontage(const FName& SectionName);

	UFUNCTION(BlueprintCallable)
	FName PlayAttackMontage(const TArray<FName>& SectionName);

	void ResetHitReactTimer();
	
	UFUNCTION()
	void RemoveHitNumber(UUserWidget* HitNumber);

	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber , FVector Location);

	void UpdateHitNumbers();

	UFUNCTION()
	void OnAgroSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnCombatRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnAgroSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnCombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnLeftWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnRightWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	void LeftWeaponTrace();

	void DestroySelf();

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	void DoDamage(AShooterCharacter* Character);
	void SpawnBlood(AShooterCharacter* Character, FName SocketName);

	void StunCharacter(AShooterCharacter* Character);

protected: //variables
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UParticleSystem> ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Combat")
	float Health = 100.f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Combat")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Combat")
	FString HeadBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HealthBarDisplayTime = 4.f;
	
	FTimerHandle HealthBarTimer;

	FTimerHandle HitReactTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanHitReact = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float HitReactTimeMin = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float HitReactTimeMax = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TArray<FName> AttackMontageSectionNames;
	

	//Map to store hitNumber Widgets and their hit locations
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TMap<UUserWidget* , FVector> HitNumbers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float HitNumberDestroyTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USphereComponent> AgroSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bStunned = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float StunChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USphereComponent> CombatRangeSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bInAttackRange;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UBoxComponent> LeftWeaponCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UBoxComponent> RightWeaponCollision;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseDamage = 20.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FName LeftWeaponSocket = TEXT("FX_Trail_L_01");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FName RightWeaponSocket = TEXT("FX_Trail_R_01");

	
	/*AI Enemy*/

	UPROPERTY(EditAnywhere, Category = "Behavior Tree")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	//Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (MakeEditWidget = true))
	FVector PatrolPoint;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (MakeEditWidget = true))
	FVector PatrolPoint2;

	UPROPERTY()
	TObjectPtr<AEnemyController> EnemyController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float EnemyDestroyTime = 2.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void BulletHit_Implementation(FHitResult& HitResult, AController* ActorController, AActor* DamageCauserActor) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumbers(int32 Damage, FVector HitLocation, bool bHeadShot);

public:

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }
	FORCEINLINE bool IsDead() { return Health == 0.f; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

};
