# Our Last Sync - Source Code
A 2-player co-op sci-fi puzzle adventure built in Unreal Engine 5.5, currently in development. Players control robots escaping a failing space station headed toward a black hole. Space anomalies - black hole and white hole, affect physics objects and environment as players work together to survive.

This repository contains the C++ source code for the project. The full project is not included.

# Source Files
## Core Gameplay
```SpaceAnomaly```: Handles both black hole and white hole behaviour in a single actor, switched at runtime via an ```EAnomalyType``` enum.
+ Black Hole: pulls overlapping physics objects using inverse-square force with a photon sphere multiplier closer to the center. Objects that cross the Schwarzschild radius are hidden, disabled, and stored in GameState for the white hole to use.
+ White Hole: retrieves stored actors from GameState on a timer and ejects them outward with a randomized impulse.
+ Runs authority-only on tick. ```bReplicates = true``` so the actor spawns on all clients.

```BaseRobot```: Player character base class. Handles movement and camera via Enhanced Input, line trace-based anomaly spawning with a server RPC, and a physics-based pickup/drop system that checks for ```Interact``` interface.

## Multiplayer
```MultiplayerSubsystem```: ```GameInstanceSubsystem``` for session management. Handles session creation, destruction, joining, and Steam invite acceptance. Detects whether Steam is running on initialization and sets ```IsSteamClientRunning``` accordingly, falls back to LAN if Steam isn't available. Delegates are Blueprint-exposed for UI binding.

```MainGameMode```: Handles player login, assigns a robot class per player index, and picks a tagged player start for each player. Also creates the second local player for split-screen when running in local mode.

```SelectedRobotIndex``` is assigned before ```Super::PostLogin``` so the correct pawn class is available when the engine spawns the pawn. Calling Super first caused both players to receive the same robot class.

```SPlayerController```: Creates the player widget and binds to the dialogue state delegate on ```SGameState```. Widget is only created on the local controller.

```CoreGameInstance```: Stores the current multiplayer mode (Local or Online) and handles server travel for local play.

## State & UI
```SGameState```: Manages replicated dialogue state and the ```StoredActors``` array shared between the black hole and white hole. ```StoredActors``` is server-only, both systems that use it are authority-only so no replication is needed.

```DialogueWorldSubsystem```: ```WorldSubsystem``` that manages dialogue playback. Called by a Blueprint trigger with a DataTable, it builds on ordered cache of ```FDialogueStruct``` entries and steps through them on a timer - pushing each entry to ```SGameState``` to replicate to clients. Runs authority-only.

Dialogue state changes are broadcast to local controllers via a multicast delegate.

```PlayerWidget```: Base widget class with ```BlueprintImplementableEvent``` functions for showing and hiding dialogue. Visual implementation is in Blueprint.

```DialogueStruct```: ```FTableRowBase``` struct for DataTable-driven dialogue. Stores order, duration, text, and audio.

## Interfaces & Utilities
```Interact```: UInterface used to mark actors as interactable for the pickup system in ```BaseRobot```.

## Notes
+ Project is in active development. Some systems are incomplete or subject to change.








