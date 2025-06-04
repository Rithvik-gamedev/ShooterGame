// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

class AEnemy;
/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	

protected:

	void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Movement")
	AEnemy* Enemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Movement")
	float Speed;
	
};
