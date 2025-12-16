# NsUI Framework Navigation Overview

This guide explains the streamlined navigation model after removing focus targets. Each navigation behaviour owns its own state, keeping the subsystem lean while still exposing helpers for cursor placement and widget focus.

## Core Concepts
- **ENsUILayer** – Layered widget stacks (`Game`, `Menu`, `Modal`) managed by the subsystem.
- **UNsUISubsystem** – Creates layouts, keeps the cursor alive, and forwards navigation/confirm/cancel/hover/focus changes directly to the active behaviour.
- **UNsUINavigationBehaviour** – Base class for navigation policies. Behaviours receive `OnEnter`, `OnExit`, `OnTick`, `OnNavigate`, `OnConfirm`, `OnCancel`, plus widget stack notifications (`OnWidgetAdded/Removed/LayerCleared`) and Slate focus/hover hooks.
- **Built-ins** – `UNsUIWidgetNavigationBehaviour` owns UMG focus history and spatial navigation; `UNsUIActorNavigationBehaviour` cycles through a supplied actor list; `UNsUILocationNavigationBehaviour` is a stub for location-based movement.

## Typical Flows
### Displaying Screens
```cpp
UNsUISubsystem::GetSubsystem()->PushWidgetToLayer(MyScreenClass, ENsUILayer::Menu);
```
Blueprints expose the same calls via **NsUI** → `Push` (hard or soft class), with `Pop`/`PopAll` to unwind stacks. The active navigation behaviour is notified whenever screens are added or removed.

### Navigation and Input
- `Navigate` routes through the active navigation behaviour (UMG, actors, world locations, or anything custom).
- `Confirm` and `Cancel` first hit the active behaviour; if none is set, they forward to Slate (Enter/Escape).
- The subsystem installs a Slate navigation wrapper so arrows only route through NsUI when the framework is active.

### Cursor Sync
- UMG focus changes call `UpdateCursorToWidget` so the retro cursor follows the focused widget.
- Actor behaviours can call `UpdateCursorToActor` each tick (or on change) to keep the cursor aligned with moving actors.
- Use `UpdateCursorToLocation` for custom behaviours that operate on world-space points.

### Actor Navigation Behaviour
```cpp
UNsUIActorNavigationBehaviour* Behaviour = NewObject<UNsUIActorNavigationBehaviour>(this);
Behaviour->SetActors(MyActorArray);
Subsystem->SetNavigationBehaviour(Behaviour);
```
`SetActors` stores the list and focuses the first entry. `OnNavigate` cycles forward/back, `OnTick` keeps the cursor projected to the current actor, and `OnConfirm`/`OnCancel` are intentionally empty for game-specific overrides.

### Custom Behaviours
Derive from `UNsUINavigationBehaviour` to implement bespoke logic (e.g., grid locations, radial menus). Use the subsystem helpers to project positions for your cursor (`GetWidgetScreenPosition`, `GetActorScreenPosition`, `GetLocationScreenPosition`) and to move the retro cursor (`UpdateCursorToWidget/Actor/Location`). Behaviours receive widget stack notifications so they can maintain their own focus state without bloating the subsystem.

## Blueprint Surface
- Screen stack: `Push`, `Pop`, `PopAll`, `ToggleDebugOverlay`.
- Behaviour control: `SetNavigationBehaviour`.

## Debugging
- `ToggleDebugOverlay` reveals layer stacks in-editor.
- The subsystem ticks via `FTickableGameObject`, so behaviours receive `OnTick` even on engine versions where `ULocalPlayerSubsystem` lacks a tick override.
