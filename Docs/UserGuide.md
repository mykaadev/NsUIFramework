# NsUIFramework User Guide

## Purpose

NsUIFramework opens and closes the UI widgets of each local player. It decides which widget gets input, gives the focus to a control, and routes Confirm, Back, the directions, and the tabs. It sets the input mode and the mouse cursor each time another widget gets input. It can also select world points or actors, and show floating texts over world points, for example "+3 coins".

NsUIFramework does not style widgets and does not animate them. Use NsUIStyle for the look and NsTween for motion. It does not replicate: each machine shows its own UI.

## Setup

1. Put the `NsUIFramework` folder in a plugin folder of the project.
2. Enable NsUIFramework in Edit > Plugins. It also enables Enhanced Input.
3. For C++, add `"NsUIFramework"` to the module dependencies in `Build.cs`.
4. Open Project Settings > Plugins > Ns UI Framework.
5. Set **Input Mapping Context** to `IMC_NsUI` and set each entry of **Input Actions** to its `IA_NsUI_` action. The plugin content folder `NsUIFramework/Input` has them.
6. Optional: set **Cursor Widget Class** to a Blueprint child of `NsUICursorWidget`.

Without step 5 the log shows an `Error`, and the UI has no keys.

## Quick Start

### Blueprint

1. Make a Widget Blueprint with `NsUIWidget` as the parent. Add a button.
2. In the player controller, call `Open Widget` from the NsUI function library.
3. Set **Widget Class** to the new widget and **Layer** to `Menu`.
4. Click Play. The widget shows, the button has the focus, and game input stops.
5. Push Escape or the gamepad Back button. The widget closes, and game input comes back.

### C++

```cpp
#include "NsUISubsystem.h"

UNsUISubsystem* const UI = LocalPlayer->GetSubsystem<UNsUISubsystem>();
UUserWidget* const Pause = UI->OpenWidget(PauseClass, nullptr, ENsUILayer::Menu);
UI->CloseWidget(Pause);
UI->CloseAll(ENsUILayer::Modal);
```

## Common Tasks

### Make a widget work with the framework

Use `NsUIWidget` as the parent. A widget with another parent can implement the interface `NsUIWidgetInterface` in Class Settings. Every event of the interface is optional.

### Respond to events

| Event | Use |
|---|---|
| `Get Config` | Return the input policy, the focus policy, the selection mode class, and `Show Mouse Cursor`. |
| `Get Focus` | Return the **Preferred Target**: the control that gets the focus when the widget has no earlier focus. |
| `On Opened` | Get the context object and the selection mode. |
| `On Activated` | The widget gets input. **Resumed** is true when it had input before. |
| `On Deactivated` | A higher widget took the input. The widget stays open. |
| `On Closed` | The widget closed. Release resources. |
| `On Focus Changed` | The focus moved to another control. Use it for the focus look and the focus sound. It comes on the frame after the move, so `HasAnyUserFocus` already gives the new control. |
| `Handle Confirm` | Return true to use Confirm before the focused control gets it. |
| `Handle Back` | Return true to use Back. Without it, Back closes the widget. |
| `Handle Navigate` | Return true to use a direction before the focus moves, for example to pan a map. |
| `Handle Tab` | Return true to use the next or previous tab action. |
| `Begin Close` | Return true to play an outro. Then call `Finish Close Widget`. |

### Choose the layer

| Layer | Use |
|---|---|
| `Game` | HUD parts and widgets of play. |
| `Menu` | Menus such as pause or settings. |
| `Modal` | Popups such as confirmations. |

Only one widget gets input: the highest layer first, and in one layer the newest widget.

### Choose the input policy

| Policy | Input |
|---|---|
| `Passive` | No input. Use it for prompts, toasts, and HUD parts. |
| `UI Only` | The default. The UI takes all input. |
| `UI And Game` | The UI takes the UI actions. All other keys and the sticks go to the game. |
| `Game First` | The game keeps the focus. `Handle Confirm` and `Handle Back` also get their keys, for example for a dialogue line. |

A widget that changes its policy while it is open calls `Refresh Config`.

### Use a widget that another system shows

Some systems make and show their own widget, for example a dialogue plugin that uses `Add to Viewport`.

1. Give the widget the input policy `Game First` or `Passive` in `Get Config`.
2. When the widget shows, call `Add External Widget` with the widget and the layer. `Native Construct` is a good place.
3. Before the widget goes away, call `Remove External Widget`. `Native Destruct` is a good place.

The framework does not move the widget and does not touch its Slate. It only routes Confirm and Back to its handlers. `Close Widget` does not close it: the system that shows it removes it.

### Toggle a widget, for example a pause menu

Call `Toggle Widget` with the class. It closes the newest open widget of the class, or opens a new one.

### Know if a widget is open

Call `Is Widget Open`, `Find Widget`, or `Get Active Widget`. Do not test the parent of the widget.

### Put the focus on a control after a rebuild

Call `Focus Child` with the widget and the control. Call `Restore Focus` to go back to the last focused control.

A widget that hides or disables its controls when it opens, for example behind a loading screen, calls `Restore Focus` when the controls show. Until then the root holds the focus, and the first key also tries the focus again.

### Take all input in the widget itself

In `Get Config`, set **Focus Policy** to `Self`. No control takes the focus, and the handlers get Confirm, Back, the directions, and the tabs.

### Select actors or world points

1. In `Get Config`, set **Selection Mode Class** to `NsUIActorSelectionMode` or `NsUILocationSelectionMode`.
2. In `On Opened`, cast the selection mode and call `Set Candidates`.
3. In `Handle Confirm`, read `Get Selected Actor` or `Get Selected Id`.

The directions move to the nearest candidate inside a cone of that direction. The mouse selects a candidate near the pointer, and a left click confirms it. Set **Select On Activate** to false to start with no selection. Bind `On Selection Changed` to react to a new selection.

### Show a floating text in the world

1. Make a Widget Blueprint with `NsUIWorldPopupWidget` as the parent.
2. Add a Text Block named `PopupText`. Optional: add animations named `Appear` and `Disappear`.
3. In the class defaults, set the motion values (see Settings).
4. Call `Show Popup` on the `NsUIWorldPopupSubsystem` of the local player.

```cpp
UNsUIWorldPopupSubsystem* const Popups = LocalPlayer->GetSubsystem<UNsUIWorldPopupSubsystem>();
Popups->ShowPopup(PopupClass, FText::FromString(TEXT("+3 coins")), FVector(0.f, 0.f, 100.f), FLinearColor::Yellow, TargetActor);
```

### Open widgets when play starts

Make a Blueprint child of `NsUIHUD`, fill **Start Widgets**, and set it as the HUD class of the game mode. The HUD closes these widgets when play ends.

## Rules

- Open every widget that takes input with `Open Widget`. Do not use `Add to Viewport` for it. A widget that another system shows uses `Add External Widget`.
- Do not call `Set Input Mode` or change the mouse cursor. The framework owns them.
- Do not bind Confirm or Back in a widget. Use `Handle Confirm` and `Handle Back`.
- Set the first focus in `Get Focus`, never in a delay or a timer.
- Keep **Consume Input** off on the UI input actions. Otherwise the game loses those keys. The log shows an `Error` when an action consumes input.

## Settings

### Project settings (Plugins > Ns UI Framework)

| Setting | Default | Effect |
|---|---|---|
| **Input Mapping Context** | None | The mapping context of the UI keys. The framework adds it to each player. |
| **Input Mapping Priority** | 1000 | The priority of the UI mapping context. Keep it above the game contexts, or Enhanced Input drops UI keys that a game action consumes. |
| **Input Actions** | None | The Enhanced Input action of each UI action. |
| **Cursor Widget Class** | None | The selection cursor. None shows no selection cursor. |
| **Root Z Order** | 10 | The viewport order of the UI root. |
| **Cursor Z Order** | 1000 | The viewport order of the selection cursor. |
| **Pointer Pick Radius** | 24 | The distance in which the mouse selects a world candidate. |
| **World Navigation Cone Degrees** | 60 | The widest angle from a direction for the next world candidate. |

### Widget config (`Get Config`)

| Setting | Default | Effect |
|---|---|---|
| **Input Policy** | `UI Only` | How the widget shares input with the game. |
| **Focus Policy** | `Children` | `Self` keeps the focus on the widget itself. |
| **Selection Mode Class** | None | A world selection mode. None selects controls. |
| **Show Mouse Cursor** | true | Shows the mouse cursor while the player uses the mouse. |

### Default keys (`IMC_NsUI`)

| Action | Keys |
|---|---|
| Confirm | Enter, Space Bar, gamepad bottom face button |
| Back | Escape, gamepad right face button |
| Up, Down, Left, Right | The arrow keys and the D-pad |
| Next Tab, Previous Tab | Gamepad right and left shoulder |

Tab and Shift+Tab move the focus to the next and the previous control. The sticks move the focus in `UI Only`.

### Selection cursor (`NsUICursorWidget`)

| Setting | Default | Effect |
|---|---|---|
| **Anchor In Target** | (0, 0) | The point of the selected control that the cursor points at. |
| **Cursor Alignment** | (0, 0) | The point of the cursor that sits on the anchor. |
| **Glide Seconds** (`NsUITweenCursorWidget`) | 0.12 s | The time of one glide. Needs NsTween. |

### World popup widget

| Setting | Default | Effect |
|---|---|---|
| **Lifetime** | 1.2 s | The time that the popup shows. |
| **Rise Speed** | 0 | The upward speed in the world. |
| **Max Distance** | 3000 cm | The popup hides beyond this distance from the camera. |
| **Hide When Blocked** | true | Hides the popup while a Visibility trace from the camera hits something. |
| **Max Active** | 4 | The most popups of the class for one player. |
| **Stack Height** | 0 | The extra height for each popup that follows the same actor. |
| **Viewport Z Order** | 0 | The viewport order of the popup. |
| **Screen Alignment** | (0, 0) | The point of the popup that sits on the world point. |

## Debug

| Tool | Effect |
|---|---|
| `ns.UI.Inspect` | Console command, not in Shipping. Logs the open widgets, the active widget, the input policy, and the pointer mode of each local player. |
| NsUI Inspector | Editor tab (Window menu). Shows the same state for each play world. |
| `LogNsUI` | Log category. `Verbose` shows each open, close, and active change. |

## Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| The UI has no keys, and the log shows an `Error` | The settings have no mapping context or input actions. | Do Setup step 5. |
| Some UI keys do nothing, for example E or Escape | A game mapping context with a higher priority maps the key to an action that consumes input. | Raise **Input Mapping Priority**. `ns.UI.Inspect` lists the UI keys that the framework read. |
| The arrows do nothing when a widget opens | The controls were hidden or disabled when the widget got input. | Call `Restore Focus` when the controls show. |
| The game loses a key while the UI is open | A UI input action consumes input. | Turn off **Consume Input** on the action. |
| `Open Widget` returns None | The player has no player controller yet. | Open the widget from the player controller or the HUD. |
| A widget shows but takes no input | A higher widget is active, or the widget is `Passive`. | Close the higher widget, or change the policy. |
| Back does not close a widget | `Handle Back` returned true. | Return false when the widget must close. |

## Limits

- While a text box has the focus, keyboard keys are never UI actions, so Escape and Backspace do not close the widget. A gamepad Confirm and Back still work, and Tab or the arrows leave the text box.
- The selection cursor follows the focused control only when it has a size on screen.
- The framework does not detect an Xbox pad from a PlayStation pad. NsInput does that for the prompt icons.
- With NsInput, the framework tells NsInput the device that the player uses in the UI. It does not read the NsInput device.
- A widget that implements the interface without the `NsUIWidget` parent does not close when the game calls `Remove From Parent`. Call `Close Widget`.

## Migrate from v1

| v1 | v2 |
|---|---|
| `INsUIScreen`, `UNsUIScreen` | `INsUIWidgetInterface`, `UNsUIWidgetInterface` |
| `OpenScreen`, `CloseScreen`, `ToggleScreen`, `FindScreen` | `OpenWidget`, `CloseWidget`, `ToggleWidget`, `FindWidget` |
| `GetNsScreenConfig`, `GetNsScreenSelection` | `GetConfig`, `GetFocus` |
| `OnNsScreenOpened`, `OnNsScreenActivated`, `OnNsScreenSuspended`, `OnNsScreenClosed` | `OnOpened`, `OnActivated`, `OnDeactivated`, `OnClosed` |
| `HandleNsScreenConfirm`, `HandleNsScreenCancel`, `HandleNsScreenNavigate` | `HandleConfirm`, `HandleBack`, `HandleNavigate` |
| `FNsUIScreenConfig` with `bInteractive`, `Gameplay`, `bGameInputOnly` | `FNsUIWidgetConfig` with `InputPolicy` |
| `bInteractive = false` | `InputPolicy = Passive` |
| `Gameplay = Allow`, or the `Game` layer with `Automatic` | `InputPolicy = UIAndGame` |
| `bGameInputOnly = true` | `InputPolicy = GameFirst` |
| `ENsUISelectionPolicy::Logical` | `FocusPolicy = Self` |
| `FNsUISelectionRequest` | `FNsUIFocusRequest` |
| `UNsUIUMGSelectionMode` | No class. The focused control is the selection. |
| `UNsUIDeveloperSettings` key lists | `UNsUISettings` with Enhanced Input actions |
| Add to Viewport screens, found by the framework | Removed. Use `OpenWidget`, or `AddExternalWidget` for a widget that another system shows. |
| `ns.UI.SmokeTest.Toggle` | Removed. Automation tests `NsUIFramework.*` replace it. |

Work items for a game:

1. Rename the interface, the functions, and the config fields with the table.
2. Replace focus polling in Tick with `On Focus Changed`.
3. Replace `GetParent()` checks with `Is Widget Open`.
4. Replace `SetUserFocus` after a rebuild with `Focus Child`.
5. Remove input preprocessors that run before the UI. The framework no longer takes keys before widgets.
6. Set the input settings (Setup step 5). A game that used extra UI keys adds them to a copy of `IMC_NsUI`.
