// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class UBehaviorTreeComponent;
/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:

	AEnemyController();
	
	virtual void OnPossess(APawn* InPawn) override;

protected:

	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

public:

	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
};
