// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/Enemy.h"

AEnemyController::AEnemyController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>("Blackboard Component");
	check(BlackboardComponent);

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTree Component");
}

void AEnemyController::OnPossess(APawn* InPawn)
{
	if (InPawn == nullptr) return;
	Super::OnPossess(InPawn);

	if (AEnemy* Enemy = Cast<AEnemy>(InPawn))
	{
		if (Enemy->GetBehaviorTree())
		{
			BlackboardComponent->InitializeBlackboard(*Enemy->GetBehaviorTree()->BlackboardAsset);
		}
	}
	
}

