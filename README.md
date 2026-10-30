<!-- GH_ONLY_START -->
<h1 align="center">
  <br>
  <a href="https://github.com/mykaadev/NsUIFramework"><img src="https://raw.githubusercontent.com/mykaadev/NsUIFramework/refs/heads/main/Resources/Banner.png" alt="NsUIFramework" width="400"></a>
</h1>

<h4 align="center">Screens on layers, input routing, and keyboard, mouse, and gamepad selection for Unreal Engine.</h4>

<div align="center">
    <a href="https://github.com/mykaadev/NsUIFramework/commits/main"><img src="https://img.shields.io/github/last-commit/mykaadev/NsUIFramework?style=plastic&logo=github&logoColor=white" alt="GitHub Last Commit"></a>
    <a href="https://github.com/mykaadev/NsUIFramework/issues"><img src="https://img.shields.io/github/issues-raw/mykaadev/NsUIFramework?style=plastic&logo=github&logoColor=white" alt="GitHub Issues"></a>
    <a href="https://github.com/mykaadev/NsUIFramework/pulls"><img src="https://img.shields.io/github/issues-pr-raw/mykaadev/NsUIFramework?style=plastic&logo=github&logoColor=white" alt="GitHub Pull Requests"></a>
    <a href="https://github.com/mykaadev/NsUIFramework"><img src="https://img.shields.io/github/stars/mykaadev/NsUIFramework?style=plastic&logo=github" alt="GitHub Stars"></a>
    <a href="https://twitter.com/mykaadev/"><img src="https://img.shields.io/twitter/follow/mykaadev?style=plastic&logo=x" alt="Twitter Follow"></a>

<p style="display:none;">
  <a href="#-summary">👀 Summary</a> •
  <a href="#-features">📦 Features</a> •
  <a href="#-requirements">⚙️ Requirements</a> •
  <a href="#-installation">🛠️ Installation</a> •
  <a href="#-getting-started">🚀 Getting Started</a> •
  <a href="#-layers">🗂️ Layers</a> •
  <a href="#-api">🔧 API</a> •
  <a href="#-limits">🚧 Limits</a> •
  <a href="#-documentation">📚 Documentation</a> •
  <a href="#-credits">❤️ Credits</a> •
  <a href="#-support">📞 Support</a> •
  <a href="#-license">📃 License</a>
</p>
<a href="https://buymeacoffee.com/mykaadev"><img src="https://www.svgrepo.com/show/476855/coffee-to-go.svg" alt="Coffee" width="50px"></a>
<p><b>Buy me a coffee!</b></p>
</div>
<!-- GH_ONLY_END -->

## 👀 Summary
NsUIFramework owns UI interaction. It opens and closes screens on three layers, decides which screen gets input, moves the selection with keyboard, mouse, and gamepad, and routes Confirm and Back. It also sets the input mode and the mouse cursor, so game code never calls `SetInputMode`.

NsUIFramework does not style widgets, animate them, or show input prompt icons.

## 📦 Features
- **Screen stack**: Open and close screens on the `Game`, `Menu`, and `Modal` layers. Only the highest interactive screen gets input.
- **Any widget**: Use `UNsUIWidget` as the parent, or implement `INsUIScreen` on any `UUserWidget`.
- **Confirm and Back**: One place to handle them, in the screen events. No key binding in each widget.
- **Input mode and cursor**: Set for you each time the active screen changes.
- **Gameplay policy**: Block gameplay input under a menu, or let it continue, for example under dialogue.
- **Selection modes**: Native UMG focus, or directional selection of actors and world points on screen.
- **Viewport discovery**: A widget added with `AddToViewport` takes part when it implements `INsUIScreen`.
- **Debug tools**: A read-only view of the screen stack, and a smoke test screen.

## ⚙️ Requirements
Unreal Engine 5.7+

## 🛠️ Installation
1. **Clone** or download this repository.
2. Copy the `NsUIFramework` folder into the `Plugins` folder of your project.
3. Add `"NsUIFramework"` to the module dependencies in `Build.cs`.
4. Generate the project files and enable the plugin.
5. Optional: open Project Settings, Game, "NsUI Framework", and change the keys.

## 🚀 Getting Started
Make a widget with `UNsUIWidget` as the parent, then open it through the subsystem of the local player:

```cpp
#include "NsUISubsystem.h"

UNsUISubsystem* const UI = LocalPlayer->GetSubsystem<UNsUISubsystem>();
UUserWidget* const Screen = UI->OpenScreen(PauseMenuClass, Context, ENsUILayer::Menu);

UI->CloseScreen(Screen);
UI->CloseAll(ENsUILayer::Modal);
```

The subsystem owns the screens that `OpenScreen` makes. The context object stays alive until the screen closes.

To handle Confirm and Back, override `HandleNsScreenConfirm` and `HandleNsScreenCancel` in the screen. Return true when the screen uses the action.

## 🗂️ Layers
| Layer | Use | Gameplay input with `Automatic` |
|---|---|---|
| `Game` | HUD parts that take input during play | Allowed |
| `Menu` | Menus such as pause or settings | Blocked |
| `Modal` | Popups such as confirmations | Blocked |

Modal comes first, then Menu, then Game. In one layer, the last screen to open has priority. Set `bInteractive` to false for prompts and toasts that take no input.

## 🔧 API
### Screens
- `UNsUISubsystem` – The local player subsystem: `OpenScreen`, `CloseScreen`, and `CloseAll`.
- `INsUIScreen` – The screen contract. All events are optional.
- `UNsUIWidget` – A `UUserWidget` parent that implements `INsUIScreen`.
- `FNsUIScreenConfig` – The layer, `bInteractive`, the gameplay policy, and the selection mode class.

### Screen Events
- `GetNsScreenConfig` – Return the config of the screen.
- `OnNsScreenOpened` / `OnNsScreenClosed` – Get the context and the selection mode, and release resources.
- `OnNsScreenActivated` / `OnNsScreenSuspended` – The screen gets input, or loses it but stays open.
- `GetNsScreenSelection` – Return automatic selection, a preferred target, or logical input.
- `HandleNsScreenConfirm` / `HandleNsScreenCancel` – Use Confirm or Back before the default action.

### Selection
- `UNsUIUMGSelectionMode` – Native UMG focus and navigation.
- `UNsUIActorSelectionMode` / `UNsUILocationSelectionMode` – Select actors or world points. Call `SetCandidates`, then read `GetSelectedActor` or `GetSelectedId`.
- `UNsUISelectionMode` – The base class for a new kind of selection, in C++.

### Debug
- `ns.UI.Inspect` – A read-only view of the screen stack, the selection, and the recovery reason. Editor only.
- `ns.UI.SmokeTest.Toggle` – A test screen with menus, modals, dynamic controls, actors, and locations. Editor only.

## 🚧 Limits
- The mouse cursor shows whenever a screen is active, also on the `Game` layer.
- No device detection and no prompt icons.
- Blueprint cannot extend `UNsUISelectionMode`.
- The subsystem ignores preview worlds.

## 📚 Documentation
- [User Guide](https://mykaadev.github.io/plugins/nsuiframework/user-guide/) – Setup, common tasks, settings, and debug tools.
- [Technical Documentation](https://mykaadev.github.io/plugins/nsuiframework/tech-documentation/) – Architecture, lifetime, extension points, and tests.

<!-- GH_ONLY_START -->
## ❤️ Credits
<a href="https://github.com/mykaadev/NsUIFramework/graphs/contributors"><img src="https://contrib.rocks/image?repo=mykaadev/NsUIFramework"/></a>

## 📞 Support
Reach out via the **[profile page](https://github.com/mykaadev)**.

## 📃 License
[![License](https://img.shields.io/badge/license-MIT-green)](https://www.tldrlegal.com/license/mit-license)
<!-- GH_ONLY_END -->
