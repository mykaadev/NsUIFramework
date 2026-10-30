# NsUIFramework User Guide

## Purpose

NsUIFramework owns UI interaction in every studio project. It opens and closes screens on three layers, decides which screen gets input, moves the selection with keyboard, mouse, and gamepad, and routes Confirm and Back. It also sets the input mode and the mouse cursor.

NsUIFramework also shows floating texts over world points, for example "+3 coins". It does not style widgets, animate screens, or show input prompt icons. Use the UI guides for style, `NsTween` for motion, and `NsInput` for prompt icons.

## Setup

1. Add `NsUIFramework` to the `Plugins` list of the `.uproject` file.
2. For C++, add `"NsUIFramework"` to the module dependencies in `Build.cs`.
3. Optional: open Project Settings, Game, "NsUI Framework", and change the keys.

## Quick Start

### 1. Make a screen

Make a widget with `UNsUIWidget` as the parent. Or implement the `NsUIScreen` interface on any `UUserWidget`, without a change of parent.

### 2. Open and close it

One subsystem exists for each local player.

```cpp
UNsUISubsystem* const UI = LocalPlayer->GetSubsystem<UNsUISubsystem>();
UUserWidget* const Screen = UI->OpenScreen(PauseMenuClass, Context, ENsUILayer::Menu);
UI->CloseScreen(Screen);
UI->CloseAll(ENsUILayer::Modal);
```

The subsystem owns screens that `OpenScreen` makes. The context object stays alive until the screen closes.

### 3. Handle Confirm and Back

Override `HandleNsScreenConfirm` and `HandleNsScreenCancel`. Return true when the screen uses the action.

## Screen Contract

All events are optional.

| Event | Use |
|---|---|
| `GetNsScreenConfig` | Return the layer, `bInteractive`, the gameplay policy, and the selection mode class. |
| `OnNsScreenOpened` | Receive the context and the selection mode instance for this screen. |
| `OnNsScreenActivated` | The screen gets input. `bResumed` is true when it comes back after another screen. |
| `OnNsScreenSuspended` | The screen loses input but stays open. |
| `OnNsScreenClosed` | The screen is closed. Release resources. |
| `GetNsScreenSelection` | Return automatic selection, a preferred target, or logical input. |
| `HandleNsScreenConfirm` | Return true to use Confirm before the native control gets it. |
| `HandleNsScreenCancel` | Return true to use Back before the default close. |

## Common Tasks

### Choose the layer

| Layer | Use | Gameplay input with `Automatic` |
|---|---|---|
| `Game` | HUD parts that take input during play. | Allowed |
| `Menu` | Menus such as pause or settings. | Blocked |
| `Modal` | Popups such as confirmations. | Blocked |

Only the highest interactive screen gets input: Modal, then Menu, then Game. In one layer, the last opened screen has priority.

### Let gameplay continue under a screen

Set the gameplay policy to `Allow`, for example for dialogue. With `Allow`, the analog sticks go to gameplay. The digital directions still move the UI selection.

### Show a passive screen

Set `bInteractive` to false for prompts, toasts, and other screens that take no input.

### Set the first focus

Return a `PreferredTarget` from `GetNsScreenSelection`. A preferred target does not replace a valid selection that the player made.

### Select actors or world points

1. Set the selection mode class to `UNsUIActorSelectionMode` or `UNsUILocationSelectionMode`.
2. In `OnNsScreenOpened`, call `SetCandidates` on the mode instance.
3. Read `GetSelectedActor` or `GetSelectedId` on Confirm.

Location ids must be unique and not empty. Candidate order breaks ties.

### Use a widget made with AddToViewport

The widget takes part only if it implements `INsUIScreen`. Its config sets its layer. The code that made it must also remove it. `CloseScreen` and `CloseAll` do not remove it.

### Show a floating text in the world

1. Make a widget Blueprint with `UNsUIWorldPopupWidget` as the parent. Add a text block named `PopupText`.
2. In the class defaults, set the motion: `Lifetime`, `RiseSpeed`, `MaxDistance`, the fades, `PopScale`, `MaxActive`, and `StackHeight`.
3. Call the subsystem of the local player:

```cpp
UNsUIWorldPopupSubsystem* const Popups = LocalPlayer->GetSubsystem<UNsUIWorldPopupSubsystem>();
Popups->ShowPopup(PopupClass, FText::FromString(TEXT("+3 coins")), FVector(0.f, 0.f, 100.f), FLinearColor::Yellow, TargetActor);
```

With a follow actor, the location is an offset from that actor. Without one, the location is a world point. The popup is local to the player who calls it. A game that wants every player to see an event calls `ShowPopup` on every machine, for example from a replicated event.

| Setting | Default |
|---|---|
| `Lifetime` | 1.2 s |
| `RiseSpeed` | 45 cm/s |
| `MaxDistance` | 3,000 cm |
| `FadeInSeconds`, `FadeOutSeconds` | 0.1 s, 0.4 s |
| `PopScale`, `PopSeconds` | 1, 0.2 s |
| `bHideWhenBlocked` | true |
| `MaxActive` | 4 for each class and player |
| `StackHeight` | 30 cm |
| `ViewportZOrder` | -100, under the screens |

`OnPopupStarted` lets the Blueprint react to a new popup, for example to show an icon.

## Rules

- Open and close interactive screens through `UNsUISubsystem`.
- Do not call `SetInputMode` or set `bShowMouseCursor` from game code. The subsystem sets them each time the active screen changes.
- Handle Confirm and Back in the screen events. Do not bind those keys in each widget.
- Extend `UNsUISelectionMode` in C++ for a new kind of selection. Blueprint cannot extend it.

## Settings

Project Settings, Game, "NsUI Framework":

| Setting | Default |
|---|---|
| `ConfirmKeys` | Enter, Space Bar, gamepad Accept |
| `CancelKeys` | Escape, gamepad Back |
| `NavigateUpKeys` | Up, D-pad Up |
| `NavigateDownKeys` | Down, D-pad Down |
| `NavigateLeftKeys` | Left, D-pad Left |
| `NavigateRightKeys` | Right, D-pad Right |
| `NavigateNextKeys` | Tab |
| `NavigatePreviousKeys` | None. Shift+Tab also moves back. |
| `CursorVelocity` | 20 |
| `CursorLerpCurve` | Cubic from (0, 0) to (1, 1) |

## Debug

Editor builds only:

| Command | Effect |
|---|---|
| `ns.UI.Inspect` | Opens a read-only view of the screen stack, the selection, and the recovery reason. |
| `ns.UI.SmokeTest.Toggle` | Opens or closes the smoke test screen with menus, modals, viewport discovery, dynamic controls, actors, and locations. |

In the smoke test, Confirm removes the selected smoke actor. Cancel closes a world selection screen.

## Limits

- Selection recovery waits until after the Slate event. Editor interaction, Shift+F1, and a lost app focus pause recovery.
- The mouse cursor shows whenever a screen is active, also on the `Game` layer.
- The UI cursor follows the selected target. Logical input has no cursor. The cursor brush is not a project setting today.
- No device detection and no prompt icons. Use `NsInput` for prompts.
- The subsystem ignores preview worlds.
