# NsUIFramework Technical Documentation

## Architecture

### Modules

| Module | Type | Loading phase | Content |
|---|---|---|---|
| `NsUIFramework` | Runtime | Default | The screen subsystem, the screen interface, the selection modes, the Slate input router, the layout and the cursor, the HUD, the function library, the developer settings, and the world popups |
| `NsUIFrameworkEditor` | Editor | Default | The `ns.UI.Inspect` and `ns.UI.SmokeTest.Toggle` console commands, the NsUI Inspector tab, and the smoke test widgets |

The `.uplugin` has no platform allow list. The plugin has no content and no default settings in `Config/`.

### Folders

| Folder | Content |
|---|---|
| `Source/NsUIFramework/Public` | `NsUIFrameworkTypes.h`, `NsUIScreen.h`, `NsUIWidget.h`, `NsUISubsystem.h`, `NsUIHUD.h`, `NsUIFunctionLibrary.h`, `NsUIDeveloperSettings.h`, `NsUIWorldPopupSubsystem.h`, `NsUIWorldPopupWidget.h`, `NsUIFramework.h` |
| `Source/NsUIFramework/Public/Selection` | `NsUISelectionMode.h`: the base mode and the UMG, world, actor, and location modes |
| `Source/NsUIFramework/Private` | The implementations, and the private headers `NsUILayout.h`, `NsUILayer.h`, `NsUICursor.h`, `NsUISlateInputRouter.h` |
| `Source/NsUIFramework/Private/Selection` | `NsUISelectionMode.cpp` |
| `Source/NsUIFrameworkEditor/Public` | `NsUIFrameworkEditor.h`, `Smoke/NsUISmokeTestWidgets.h` |
| `Source/NsUIFrameworkEditor/Private` | `NsUIFrameworkEditor.cpp`, `NsUISmokeTestWidgets.cpp` |

### Main Classes

| Type | Responsibility |
|---|---|
| `UNsUISubsystem` | A `ULocalPlayerSubsystem` and `FTickableGameObject`. It owns the screen stack, the presentation, the input policy, the navigation config, and selection recovery. |
| `INsUIScreen` | The optional screen contract. Every function is a `BlueprintNativeEvent`. |
| `UNsUIWidget` | A `UUserWidget` parent that implements `INsUIScreen`. |
| `FNsUIScreenConfig` | `Layer`, `bInteractive`, `Gameplay`, `SelectionMode`. |
| `FNsUISelectionRequest` | `Policy` (`Automatic` or `Logical`) and `PreferredTarget`. |
| `ENsUILayer`, `ENsUIGameplayPolicy`, `ENsUISelectionPolicy` | The layer, gameplay, and selection enums. |
| `UNsUISelectionMode` | The abstract base of selection. One object for each screen. |
| `UNsUIUMGSelectionMode` | Native UMG focus and navigation. It collects focusable controls, also inside nested user widgets and list views. The default mode. |
| `UNsUIWorldSelectionMode` | The abstract base of directional selection of world candidates through screen projection. |
| `UNsUIActorSelectionMode`, `UNsUILocationSelectionMode` | World modes for weak actor lists and for `FNsUILocation` lists (id and position). |
| `FNsUISlateInputRouter` | A Slate input pre-processor. It routes keys, analog input, and pointer events to the subsystem, and watches focus changes and app activation. |
| `UNsUILayout`, `UNsUILayer` | Private. The layout holds one overlay panel for each layer. |
| `UNsUICursor` | Private. A non-hit-testable image that follows the selection. |
| `UNsUIDeveloperSettings` | The keys and the cursor motion (`config=Game`, `defaultconfig`). |
| `ANsUIHUD` | A `Blueprintable` `AHUD` that opens `StartScreens` through `UNsUIFunctionLibrary::OpenScreen` on `BeginPlay`. |
| `UNsUIFunctionLibrary` | Static Blueprint calls: `GetUISubsystem`, `OpenScreen`, `ToggleScreen`, `CloseScreen`. |
| `UNsUIWorldPopupSubsystem` | A `ULocalPlayerSubsystem` and conditional `FTickableGameObject`. It shows, moves, and pools floating texts over world points. |
| `UNsUIWorldPopupWidget` | The abstract `UUserWidget` parent of one popup. It holds the motion values and the optional `PopupText`. |
| `FNsUIWorldPopupEntry` | One active popup: the widget, the follow actor, the point or offset, and the age. |

### Data Flow

#### Open flow

1. `OpenScreen` calls `EnsurePresentation`. The first call makes the layout, wraps it in a focusable input root and a constraint canvas for the split screen area of the player, and adds it to the game viewport. It also adds the cursor widget to the viewport at Z order `MAX_int32 - 100`.
2. `OpenScreen` makes the widget for the local player controller and adds it to the layer panel of the requested layer.
3. `AddEntry` reads the config from `GetNsScreenConfig`. For an owned screen, the `Layer` argument replaces the config layer.
4. `AddEntry` makes the selection mode object. A null or abstract class falls back to `UNsUIUMGSelectionMode`.
5. `AddEntry` puts a navigation stop on each escape boundary of the screen root, so navigation stays in the screen.
6. The subsystem calls `OnNsScreenOpened` with the context and the mode.
7. `Reconcile` picks the active screen and calls `OnNsScreenSuspended` and `OnNsScreenActivated` as needed.
8. `ApplyInputPolicy` sets the input mode and the cursor when the active screen changes.

`Reconcile` picks the interactive, visible, and enabled screen with the highest layer. In one layer, the latest entry wins. Other interactive screens become `HitTestInvisible`. When the active screen is owned and a viewport screen is open, `Reconcile` moves the presentation over the highest viewport screen.

#### Close flow

`RemoveEntry` gives back the original navigation metadata and visibility, calls `OnNsScreenClosed`, removes an owned widget from its parent, and calls `Reconcile`. `CloseScreen` and `CloseAll` act on owned entries only.

#### Viewport discovery

The subsystem listens to `UGameViewportSubsystem::OnWidgetAdded` and `OnWidgetRemoved`. On the first tick of a world, it also searches the existing user widgets in Z order. A user widget in the same game world, for the same local player, that implements `INsUIScreen` becomes an external entry. The subsystem never removes an external widget from the viewport.

#### Input policy

| Active screen | Input mode | Mouse cursor |
|---|---|---|
| None | `FInputModeGameOnly` | Hidden |
| Gameplay allowed | `FInputModeGameAndUI`, no lock, cursor not hidden on capture | Shown |
| Gameplay blocked | `FInputModeUIOnly`, no lock | Shown |

Gameplay is allowed when the policy is `Allow`, or when it is `Automatic` and the layer is `Game`. `UpdateNavigationConfig` installs a copy of the user navigation config while a screen is active. It maps the confirm and cancel keys to Accept and Back, maps the direction keys, and turns analog navigation off when gameplay is allowed.

#### Input routing

The router is a Slate input pre-processor, so it sees events before widgets do. It resolves the subsystem from the Slate user index.

- Confirm and Cancel keys go to `RouteAction`. The screen handler runs first, then the selection mode. Cancel then closes an owned screen. A consumed key also consumes its key up.
- Direction keys, Tab, and Shift+Tab go to `RouteNavigate`. The UMG mode leaves them to native Slate navigation.
- Analog input becomes a direction for world modes. The UMG mode leaves analog input to Slate.
- Pointer moves go to `ObservePointer`, so the mouse can select. A left click on a world candidate selects it and calls `RouteAction(true)`.
- Shift+F1, a click outside the game viewport, and app deactivation suspend the subsystem. Focus back in the game viewport resumes it.

#### Tick

`UNsUISubsystem::Tick` runs each frame in game worlds, also while the game is paused:

1. It applies a dirty input policy and runs viewport discovery once for each world.
2. It drops entries whose widgets left their parent or the viewport.
3. It calls `Reconcile`.
4. For the active screen, every 0.1 s it calls `Refresh` on the mode and checks `HasSelection`.
5. It runs `RecoverSelection` when a reason is set. Recovery happens here, never inside a Slate event.
6. It moves the cursor toward `GetCursorPosition` of the mode.

#### World popups

1. `ShowPopup` ends the oldest popups of the class while the class has `MaxActive` popups.
2. It takes a pooled widget of the class for the same player controller, or makes one.
3. It adds the widget to the player screen at `ViewportZOrder`, with the alignment (0.5, 1).
4. A popup on an actor that already has popups starts `StackHeight` higher for each of them.
5. Each tick moves the point up by `RiseSpeed`, applies the fade and the pop scale, and projects the point to the screen.
6. The popup hides beyond `MaxDistance` from the camera and behind the camera. With `bHideWhenBlocked`, it also hides when a `ECC_Visibility` trace from the camera hits something. The trace ignores the pawn of the player and the follow actor.
7. At `Lifetime`, or when the follow actor goes away, the widget leaves the screen and goes back to the pool.

## Lifetime

- `FNsUIFrameworkModule::StartupModule` registers the input router when Slate exists, and again on `OnPostEngineInit`. `ShutdownModule` removes it.
- `UNsUISubsystem::Initialize` binds the viewport and world cleanup delegates. `Deinitialize` removes every entry (and owned widgets), the presentation, and the cursor.
- An entry holds strong references to its widget, its context, and its selection mode until `RemoveEntry`. A mode stays alive while its screen is suspended.
- On world cleanup, the subsystem removes the entries, the layout, and the cursor of that world. It marks the input policy as dirty, so the next tick sets the input mode for the new player controller, also when no screen is active.
- `ANsUIHUD::BeginPlay` opens the start screens for a local player controller. The screens close on world cleanup.
- On world cleanup, the world popup subsystem ends its popups and drops pooled widgets of that world. `Deinitialize` clears all popups and the pool.
- `FNsUIFrameworkEditorModule` registers the two console commands and the inspector tab spawner on startup, and removes them on shutdown.

## Extension Points

| Point | Use |
|---|---|
| `INsUIScreen` | Implement it on any widget, in C++ or Blueprint. In C++, override `GetNsScreenConfig_Implementation`, `GetNsScreenSelection_Implementation`, `OnNsScreenOpened_Implementation(UObject* Context, UNsUISelectionMode* SelectionMode)`, `OnNsScreenActivated_Implementation(bool bResumed)`, `OnNsScreenSuspended_Implementation`, `OnNsScreenClosed_Implementation`, `HandleNsScreenConfirm_Implementation`, `HandleNsScreenCancel_Implementation`. |
| `UNsUISelectionMode` | C++ only (`Abstract`, `NotBlueprintable`). Override the pure virtuals `Restore(UWidget*)`, `HasSelection()`, and `GetCursorPosition(FVector2D&)`. Optional virtuals: `Initialize`, `Refresh`, `Navigate(EUINavigation)`, `UsesNativeNavigation`, `ObservePointer`, `Confirm`, `Cancel`, `GetFocusTarget`, `ObserveFocus`, `DescribeSelection`. |
| `UNsUIWorldSelectionMode` | Override `GetCandidateCount()` and `GetCandidatePosition(int32 Index, FVector& Position)` for a new world candidate source. |
| `ANsUIHUD` | Subclass in Blueprint or C++. `BeginPlay` is virtual. |
| `UNsUIWorldPopupWidget` | Subclass in Blueprint. Implement the `BlueprintImplementableEvent` `OnPopupStarted(const FText& Text, FLinearColor Color)`. |
| `UNsUIDeveloperSettings` | The keys and the cursor motion, in `DefaultGame.ini`. |

## Dependencies

| Dependency | Kind | Why |
|---|---|---|
| Core, UMG, DeveloperSettings | Public, `NsUIFramework` | Public headers use UMG widgets and `UDeveloperSettings` |
| CoreUObject, Engine, Slate, SlateCore, InputCore | Private, `NsUIFramework` | Subsystems, player controllers, the input pre-processor, keys |
| Core, NsUIFramework, UMG | Public, `NsUIFrameworkEditor` | The smoke test widgets implement `INsUIScreen` |
| CoreUObject, Engine, Slate, SlateCore, InputCore | Private, `NsUIFrameworkEditor` | Console commands, the inspector tab, spawned target points |

The `.uplugin` lists no plugin dependencies.

## Performance

- `UNsUISubsystem` ticks each frame in game worlds. Each tick calls `Reconcile`, which walks the screen list.
- The active mode gets `Refresh` every 0.1 s. The UMG mode then walks the whole widget tree of the screen.
- World modes project candidates on navigation, on each pointer move, every 0.1 s for `HasSelection`, and each frame for the cursor.
- The world popup subsystem ticks only while a popup shows. Each popup does one projection each frame and, with `bHideWhenBlocked`, one line trace.
- Popup widgets go back to a pool, for each class and player controller.

## Networking

The plugin does not replicate. Each local player has its own subsystems.

## Testing

The plugin has no automated tests.

Manual smoke test, in a Play In Editor session:

1. Run `ns.UI.SmokeTest.Toggle`.
2. Open a menu, then a modal. Make sure that only the modal gets input.
3. Close the modal. Make sure that the selection goes back to the menu.
4. Click **Viewport screen**. Make sure that the new screen gets input.
5. Open the actor and location screens. Use them with a gamepad and with a mouse.
6. Run `ns.UI.Inspect` and read the stack and the recovery reason.

## Change Rules

- Keep the names of `FNsUIScreenConfig`, `FNsUISelectionRequest`, `FNsUIStartScreen`, and the `UNsUIWorldPopupWidget` properties. Blueprints and assets save them.
- Keep the `INsUIScreen` function names and signatures. Blueprint implementations bind to them by name.
- Keep the values and the order of `ENsUILayer`. `Reconcile` compares layers by value.
