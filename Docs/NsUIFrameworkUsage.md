# NsUI Framework Navigation Overview

This guide explains the streamlined navigation model after removing focus targets. Each navigation behaviour owns its own state, keeping the subsystem lean while still exposing helpers for cursor placement and widget focus.

## Core Concepts
- **ENsUILayer** – Layered widget stacks (`Game`, `Menu`, `Modal`) managed by the subsystem.
- **UNsUISubsystem** – Creates layouts, tracks widget focus, syncs the retro cursor, and delegates navigation input to the active behaviour.
- **UNsUINavigationBehaviour** – Base class for navigation policies. Behaviours receive `OnEnter`, `OnExit`, `OnTick`, `OnNavigate`, `OnConfirm`, and `OnCancel` to drive custom flows.
- **Built-ins** – `UNsUIWidgetNavigationBehaviour` handles UMG focus; `UNsUIActorNavigationBehaviour` cycles through a supplied actor list; `UNsUILocationNavigationBehaviour` is a stub for location-based movement.

## Typical Flows
### Displaying Screens
```cpp
UNsUISubsystem::GetSubsystem()->PushWidgetToLayer(MyScreenClass, ENsUILayer::Menu);
```
Blueprints expose the same calls via **NsUI** → `Push` (hard or soft class), with `Pop`/`PopAll` to unwind stacks. Focus history per layer is restored automatically.

### Navigation and Input
- `Navigate` routes through the active navigation behaviour. Without an override, the subsystem uses widget spatial navigation.
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
Derive from `UNsUINavigationBehaviour` to implement bespoke logic (e.g., grid locations, radial menus). Use the subsystem helpers to:
- Move focus between widgets: `NavigateWidgets(Direction)`.
- Drive cursor placement: `UpdateCursorToWidget`, `UpdateCursorToActor`, or `UpdateCursorToLocation`.

## Blueprint Surface
- Screen stack: `Push`, `Pop`, `PopAll`, `ToggleDebugOverlay`.
- Behaviour control: `SetNavigationBehaviour`.

## Debugging
- `ToggleDebugOverlay` reveals layer stacks and focus history in-editor.
- The subsystem ticks via `FTickableGameObject`, so behaviours receive `OnTick` even on engine versions where `ULocalPlayerSubsystem` lacks a tick override.
