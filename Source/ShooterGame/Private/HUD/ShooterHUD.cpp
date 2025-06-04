// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ShooterHUD.h"

#include "Blueprint/UserWidget.h"
#include "Player/ShooterPlayerController.h"

AShooterHUD::AShooterHUD()
{

	
}

void AShooterHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetWorld())
	{
		
		PlayerController = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	}
	
	if (PlayerController && AmmoWidgetClass)
	{
		AmmoWidget = CreateWidget<UUserWidget>(PlayerController, AmmoWidgetClass);
		if (AmmoWidget)
		{
			AmmoWidget->AddToViewport();
		}
	}
}
