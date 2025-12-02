# AAA-Grade UI Framework Principles & Gaps

This document outlines concerns and implementation approaches to evolve the NsUIFramework toward an industry-standard, platform-agnostic UI stack. It focuses on predictable navigation, stack/state management, accessibility, performance, and production workflows.

## 1) Core architectural expectations
- **Explicit input abstraction and routing**: Normalize all inputs (gamepad, KBM, touch, VR, streaming remotes) through a command bus with context-aware routing before reaching widgets. Guarantee per-context deadzones, repeat rates, and debouncing; expose routing layers (global → scene → layer → widget) so designers can redirect without code changes.
- **Stateful layout & stack model**: Keep the current layer-stack concept but formalize it as a state machine with typed transitions (Push/Pop/Replace/Remove/Reset) and telemetry hooks. Each transition should produce a `UIStateEvent` for analytics, replay, and automated tests.
- **Deterministic focus contract**: For any push/replace/pop/remove, the system must deterministically decide (a) which widget gains focus, (b) how to restore focus when stacks unwind, and (c) how to resolve clashes with world/actor selection. Publish the focus rules in one place and add asserts when the rules cannot be satisfied.

## 2) Navigation & focus patterns
- **Strong interface for navigation metadata**: Continue to use interfaces like `INsUINavigation`, but make neighbor hints optional overrides on top of a reliable spatial solver. Prefer coarse-grain authoring (sections, lists, grids) over per-widget wiring; add authoring-time validation to catch missing neighbors or non-focusable widgets.
- **Priority-based focus resolution**: Use a priority stack (modal overlays > dialogs > menus > HUD) and block lower layers until the higher priority explicitly yields. This prevents “hidden-in-stack” widgets from stealing focus or consuming input.
- **Fallback spatial search with filters**: Keep spatial search but add filters for visibility, enabled state, platform, and interaction mode (mouse vs. gamepad). Record debug traces that show how a focus target was chosen to ease tuning.
- **Context-aware confirm/cancel**: Instead of purely registry-based confirm/cancel, route through a context graph: global shortcuts, layer-level handlers, widget-level handlers, then default behaviors. Allow temporary overrides (e.g., photo mode) to register and scope themselves without touching existing widgets.

## 3) Stack & state management upgrades
- **Address blind pops**: Layer stacks should support keyed removal and replacement (already present) plus named scopes. Let callers address widgets by stable IDs or tags, and expose queries (`FindFirst/FindAll by tag`) to avoid iterative popping.
- **Transitions as data**: Represent stack transitions as data assets (e.g., “Pause → Settings → Rebinds”) to enable reuse, async loading, and preview in editor. Transitions carry: target widget class, focus rule, animation preset, and platform constraints.
- **History & restoration**: Persist stack history snapshots per player/session so the framework can restore UI when rejoining or recovering from a crash. Include serialization of focus target and transient widget state when supported by the widget.
- **Scoped lifetime policies**: Add lifetime modes—`Transient` (auto-pop on blur), `Modal` (blocks lower layers), `Persistent` (HUD), and `Pinned` (cannot be removed unless by tag). Use policies to prevent accidental removal of critical HUD elements when using generic removal APIs.

## 4) Accessibility & inclusivity
- **Screen reader & narration**: Provide semantic trees and ARIA-like roles for UMG/Slate to drive narration. Every focus move should emit a narration event containing label, value, state, and hint.
- **Remappable + device-aware prompts**: Centralize input prompt generation so any widget can request “Confirm” text/icon for the current device. Cache prompt sets per platform and locale; expose a debug surface to verify prompt coverage.
- **Color/contrast & font scaling**: Support runtime theming with WCAG contrast validation and per-widget opt-outs. Pair this with a global text scaling setting that reflows layouts using dynamic layout rules instead of manual anchor tweaks.
- **Haptics & audio feedback**: Standardize focus, confirm, error, and boundary-feedback events that map to haptic patterns and UI SFX; keep them data-driven so designers can swap banks without code changes.

## 5) Performance & scalability
- **Pooling & virtualization**: Add widget pooling for list/grid items and heavy overlays, plus viewport-visibility-driven tick throttling. Track perf budgets per layer and log when animations or materials exceed budgets.
- **Async asset flow**: Make pushes/replacements async-aware: if a widget needs assets, show a lightweight placeholder and complete focus transfer only after readiness. Include cancellation hooks when users back out mid-load.
- **Deterministic animation system**: Standardize enter/exit animations with named presets (e.g., `ModalFade`, `SlideInRight`, `HUDPulse`). Animations should be interruptible, forward-seekable, and emit lifecycle events for telemetry.

## 6) Authoring experience
- **Blueprint-first, C++-backed**: Keep Blueprint usability but back critical flows with C++ services. Expose validation tools (an “Input Navigation Audit” tab) to visualize navigation graphs and detect dead ends or unfocusable widgets.
- **Data-driven definitions**: Move recurring patterns into data assets (menu definitions, prompt sets, accessibility roles, animation presets). This reduces blueprint sprawl and enables live reloading.
- **Preview & simulation**: Provide editor simulations for devices (gamepad vs. touch), localization lengths, colorblind filters, and reduced-motion. Persist per-level UI layout previews for quick iteration.

## 7) Reliability, testing, and telemetry
- **Deterministic testing harness**: Add an automated UI runner that can: load a layout, push widgets, simulate navigation/confirm/cancel, assert focus targets, and capture golden screenshots. Make it data-driven so QA can author flows without code.
- **Metrics & breadcrumbs**: Emit breadcrumbs for every transition, focus change, and input consume path. Attach widget class, stack depth, layer type, and invocation context so issues like “hidden widget eating input” are traceable.
- **Robust error handling**: Fail fast on missing bindings (e.g., `LayerRoot` null) and provide on-screen debug overlays that show stack state, current focus, and input ownership.

## 8) World interaction parity
- **Unified focus graph**: Treat world/actor selection as another focus provider with explicit priority rules versus UI layers. Surface APIs to temporarily elevate world focus (photo mode) or suppress it (modal dialogs) without manual flag flipping.
- **Hit-proxy and viewport alignment**: Ensure actor focus targets include Z-order, bounding shape, and obstruction checks so navigation between UI and world feels consistent. Allow designers to preview projected bounds inside the UI debug overlay.

## 9) Migration suggestions for the current codebase
- Define a single “focus & navigation contract” doc/code module that spells out how `UNsUILayer`, `UNsUISubsystem`, and `INsUINavigation` interact. Add runtime asserts when the contract is violated to catch hidden/invalid widgets early.
- Extend the current layer stack to store stable IDs/tags per `UNsUIWidgetStackElement`, enabling targeted removal/replacement without blind pops or pointer searches.
- Add a priority flag to `UNsUILayer` instances to block lower layers and redirect input until the priority yields; expose this in `UNsUISubsystem` routing so actors vs. UI can be resolved centrally.
- Create a data-driven transition asset (`UNsUIFlowAsset`) that encapsulates Push/Replace/Remove + animation + focus rule, and let Blueprint library helpers consume it instead of manual calls.
- Introduce a minimal telemetry surface: on every Push/Pop/Replace/Remove, emit a struct containing layer, widget class, invoker, focus before/after, and reason; log to a rotating debug buffer and analytics hook for QA.
- Provide debug visualization widgets that render the current stack, focus target, and navigation candidates (including actor selection candidates) to make iteration predictable.

## 10) Adoption principles
- Prefer contracts and validation over conventions: make it hard to wire navigation incorrectly by baking checks into editor utilities and runtime asserts.
- Default to deterministic behavior: every transition, confirm/cancel path, and spatial search should be explainable via a trace.
- Make designer workflows first-class: common edits (reordering menus, swapping input prompts, updating theme) should be achievable without code changes or deep stack knowledge.
