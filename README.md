# NsUIFramework

UI widgets, focus, and UI input for each local player in Unreal Engine 5.

- Opens and closes widgets on three layers: Game, Menu, and Modal.
- Gives input to one widget: the highest layer, then the newest.
- Routes Confirm, Back, the directions, and the tabs from Enhanced Input actions, so player remaps apply to the UI.
- Text boxes and the console keep their keys. The framework acts only on keys that no control used.
- Sets the input mode and the mouse cursor. Hover gives the focus, and a selection cursor shows for keys and gamepads.
- Selects actors or world points with the keys, a gamepad, or the mouse.
- Shows floating texts over world points.
- Optional: a gliding cursor with NsTween, and device sync with NsInput.

```cpp
UNsUISubsystem* const UI = LocalPlayer->GetSubsystem<UNsUISubsystem>();
UUserWidget* const Pause = UI->OpenWidget(PauseClass, nullptr, ENsUILayer::Menu);
```

```cpp
// In a widget that uses NsUIWidget as its parent
FNsUIWidgetConfig UMyPauseMenu::GetConfig_Implementation() const
{
    FNsUIWidgetConfig Config;
    Config.InputPolicy = ENsUIInputPolicy::UIOnly;
    return Config;
}

bool UMyPauseMenu::HandleBack_Implementation()
{
    Resume();
    return true;
}
```

## Docs

- [User Guide](Docs/UserGuide.md): setup, common tasks, settings, debug, and the migration from v1.
- [Technical Documentation](Docs/TechDocumentation.md): architecture, input flow, data ownership, lifetime, and tests.

## Requirements

- Unreal Engine 5.7.
- The Enhanced Input plugin.
- Optional: NsTween, NsInput.

## License

See [LICENSE](LICENSE).
