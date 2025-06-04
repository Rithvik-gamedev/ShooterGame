// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

class AShooterPlayerController;
/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AShooterHUD : public AHUD
{
	GENERATED_BODY()
public:
	AShooterHUD();
protected:

	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AShooterPlayerController> PlayerController;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UUserWidget> AmmoWidgetClass;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<UUserWidget> AmmoWidget;
};
