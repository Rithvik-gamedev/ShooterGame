# ShooterGame

A high-fidelity Third-Person Shooter project built in **Unreal Engine** with C++. This project demonstrates complete gameplay loops, enemy AI, weapon mechanics, and integrates AAA-quality Paragon assets to deliver stunning visuals.

## Key Features

* **Advanced Weapon & Inventory System:** Fully implemented weapon classes (`Weapon.h`), ammunition management (`Ammo.h`), and item pick-up systems (`Item.h`).
* **Intelligent Enemy AI:** Custom enemy controllers (`EnemyController.cpp`) and behavior trees to actively track and engage the player, along with dynamic animation instances (`EnemyAnimInstance.cpp`).
* **High-Quality Assets:** Integrates Epic Games' premium Paragon characters and animations, featuring **Twinblast** (Player) and **Grux** (Enemy), alongside detailed Paragon environment props.
* **Explosive Interactions:** Dedicated explosive classes and environmental hazards.
* **Custom HUD & UI:** Fully functional heads-up display managing health, ammo count, crosshairs, and vital combat feedback.

## Technical Details

* **Engine:** Unreal Engine 5.4
* **Core Framework:** C++ and Blueprints
* **Architecture:** Component-based systems targeting `ShooterGameCharacter`, `Enemy`, and flexible `Interfaces`.
* **Version Control:** Managed with Git Large File Storage (Git LFS) for massive textures, 3D models (`.uasset`), and packed levels.

## Getting Started

1. Clone the repository (Ensure you have Git LFS installed).
2. Right-click the `.uproject` file and select **Generate Visual Studio project files**.
3. Open the `.sln` file and build the project in **Development Editor** configuration.
4. Launch the `.uproject` to open the Unreal Editor!

## Environments

Features beautifully constructed environments utilizing the `Scene_Warehouse` packed levels, `Polar` assets, and `SuperGrid` for excellent prototyping out-of-the-box.
