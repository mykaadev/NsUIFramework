# NsUI Framework Focus & Selection Overview

This document summarizes how to drive the retro cursor, focus targets, and pluggable selection behaviors in both C++ and Blueprint. It is intended as a fast reference for engineers and designers wiring up interactive flows that span UMG widgets and in-world elements.

## Core Concepts

- **ENsUILayer** – Screen strata (`Game`, `Menu`, `Modal`) used to organize stacks of widgets with predictable ordering and input routing.
- **FNsUIFocusTarget** – Unified focus descriptor that can reference a UWidget, an AActor, or a raw world-space location. The subsystem projects world positions to screen space so the retro cursor can follow non-widget targets.
- **UNsUISubsystem** – Local-player subsystem that owns layout creation, cursor syncing, widget stack management, and selection behaviors.
- **UNsUISelectionBehavior** – Swappable behavior object that receives `OnEnter`, `OnExit`, `OnTick`, `OnConfirm`, `OnHover`, and `OnCancel` callbacks for custom interaction logic.

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

- The subsystem keeps `CurrentFocus` as an `FNsUIFocusTarget`. Calling navigation/confirm/cancel routes input through Slate and/or the active selection behavior.
- Cursor position is updated via `UpdateCursorToCurrentFocus()`, which projects widgets, actors, or locations into screen space.
- `LastInputSource` is tracked to hide the retro cursor when mouse input is active and show it for keyboard/gamepad navigation.

### Selection Sessions Over World Targets

Use selection sessions to traverse arbitrary actors or locations while reusing the retro cursor and behavior callbacks.

```cpp
// Build targets and behavior, then start a session.
TArray<AActor*> Actors = {/* ... */};
TArray<FVector> Locations = {/* ... */};
UNsUISelectionBehavior* Behavior = NewObject<UMySelectionBehavior>(GetTransientPackage());
UNsUISubsystem::GetSubsystem()->BeginSelectionSession(Behavior, Actors, Locations);
```

Blueprints expose the same calls under the **NsUI|Selection** category:
- `BeginSelectionSession`
- `EndSelectionSession`
- `FocusNextTarget`
- `FocusPreviousTarget`
- `SetSelectionBehavior`

### Authoring a Selection Behavior

Create a Blueprint derived from `UNsUISelectionBehavior` and implement the native events:

- **OnEnter** – initialize state or spawn previews when the behavior is activated.
- **OnExit** – clean up spawned actors/widgets when the behavior is removed.
- **OnTick** – per-frame updates (drag outlines, timers, aim assists). Only called while active.
- **OnHover** – invoked whenever focus moves to a new `FNsUIFocusTarget`. Use `Target.Kind` to branch between widget, actor, or location handling.
- **OnConfirm / OnCancel** – handle user intent; you may end the session or keep it alive.

### Driving Selection From Input

- Call `Confirm` or `Cancel` on the subsystem (or map to input actions) to forward to the active behavior. If no behavior is active, the calls fall back to Slate’s default confirm/cancel handling.
- Navigation (`Navigate`) attempts Slate focus first, then falls back to spatial navigation through registered widgets.

## Debugging Tips

- Use `ToggleDebugOverlay` (Blueprint or C++) to display focus history, layer stacks, and cursor data in-editor.
- The subsystem ticks via `FTickableGameObject`, so behaviors receive `OnTick` even on engine versions where `ULocalPlayerSubsystem` lacks a tick override.
- `FNsUIFocusTarget::DebugName` can be set when constructing focus entries to label targets in logs or overlays.

## Key API Surface (Blueprint)

- **Screen Stack**: `Push`, `Pop`, `PopAll`, `ToggleDebugOverlay`
- **Selection**: `SetSelectionBehavior`, `BeginSelectionSession`, `EndSelectionSession`, `FocusNextTarget`, `FocusPreviousTarget`

## Key API Surface (C++)

- `UNsUISubsystem::PushWidgetToLayer`, `PopWidgetFromLayer`, `PopAllWidgetsFromLayer`
- `UNsUISubsystem::SetSelectionBehavior`
- `UNsUISubsystem::BeginSelectionSession`, `EndSelectionSession`
- `UNsUISubsystem::FocusNextTarget`, `FocusPreviousTarget`
- `UNsUISubsystem::Confirm`, `Cancel`, `Navigate`

With these calls you can compose experiences that seamlessly transition focus between menus and world objects while keeping the retro cursor synchronized.
