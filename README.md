# ShooterGame

A third-person action shooter built in **Unreal Engine 5.4** using C++ and Blueprints. The project covers a full gameplay loop including weapon management, inventory, intelligent enemy AI, and environmental hazards, all integrated with premium Paragon assets to deliver high-fidelity visuals.

---

## Table of Contents

- [Technical Overview](#technical-overview)
- [Gameplay Features](#gameplay-features)
- [Weapon System](#weapon-system)
- [Inventory and Item System](#inventory-and-item-system)
- [Enemy AI](#enemy-ai)
- [Animation System](#animation-system)
- [Explosives and Hazards](#explosives-and-hazards)
- [HUD and UI](#hud-and-ui)
- [Input System](#input-system)
- [Assets](#assets)
- [Getting Started](#getting-started)

---

## Technical Overview

| Property | Details |
|---|---|
| Engine | Unreal Engine 5.4 |
| Language | C++ with Blueprint integration |
| Input System | Enhanced Input System (`EnhancedInputComponent`) |
| AI Framework | Behavior Trees with custom `AIModule` controllers |
| Plugins | Motion Warping, Pose Search, Chooser, Modeling Tools |
| Version Control | Git with Git Large File Storage (LFS) |

---

## Gameplay Features

**Character States**

The player character operates through a state machine (`ECharacterState`) with the following states: Unoccupied, Shooting, Reloading, Equipping, and Stunned. State transitions govern which actions are permitted and which animations play.

**Movement**

- Full third-person movement with a spring arm and follow camera
- Crouch toggle with interpolated capsule height transition (`StandingCapsuleHalfHeight` 88 units to `CrouchingCapsuleHalfHeight` 55 units)
- Movement speed varies contextually: base speed, aim-walk speed (350), and crouch speed (270)
- Jump support

**Aiming**

- Hold-to-aim with smooth FOV interpolation from default to a configurable zoomed FOV (default 30 degrees)
- Separate sensitivity values for hip-fire and aimed states (`HipFireTurnRate`, `AimingTurnRate`)
- Camera sensitivity reduces automatically when aiming

**Combat**

- Line-trace based shooting from a dual-pass crosshair screen trace into a weapon barrel trace
- Directional hit reactions determined from the angle of the impact point relative to the character
- Stun system: both the player and enemies have a configurable stun chance on receiving a hit
- Health and death with dedicated death montages for both player and enemies
- Radial damage applied from explosive actors

**Physical Surface Detection**

`GetSurfaceType()` is exposed to Blueprints and allows footstep and impact sounds to respond to the underlying physical material.

---

## Weapon System

Weapons are data-table driven. The `FWeaponDataTable` struct stores per-weapon configuration including type, ammo type, fire rate, damage values, sounds, mesh, crosshair textures, material overrides, reload montage section, and clip bone name. This allows adding new weapons without modifying C++ code.

**Weapon Types (`EWeaponType`)**

- Submachine Gun
- Assault Rifle
- Pistol

**Ammo Types (`EAmmoType`)**

- 9MM
- Assault Rifle

**Weapon Mechanics**

- Automatic and semi-automatic fire modes, configurable per data table row
- Per-weapon fire rate timer (`ShootTimer`)
- Magazine system: `CurrentAmmo` and `MaxAmmo` per weapon instance
- Clip bone animation during reload via `GrabClip` and `ReleaseClip` Blueprint-callable functions
- Weapon throw on swap: the dropped weapon receives a physics impulse and falls before disabling collision
- Pistol slide displacement: a curve-driven animation moves the pistol slide back on each shot, with configurable `MaxSlideDistance` and `MaxRecoilRotation`
- Muzzle flash particle and fire sound are stored per-weapon
- Per-weapon custom crosshair textures (Middle, Top, Bottom, Left, Right)
- Headshot detection using a named head bone, with separate `HeadShotDamage` and body `Damage` values

---

## Inventory and Item System

**Item Base Class (`AItem`)**

All pickups derive from `AItem`, which provides:

- Area sphere for proximity detection and widget display
- Smooth interpolation animation when picked up: the item flies toward a scene component on the character before equipping
- Multiple interp target slots on the character (`InterpLocations` array) to support concurrent pick-up animations
- Pulsing glow material driven by a `UCurveVector` (`PulseCurve`)
- Dynamic material instance updated at runtime to reflect rarity colours
- Custom depth stencil for outline highlighting when in range

**Item Rarity (`EItemRarity`)**

Five rarity tiers: Damaged, Common, Uncommon, Rare, and Legendary. Each tier is configured through a data table (`FItemRarityTable`) defining glow colour, light colour, dark colour, star count, icon background, and a custom depth stencil value.

**Item States (`EItemState`)**

PickUp, EquipInterping, PickedUp, Equipped, Falling. State changes control physics, collision, and material visibility.

**Inventory**

- Six-slot inventory (`InventoryCapacity = 6`)
- Weapons occupy inventory slots and can be swapped through quick-slot keys (0 through 5)
- Delegates (`FEquipItemDelegate`, `FHighlightIconDelegate`) notify the UI of slot changes and trigger icon animations
- Auto-reload triggers when the magazine empties and the character is carrying matching ammo
- `AmmoMap` tracks carried ammo counts per `EAmmoType`

**Ammo Pickups (`AAmmo`)**

Dedicated ammo actor class. Picking up ammo increments the matching `AmmoMap` entry on the character.

---

## Enemy AI

Enemies use Unreal's Behavior Tree framework driven by a custom `AEnemyController`.

**Detection**

- Agro sphere (`AgroSphere`): when the player enters this range the enemy begins pursuit
- Combat range sphere (`CombatRangeSphere`): triggers melee attack sequences when the player is within close range

**Combat Behavior**

- Randomised attack montage sections played from `AttackMontageSectionNames`
- Left and right weapon box colliders (`LeftWeaponCollision`, `RightWeaponCollision`) enable per-hand melee hit detection
- Blood spawn at weapon socket locations on hit (`FX_Trail_L_01`, `FX_Trail_R_01`)
- Stun state (`bStunned`) interrupts the AI and plays a hit react montage with a configurable stun chance

**Health and Death**

- Health bar widget shown on hit and hidden after a configurable display time (`HealthBarDisplayTime`)
- Floating damage number widgets spawned at each hit location and removed after `HitNumberDestroyTime`
- Headshot detection via named bone, with differentiated damage output shown in the UI
- Death montage plays on health reaching zero, followed by a destruction timer (`EnemyDestroyTime`)

**Patrol**

Two configurable patrol points (`PatrolPoint`, `PatrolPoint2`) exposed as editable widgets in the viewport for easy placement.

**Hit Interface (`IHitInterface`)**

A C++ interface implemented by both `AEnemy` and `AExplosive`. The `BulletHit_Implementation` function provides a uniform contract for any actor that can be shot, decoupling the weapon trace logic from specific actor types.

---

## Animation System

**Player (`UShooterAnimInstance`)**

- Thread-safe animation update using `NativeThreadSafeUpdateAnimation`
- Turn-in-place: tracks yaw offset (`RootYawOffset`) to rotate the lower body while the upper body faces the aim direction
- Directional lean: computes `LeanYawDelta` from frame-to-frame rotation delta to blend lean animations during strafing
- States driven to the animation graph: `bIsInAir`, `bIsAccelerating`, `bIsAiming`, `bIsCrouching`, `Speed`, `Direction`, `WeaponType`
- FABRIK (Forward and Backward Reaching Inverse Kinematics) enabled on demand (`bShouldUseFABRIK`) for left-hand weapon grip

**Player Montages**

- GunFire, Reload (per-weapon section), Equip, HitReact (directional sections), Death

**Enemy (`UEnemyAnimInstance`)**

Custom animation instance for the enemy character driving hit reactions, attacks, and death.

---

## Explosives and Hazards

`AExplosive` implements `IHitInterface` so it reacts to bullet hits. On impact it applies radial damage to all actors within its overlap sphere, with a configurable maximum damage (`ExplosiveDamage`, default 100) and minimum falloff damage (`ExplosiveMinimumDamage`, default 50). An impact particle and sound play on detonation.

---

## HUD and UI

`AShooterHUD` extends `AHUD` and is referenced via `AShooterPlayerController`. The UI layer (implemented in Blueprints) displays:

- Health bar
- Current ammo and magazine count for the equipped weapon
- Inventory bar with six slots, icon highlights driven by the `FHighlightIconDelegate`, and equip animations driven by `FEquipItemDelegate`
- Dynamic crosshair that expands based on movement speed, airborne state, aiming state, and active fire (`CrosshairSpreadMultiplier` composed from five individual factors)
- Per-weapon crosshair textures loaded from the weapon data table
- Floating damage numbers on enemies, with a distinct visual for headshots

---

## Input System

Uses Unreal's Enhanced Input System with an `UInputMappingContext` assigned to the player controller. Bound actions:

| Action | Description |
|---|---|
| Move | WASD / left stick |
| Look | Mouse / right stick |
| Jump | Spacebar / face button |
| Shoot | Left mouse button / right trigger |
| Aim | Right mouse button / left trigger |
| Reload | R key |
| Crouch | C key |
| Equip / Interact | E key |
| Quick Slot 0-5 | Number keys 0 through 5 |

---

## Assets

The project integrates Epic Games' **Paragon** character assets:

- **Twinblast** as the playable character
- **Grux** as the enemy

Environment content includes warehouse packed levels (`Scene_Warehouse`), Polar-themed props, and SuperGrid geometry for prototyping levels. All large binary assets (`.uasset`, textures, skeletal meshes, packed levels) are tracked with **Git LFS**.

---

## Getting Started

1. Ensure **Git LFS** is installed before cloning: `git lfs install`
2. Clone the repository: `git clone <repository-url>`
3. Right-click `ShooterGame.uproject` and select **Generate Visual Studio project files**
4. Open `ShooterGame.sln` in Visual Studio and set the build configuration to **Development Editor**
5. Build the solution (Ctrl+Shift+B)
6. Launch `ShooterGame.uproject` to open the Unreal Editor

**Requirements**

- Unreal Engine 5.4
- Visual Studio 2022 with the **Game development with C++** workload
- Git with Git LFS
