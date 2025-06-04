#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9MM UMETA(DisplayName = "9MM"),
	EAT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),

	EAT_DefaultMAX UMETA(DisplayName = "DefaultMAX"),
};