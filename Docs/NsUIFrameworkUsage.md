# NsUIFramework

## Screens

One host manages interaction for each local player.

```cpp
UNsUISubsystem* UI = LocalPlayer->GetSubsystem<UNsUISubsystem>();
UUserWidget* Screen = UI->OpenScreen(MenuClass, Context, ENsUILayer::Menu);
UI->CloseScreen(Screen);
UI->CloseAll(ENsUILayer::Modal);
```

The host owns screens opened through this API. Context stays alive until the screen closes.

A widget added with `AddToViewport` participates only when it implements `INsUIScreen`.
Its creator still owns its removal. Host close commands do not remove external widgets.
Discovery uses viewport events and one initial search. The host excludes preview worlds.

## Screen Contract

Implement `INsUIScreen` without changing the Blueprint parent.

- `GetNsScreenConfig`: layer, input participation, gameplay policy, and selection mode.
- `OnNsScreenOpened`: context and the selection mode instance for this screen.
- `OnNsScreenActivated`: activation, with a flag for resumed screens.
- `OnNsScreenSuspended` and `OnNsScreenClosed`: interaction and lifetime changes.
- `GetNsScreenSelection`: automatic selection, an optional preferred target, or logical input.
- `HandleNsScreenConfirm` and `HandleNsScreenCancel`: return true when the screen consumes the action.

Host screens use the layer passed to `OpenScreen`. External screens use their configuration.
Set `bInteractive` to false for prompts and other passive screens.

Automatic gameplay policy allows gameplay on Game and blocks it on Menu and Modal.
Dialogue sets gameplay policy to Allow.
Allow reserves analog sticks for gameplay. Digital directions still navigate the UI.

Only the highest interactive screen receives input: Modal, then Menu, then Game.
Within a layer, the last opened screen takes priority.
Closing the active screen restores the previous valid selection.
A preferred target does not replace a valid user selection.

## Selection

Each screen owns one `UNsUISelectionMode` instance, including while suspended.
Extend this abstract class in C++. Modes do not own Slate hooks or input mode.

- `UNsUIUMGSelectionMode`: nested widget trees, native control actions, and native navigation.
- `UNsUIActorSelectionMode`: explicit weak actor references through `SetCandidates`.
- `UNsUILocationSelectionMode`: explicit positions with unique, nonempty IDs through `SetCandidates`.

Supply world candidates in `OnNsScreenOpened` or update the retained mode instance.
World modes use screen projection. Candidate order resolves ties.
Override mode actions to act on `GetSelectedActor` or `GetSelectedId`.

The cursor follows the selected target. Logical input has no cursor.
Selection recovery waits until after the Slate event.
Editor interaction, Shift+F1, and application deactivation suspend recovery.

## Editor

`ns.UI.SmokeTest.Toggle` opens or closes the manual smoke screen.
It includes menus, modals, viewport discovery, dynamic controls, actors, and locations.
Confirm removes the selected smoke actor. Cancel closes a world selection screen.

`ns.UI.Inspect` opens a read-only view of the stack, selection, and recovery reason.
The editor tools do not change runtime routing.
