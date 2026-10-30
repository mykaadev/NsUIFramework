# NsUIFramework Technical Documentation

## Architecture

Two modules.

| Module | Type | Content |
|---|---|---|
| `NsUIFramework` | Runtime | The subsystem, the screen interface, the selection modes, the input router, the layout, the cursor, and the world popups. |
| `NsUIFrameworkEditor` | Editor | The `ns.UI.Inspect` and `ns.UI.SmokeTest.Toggle` commands and the smoke test widgets. |

| Type | Role |
|---|---|
| `UNsUISubsystem` | Local player subsystem and tickable object. It owns the screen stack, the presentation, the input policy, and selection recovery. |
| `INsUIScreen` | The optional screen contract. |
| `UNsUIWidget` | A `UUserWidget` parent that implements `INsUIScreen`. |
| `FNsUIScreenConfig` | `Layer`, `bInteractive`, `Gameplay`, `SelectionMode`. |
| `FNsUISelectionRequest` | `Policy` (`Automatic` or `Logical`) and `PreferredTarget`. |
| `ENsUILayer`, `ENsUIGameplayPolicy`, `ENsUISelectionPolicy` | Layer, gameplay, and selection enums. |
| `UNsUISelectionMode` | Abstract base for selection. One instance for each screen. |
| `UNsUIUMGSelectionMode` | Native UMG focus and navigation for widget trees. |
| `UNsUIWorldSelectionMode` | Abstract base for directional selection of world candidates through screen projection. |
| `UNsUIActorSelectionMode`, `UNsUILocationSelectionMode` | World modes for weak actor lists and for id and position lists. |
| `FNsUISlateInputRouter` | A Slate `IInputProcessor`. It routes keys, analog, and pointer events to the subsystem. |
| `UNsUILayout`, `UNsUILayer` | Private. The layout holds one overlay panel for each layer. |
| `UNsUICursor` | Private. A non-hit-testable image that follows the selection. |
| `UNsUIDeveloperSettings` | Keys and cursor motion. |
| `UNsUIWorldPopupSubsystem` | Local player subsystem and conditional tickable object. It shows, moves, and pools floating texts over world points. |
| `UNsUIWorldPopupWidget` | Abstract `UUserWidget` parent of one popup. It holds the motion values and the optional `PopupText`. |
| `FNsUIWorldPopupEntry` | One active popup: the widget, the follow actor, the point or offset, and the age. |

### World popups

1. `ShowPopup` ends the oldest popup of the class when the class has `MaxActive` popups.
2. It takes a pooled widget of the class for the same player controller, or makes one.
3. It adds the widget to the player screen at `ViewportZOrder`, with the alignment (0.5, 1).
4. A popup on an actor that has popups already starts `StackHeight` higher for each of them.
5. Each tick moves the point up by `RiseSpeed`, applies the fade and the pop scale, and projects the point to the screen.
6. The popup hides beyond `MaxDistance` from the camera, behind the camera, and, with `bHideWhenBlocked`, when a visibility trace from the camera hits something. The trace ignores the pawn of the player and the follow actor.
7. At `Lifetime`, or when the follow actor goes away, the widget leaves the screen and goes back to the pool.

### Open flow

1. `OpenScreen` makes the widget for the local player controller and adds an owned entry with the context.
2. `EnsurePresentation` makes the layout and the cursor one time, and adds the layout to the game viewport.
3. The widget goes into the layer panel of its layer.
4. The subsystem calls `OnNsScreenOpened` with the context and a new selection mode instance.
5. `Reconcile` picks the active screen and calls `OnNsScreenActivated` and `OnNsScreenSuspended` as needed.
6. `ApplyInputPolicy` sets the input mode for the new active screen.

### Viewport discovery

The subsystem listens to viewport add and remove events and does one search at startup. A viewport widget that implements `INsUIScreen` becomes an external entry. External entries are not removed by the host.

### Input policy

| Active screen | Input mode | Mouse cursor |
|---|---|---|
| None | `FInputModeGameOnly` | Hidden |
| Gameplay allowed | `FInputModeGameAndUI`, no lock, cursor not hidden on capture | Shown |
| Gameplay blocked | `FInputModeUIOnly`, no lock | Shown |

Gameplay is allowed when the policy is `Allow`, or when it is `Automatic` and the layer is `Game`.

### Input routing

The router runs before normal Slate input:

- Confirm and Cancel keys go to `RouteAction`. The screen handlers run first, then the selection mode.
- Navigation keys and Tab or Shift+Tab go to `RouteNavigate`, unless the mode uses native navigation.
- Analog input becomes a direction, unless the mode uses native navigation.
- Pointer moves go to `ObservePointer`, so the mouse can select.
- Shift+F1 and app focus changes suspend the subsystem.

## Lifetime

- The subsystem ticks when the game is paused (`IsTickableWhenPaused` is true).
- Selection recovery runs after Slate events, never inside one.
- A selection mode instance stays alive while its screen is suspended.
- On world cleanup, the world popup subsystem drops its popups and pooled widgets of that world.
- On world cleanup, the subsystem removes its entries for that world. It also marks the input policy as dirty, so the next tick sets the input mode for the new player controller, also when no screen is active. Game code does not need its own `SetInputMode` call after a map change.

## Extension Points

| Point | Use |
|---|---|
| `INsUIScreen` | Any widget, C++ or Blueprint. |
| `UNsUISelectionMode` | C++ only (`NotBlueprintable`). Override `Restore`, `HasSelection`, `Navigate`, `Confirm`, `Cancel`, `GetFocusTarget`, `GetCursorPosition`, and more. |
| `UNsUIWorldSelectionMode` | Override `GetCandidateCount` and `GetCandidatePosition` for a new world candidate source. |
| `UNsUIDeveloperSettings` | Keys and cursor motion. |


## Dependencies

Engine modules only: Slate, UMG, and input modules. No other studio plugin.

## Performance

- The subsystem ticks each frame for cursor motion and recovery.
- `Reconcile` runs on stack changes, not every frame.
- World selection projects each candidate on navigation. Keep candidate lists small.
- The world popup subsystem ticks only while a popup shows. Each popup does one projection and, with `bHideWhenBlocked`, one line trace each frame.

## Networking

Local only. One subsystem for each local player.

## Testing

No automated tests. Use the smoke test in an editor build:

1. Run `ns.UI.SmokeTest.Toggle`.
2. Open a menu, then a modal. Make sure that only the modal gets input.
3. Close the modal. Make sure that the selection goes back to the menu.
4. Use the actor and location screens with a gamepad and with a mouse.
5. Run `ns.UI.Inspect` and read the stack and the recovery reason.

## Change Rules

- Every studio project uses this plugin. Keep `INsUIScreen` and `UNsUISubsystem` signatures stable.
- Keep the `ShowPopup` signature stable. Game code calls it.
- The layout class is fixed in code, so a project cannot change the cursor brush. A setting for it is a candidate feature.
- Device detection and prompt icons belong in `NsInput`. An optional integration module can join them.
