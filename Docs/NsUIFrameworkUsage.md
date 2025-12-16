# NsUI Framework Focus & Navigation Overview

This document summarizes how to drive the retro cursor, focus targets, and pluggable navigation behaviours in both C++ and Blueprint. It is intended as a fast reference for engineers and designers wiring up interactive flows that span UMG widgets and in-world elements.

## Core Concepts

- **ENsUILayer** – Screen strata (`Game`, `Menu`, `Modal`) used to organize stacks of widgets with predictable ordering and input routing.
- **FNsUIFocusTarget** – Unified focus descriptor that can reference a UWidget, an AActor, or a raw world-space location. The subsystem projects world positions to screen space so the retro cursor can follow non-widget targets.
- **UNsUISubsystem** – Local-player subsystem that owns layout creation, cursor syncing, widget stack management, and navigation behaviours.
- **UNsUINavigationBehaviour** – Swappable behavior object that receives `OnEnter`, `OnExit`, `OnTick`, `OnNavigate`, `OnConfirm`, `OnHover`, and `OnCancel` callbacks for custom interaction logic.
- **Built-in behaviours** – `UNsUIWidgetNavigationBehaviour` (UMG widgets), `UNsUIActorNavigationBehaviour` (actors only), and `UNsUILocationNavigationBehaviour` (world points only). Navigation mode enums are deprecated in favor of explicitly passing a behaviour instance.

## Typical Flows

### Displaying Screens

```cpp
// C++ (synchronous class)
UNsUISubsystem::GetSubsystem()->PushWidgetToLayer(MyScreenClass, ENsUILayer::Menu);

// Blueprint
// Call NsUI -> Push (SoftClass) or Push (Class) with your widget asset and layer.
```

Use `Pop` or `PopAll` to remove screens. The subsystem tracks focus history per layer so backing out restores prior focus automatically.

### Managing Focus and the Retro Cursor

- The subsystem keeps `CurrentFocus` as an `FNsUIFocusTarget`. Calling navigation/confirm/cancel routes input through Slate and/or the active navigation behaviour.
- Cursor position is updated via `UpdateCursorToCurrentFocus()`, which projects widgets, actors, or locations into screen space.
- `LastInputSource` is tracked to hide the retro cursor when mouse input is active and show it for keyboard/gamepad navigation.

### Selection Sessions Over World Targets

Use selection sessions to traverse arbitrary actors or locations while reusing the retro cursor and behavior callbacks.

```cpp
// Build targets and behavior, then start a session.
TArray<AActor*> Actors = {/* ... */};
TArray<FVector> Locations = {/* ... */};
UNsUINavigationBehaviour* Behaviour = NewObject<UNsUIActorNavigationBehaviour>(GetTransientPackage());
UNsUISubsystem::GetSubsystem()->BeginSelectionSession(Behaviour, Actors, Locations);
```

Blueprints expose the same calls under the **NsUI|Selection** category:
- `BeginSelectionSession`
- `EndSelectionSession`
- `FocusNextTarget`
- `FocusPreviousTarget`
- `SetNavigationBehaviour`

### Authoring a Selection Behavior

Create a Blueprint derived from `UNsUINavigationBehaviour` (or subclass the provided `UNsUIWidgetNavigationBehaviour`, `UNsUIActorNavigationBehaviour`, or `UNsUILocationNavigationBehaviour`) and implement the native events:

- **OnEnter** – initialize state or spawn previews when the behavior is activated.
- **OnExit** – clean up spawned actors/widgets when the behavior is removed.
- **OnTick** – per-frame updates (drag outlines, timers, aim assists). Only called while active.
- **OnNavigate** – handle directional input. The default implementation cycles selection targets; the built-in behaviours focus widgets, actors, or locations respectively.
- **OnHover** – invoked whenever focus moves to a new `FNsUIFocusTarget`. Use `Target.Kind` to branch between widget, actor, or location handling.
- **OnConfirm / OnCancel** – handle user intent; you may end the session or keep it alive.

### Driving Selection From Input

- Call `Confirm` or `Cancel` on the subsystem (or map to input actions) to forward to the active navigation behaviour. If no behaviour is active, the calls fall back to Slate’s default confirm/cancel handling.
- Navigation (`Navigate`) routes through the active navigation behaviour; without one, NsUI falls back to widget spatial navigation.

## Debugging Tips

- Use `ToggleDebugOverlay` (Blueprint or C++) to display focus history, layer stacks, and cursor data in-editor.
- The subsystem ticks via `FTickableGameObject`, so behaviors receive `OnTick` even on engine versions where `ULocalPlayerSubsystem` lacks a tick override.
- `FNsUIFocusTarget::DebugName` can be set when constructing focus entries to label targets in logs or overlays.

## Key API Surface (Blueprint)

- **Screen Stack**: `Push`, `Pop`, `PopAll`, `ToggleDebugOverlay`
- **Selection**: `SetNavigationBehaviour`, `BeginSelectionSession`, `EndSelectionSession`, `FocusNextTarget`, `FocusPreviousTarget`

## Key API Surface (C++)

- `UNsUISubsystem::PushWidgetToLayer`, `PopWidgetFromLayer`, `PopAllWidgetsFromLayer`
- `UNsUISubsystem::SetNavigationBehaviour`
- `UNsUISubsystem::BeginSelectionSession`, `EndSelectionSession`
- `UNsUISubsystem::FocusNextTarget`, `FocusPreviousTarget`
- `UNsUISubsystem::Confirm`, `Cancel`, `Navigate`

With these calls you can compose experiences that seamlessly transition focus between menus and world objects while keeping the retro cursor synchronized.
