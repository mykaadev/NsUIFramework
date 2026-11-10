# NsUIFramework Technical Documentation

## Architecture

### Modules

| Module | Type | Content |
|---|---|---|
| `NsUIFramework` | Runtime | The UI subsystem, the widget stack, the Slate root, the UI input, the focus tracker, the selection modes, the selection cursor, the HUD, the function library, the settings, the world popups, `ns.UI.Inspect`, and the automation tests |
| `NsUIFrameworkTween` | Runtime | `UNsUITweenCursorWidget`: a cursor that glides with NsTween. `WITH_NSTWEEN` is 0 without NsTween, and the cursor then moves at once |
| `NsUIFrameworkInput` | Runtime | `UNsUIInputBridgeSubsystem`: tells NsInput the device that the player uses in the UI. `WITH_NSINPUT` is 0 without NsInput, and the module then does nothing |
| `NsUIFrameworkEditor` | Editor | The NsUI Inspector tab |

The plugin content has the default UI input: `Content/Input/IMC_NsUI` and one `IA_NsUI_` action for each UI action. The actions do not consume input.

### Main classes

| Class | Job |
|---|---|
| `UNsUISubsystem` | A `ULocalPlayerSubsystem` with no tick. The only writer of the open widgets, the input mode, the mouse cursor, and the UI navigation config of its player |
| `FNsUIWidgetStack` | Private, plain C++. Holds one `FNsUIWidgetEntry` for each open widget and picks the active one: the highest layer that takes input, then the newest |
| `SNsUIRoot` | Private Slate widget in the game viewport of the player. One `SOverlay` for each layer. Gets the keys, the directions, and the pointer events that no control used |
| `SNsUIWidgetSlot` | Private Slate box around each open widget. Owns the hit test of the widget and keeps navigation inside it |
| `FNsUIInputActions` | Private. Reads the keys of the UI actions from Enhanced Input and owns the Slate navigation config of the player while a widget takes input |
| `FNsUIFocusTracker` | Private. Watches the Slate focus of the player, owns the suspended state, and runs a focus restore on the next frame |
| `INsUIWidgetInterface` | The optional contract of a widget. Every event is a `BlueprintNativeEvent` with a default |
| `UNsUIWidget` | Abstract `UUserWidget` parent that implements the interface. `RemoveFromParent` closes the widget through the subsystem |
| `UNsUISelectionMode` | Abstract base of the selection of things that are not controls. `UNsUIWorldSelectionMode`, `UNsUIActorSelectionMode`, and `UNsUILocationSelectionMode` select world positions |
| `UNsUICursorWidget` | Abstract Blueprintable selection cursor. Follows a focused control or a selection mode |
| `UNsUISettings` | `UDeveloperSettings`, `Config=Game`, category Plugins |
| `ANsUIHUD`, `UNsUIFunctionLibrary` | Blueprint entry points |
| `UNsUIWorldPopupSubsystem`, `UNsUIWorldPopupWidget` | Floating texts over world points |

### Input flow

The framework routes UI input the Slate way: a key goes to the focused control first, and the framework acts only on what no control used.

1. A key goes down. `SNsUIRoot::OnPreviewKeyDown` runs first, because the root is the top of the path. It reports the device and, for Confirm only, calls `HandleConfirm`. It skips this while a text box has the focus (keyboard keys only) or the console is open.
2. The focused control gets the key. A text box keeps the keys that it types. A button uses Confirm through the Slate navigation config, which maps the UI keys to Accept and Back.
3. A key that no control used bubbles to `SNsUIRoot::OnKeyDown`. While a text box has the focus, the root ignores keyboard keys, because a text box types some keys (such as Backspace) in the character event and leaves their key down unhandled. Back goes to `HandleBack`, then to the selection mode, then closes the widget. The tab actions go to `HandleTab`.
4. A key that the root does not use bubbles on to the game viewport. With `UIOnly` the player controller ignores input (`FInputModeUIOnly`), but the console still gets its keys. With `UIAndGame` the game gets the key.
5. A direction becomes a Slate navigation event. `SNsUIWidgetSlot::OnNavigation` calls `HandleNavigate`, then the selection mode. If one uses it, the slot returns a custom reply with no target, so the focus stays. If not, the slot returns Stop, so Slate moves the focus inside the widget only.
6. `GameFirst`: the game view keeps the focus. An Enhanced Input component on the player controller gives the Confirm and Back actions to the handlers.

### Data ownership

Each piece of data has one owner, and only the owner writes it.

| Data | Owner and writer | Rebuilt |
|---|---|---|
| Open widgets, their context, config, and selection mode | `FNsUIWidgetStack`, through `OpenWidget`, `AddExternalWidget`, `CloseWidget`, `FinishCloseWidget`, `RemoveExternalWidget`, `CloseAll`, `RefreshConfig`, and world cleanup | Config at open and on `RefreshConfig` |
| Lifetime and placement of an external widget | The system that shows it. An external entry has no slot, and the framework never writes its Slate | None |
| Active widget | `FNsUIWidgetStack` | On open, close, and `RefreshConfig` |
| Hit test and navigation boundary | `SNsUIWidgetSlot` | When the active widget changes. The framework never writes the visibility or the navigation metadata of a widget |
| Input mode and mouse cursor | `UNsUISubsystem::ApplyInputPolicy` | On each active change, pointer mode change, and new player controller |
| Navigation config of the player | `FNsUIInputActions`. It starts from the engine default keys and adds the UI action keys | When Enhanced Input rebuilds its mappings |
| Focus | Slate. The framework writes it in `ApplyFocus`, `FocusChild`, and hover | Restore on the next frame when the focus leaves the active widget |
| Last focused control | The entry of the widget, written in `HandleFocusMoved` and `FocusChild` | None |
| Suspended state | `FNsUIFocusTracker` | On a focus change and on app activation |
| World candidates | The widget that calls `SetCandidates` | Screen positions are projected when needed |
| Cursor position | `UNsUICursorWidget` | Each frame while it shows, from the target geometry |
| Popups and pool | `UNsUIWorldPopupSubsystem` | Each frame while a popup shows |

The root and the focus tracker call the subsystem only through delegates that the subsystem binds to its private functions. No class is a `friend` of another.

## Lifetime

- `UNsUISubsystem::Initialize` makes the stack, the input actions, and the focus tracker, and binds the world cleanup and the Enhanced Input mapping rebuild.
- `PlayerControllerChanged` adds the UI mapping context to the player and applies the input policy.
- The first `OpenWidget` makes the root and adds it to the viewport of the player. It also makes the selection cursor when the settings name one.
- `AddExternalWidget` adds an entry with no slot, for a widget that another system shows. Only `GameFirst` and `Passive` are allowed, because the framework cannot give focus to a widget outside its root. `RemoveExternalWidget` removes the entry.
- `CloseWidget` asks `BeginClose`. A widget in its outro stays in the stack, gets no input, and leaves on `FinishCloseWidget`.
- World cleanup removes the widgets of that world and the cursor. When no widget is left, it removes the root, because a map change removes all viewport widgets.
- `Deinitialize` calls `OnClosed` on every widget, removes the root and the cursor, and gives back the navigation config.
- Pause: the subsystem has no tick, so pause changes nothing. Input still works while the game is paused. World popups pause with the game.
- Focus: Slate sends its focus event before the focus moves, so `FNsUIFocusTracker` reports the move on the next frame. A widget with no control that can take the focus leaves the focus on the root. `RestoreFocus`, or the next key on the root, tries the focus again.
- Console: the engine moves the focus to the game viewport while the console is open. The framework does not restore the focus during that time.

## Extension Points

| Point | Use |
|---|---|
| `INsUIWidgetInterface` | Implement it on any widget, in C++ or Blueprint |
| `UNsUISelectionMode` | Subclass in C++ for a new kind of selection. Override `Navigate`, `PickAt`, `Confirm`, `Cancel`, `GetSelectionPosition`, and `Activate` |
| `UNsUIWorldSelectionMode` | Override `GetCandidateCount` and `GetCandidatePosition` for a new world candidate source |
| `UNsUICursorWidget` | Make a Blueprint child for the art. Implement `OnCursorMoved` to animate in Blueprint, or override `MoveCursor` in C++ |
| `UNsUIWorldPopupWidget` | Make a Blueprint child. Implement `OnPopupStarted` |
| `ANsUIHUD` | Subclass in Blueprint or C++ |
| `UNsUISubsystem::OnDeviceUsed` | A native event for device integrations, such as the NsInput module |

## Dependencies

| Dependency | Kind | Why |
|---|---|---|
| Core, CoreUObject, Engine, InputCore, SlateCore, UMG, DeveloperSettings, EnhancedInput | Public, `NsUIFramework` | Public headers use UObjects, local player subsystems, keys, Slate enums, UMG widgets, settings, and input actions |
| Slate | Private, `NsUIFramework` | The root widget, the focus, and the navigation config |
| EnhancedInput | Required engine plugin | The UI keys |
| NsTween | Optional studio plugin | `NsUIFrameworkTween` |
| NsInput | Optional studio plugin | `NsUIFrameworkInput` |

## Performance

- `UNsUISubsystem` does not tick. Work happens on input, focus change, open, and close.
- A focus change walks the Slate parents of the focused widget and the UMG tree of its user widget once.
- A mouse move over the UI finds the control under the pointer once, for hover focus.
- World selection projects the candidates on a direction, a pointer move, and each frame of the cursor.
- The selection cursor ticks only while it shows.
- Each world popup does one projection and, with `bHideWhenBlocked`, one line trace each frame. The popup subsystem ticks only while a popup shows. Popup widgets go back to a pool.

## Networking

The plugin does not replicate. Each local player has its own subsystems, and each machine shows its own UI.

## Testing

Automation tests, in `Private/Tests/`, run in the editor:

| Test | Checks |
|---|---|
| `NsUIFramework.Stack.Order` | Layer order, newest first in a layer, the earlier active widget |
| `NsUIFramework.Stack.PassiveAndClosing` | Passive widgets and widgets in their outro get no input |
| `NsUIFramework.Root.TextBoxKeepsKeys` | Backspace and E stay in a focused text box. A gamepad Confirm still reaches the handler |
| `NsUIFramework.Root.ConfirmAndBack` | Back bubbles to the root. The Confirm handler runs before the button, and a used Confirm never clicks the button |
| `NsUIFramework.Root.NavigateHandler` | A direction that the handler uses does not move the focus |

Run them with `Automation RunTests NsUIFramework`. The console rule, the input policies, and the Enhanced Input keys need a game world: check them with a play test.

## Change Rules

- Keep the names of `INsUIWidgetInterface` events and of the `UNsUISubsystem` Blueprint functions stable. Games and Blueprints call them by name.
- Keep the UI input actions without Consume Input.
- Keep the subsystem the only writer of the input mode and the mouse cursor.
