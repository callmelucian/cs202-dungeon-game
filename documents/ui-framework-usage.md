# SFML UI Framework — Quick Start & Usage Manual

This document provides a quick-start reference and style guide for developers using the custom SFML UI Framework. It focuses on practical code patterns and layouts, enabling you to build complex interfaces without needing to read the framework's internal source code.

---

## Table of Contents
1. [Core Concepts](#1-core-concepts)
2. [Setting Up a Screen (GameState)](#2-setting-up-a-screen-gamestate)
3. [Creating Components](#3-creating-components)
4. [Sizing Modes (`SizeMode`)](#4-sizing-modes-sizemode)
5. [Fluent Setter Interface (Chaining)](#5-fluent-setter-interface-chaining)
6. [Basic Layout Containers (FlexBox)](#6-basic-layout-containers-flexbox)
   - 6.1 [Horizontal Box (`HorizontalBox`)](#61-horizontal-box-horizontalbox)
   - 6.2 [Vertical Box (`VerticalBox`)](#62-vertical-box-verticalbox)
   - 6.3 [Spacing & Distributions](#63-spacing--distributions)
   - 6.4 [Cross-Axis Alignment](#64-cross-axis-alignment)
7. [Working with Widgets](#7-working-with-widgets)
   - 7.1 [Text (`UI::Text`)](#71-text-uitext)
   - 7.2 [Buttons (`UI::Button`)](#72-buttons-uibutton)
   - 7.3 [Sliders (`UI::Slider`)](#73-sliders-uislider)
   - 7.4 [Text Inputs (`UI::TextInput`)](#74-text-inputs-uitextinput)
8. [Child Defaults System](#8-child-defaults-system)
9. [Event & Update Propagation](#9-event--update-propagation)
10. [Common UI Layout Examples](#10-common-ui-layout-examples)

---

## 1. Core Concepts

The UI Framework organizes user interface elements in a **tree hierarchy**:
- **Root Node**: A `UI::Container` that matches the window's dimensions.
- **Leaf Nodes (Widgets)**: Buttons, sliders, text fields, and static labels.
- **Internal Nodes (Containers)**: Basic containers or directional boxes (`HorizontalBox`, `VerticalBox`) that group and align child components.

Every frame, the hierarchy is processed recursively:
1. **Events** are passed down top-down to handle clicks, hover states, and keystrokes.
2. **Layout Sizing** is resolved top-down (parent constraints constrain child sizes) and bottom-up (children size content-dependent parent size requests).
3. **Drawing** renders parents first, then children in their insertion order.

---

## 2. Setting Up a Screen (GameState)

To create a new game screen (such as main menu, pause overlay, or game-over screen), inherit from `GameState`. The base class manages a `std::unique_ptr<UI::Container> root` node matching the window width and height.

```cpp
#include "../game-state.hpp"
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"

class MyMenuState : public GameState {
public:
    MyMenuState(StateManager& manager) : GameState(manager) {
        // Build your UI here using the root pointer
        layoutBox = root->createChild<UI::VerticalBox>()
            ->setModeX(UI::SizeMode::Expanded)
            ->setModeY(UI::SizeMode::Expanded);
    }
private:
    UI::VerticalBox* layoutBox;
};
```

---

## 3. Creating Components

Never instantiate UI components manually with `new` or `std::make_unique`. Always create them as children of an existing container using the `createChild<T>(ConstructorArgs...)` method:

```cpp
// Correct: Adding a child text element to layoutBox
UI::Text* label = layoutBox->createChild<UI::Text>("regular", 24);
```

---

## 4. Sizing Modes (`SizeMode`)

Each component has separate sizing modes for the X-axis and Y-axis, set via `setModeX()` and `setModeY()`.

| SizeMode | Behavior |
| :--- | :--- |
| **`SizeMode::Fixed`** | Uses a user-specified length set via `setFixedWidth()` or `setFixedHeight()`. |
| **`SizeMode::Contained`** | Auto-wraps. The size is calculated bottom-up to exactly contain all child components. For leaf widgets like `UI::Text`, this auto-sizes the component to the exact text bounding box. |
| **`SizeMode::Expanded`** | Fills all remaining space provided by the parent container. |

---

## 5. Fluent Setter Interface (Chaining)

The framework utilizes a Curiously Recurring Template Pattern (CRTP) to keep setters type-safe while supporting convenient chaining syntax. Setters always return a pointer to the derived class:

```cpp
UI::Button* myBtn = layoutBox->createChild<UI::Button>("Click Me", "regular")
    ->setModeX(UI::SizeMode::Fixed)
    ->setFixedWidth(200.f)
    ->setModeY(UI::SizeMode::Fixed)
    ->setFixedHeight(50.f)
    ->setMarginTop(10.f);
```

---

## 6. Basic Layout Containers (FlexBox)

Use `FlexBox` templates to automatically sequence components. There are two standard type aliases:

### 6.1 Horizontal Box (`HorizontalBox`)
Places children sequentially from **left to right** along the X-axis.
```cpp
UI::HorizontalBox* row = container->createChild<UI::HorizontalBox>();
```

### 6.2 Vertical Box (`VerticalBox`)
Places children sequentially from **top to bottom** along the Y-axis.
```cpp
UI::VerticalBox* col = container->createChild<UI::VerticalBox>();
```

### 6.3 Spacing & Distributions
Align children along the main axis of a `FlexBox` using `setDistribution(UI::Distribution)` and control the gap size via `setSpacing(float)`:

- `setSpacing(20.f)`: Sets a fixed pixel gap between all children.
- **Distributions** (only apply if no children are `Expanded`):
  - `Distribution::FlexStart`: Packs children tightly at the start (default).
  - `Distribution::FlexEnd`: Packs children tightly at the end.
  - `Distribution::SpaceBetween`: Spreads children evenly; no outer gaps.
  - `Distribution::SpaceAround`: Spreads children evenly with half-size outer gaps.
  - `Distribution::SpaceEvenly`: Spreads children evenly with equal outer gaps.

### 6.4 Cross-Axis Alignment
Align children along the secondary (cross) axis using the parent container's alignment setters:
- **For `HorizontalBox`** (cross-axis is Y): Use `setAlignmentY(UI::AlignmentY)` with `Top`, `Middle`, or `Bottom`.
- **For `VerticalBox`** (cross-axis is X): Use `setAlignmentX(UI::AlignmentX)` with `Left`, `Center`, or `Right`.

```cpp
layoutBox->setAlignmentX(UI::AlignmentX::Center); // Centers all children horizontally
```

---

## 7. Working with Widgets

### 7.1 Text (`UI::Text`)
Constructed with a font key registered in the `AssetManager` (`"regular"`, `"bold"`, `"italic"`, `"bold-italic"`) and a character size:

```cpp
UI::Text* title = layoutBox->createChild<UI::Text>("bold", 48)
    ->setString("Settings Menu")
    ->setFillColor(sf::Color::White);
```

### 7.2 Buttons (`UI::Button`)
Constructed with a label string, font key, and character size:

```cpp
UI::Button* startBtn = layoutBox->createChild<UI::Button>("Start Game", "regular")
    ->setFixedSize(sf::Vector2f(200.f, 50.f));

// Set callback triggered on click
startBtn->setOnClick([]() {
    std::cout << "Button clicked!" << std::endl;
});
```

### 7.3 Sliders (`UI::Slider`)
Constructed with minimum value, maximum value, and initial value:

```cpp
UI::Slider* volumeSlider = layoutBox->createChild<UI::Slider>(0.f, 100.f, 75.f)
    ->setFixedSize(sf::Vector2f(300.f, 30.f));

volumeSlider->setOnValueChanged([](float newVal) {
    std::cout << "New volume: " << newVal << std::endl;
});
```

### 7.4 Text Inputs (`UI::TextInput`)
Constructed with a font key and character size:

```cpp
UI::TextInput* nameInput = layoutBox->createChild<UI::TextInput>("regular", 16)
    ->setFixedSize(sf::Vector2f(250.f, 40.f));

std::string enteredText = nameInput->getString();
```

---

## 8. Child Defaults System

To avoid writing repetitive setters for multiple siblings (e.g., configuring ten identical buttons in a row), you can apply defaults to a parent container. Children created after the defaults are set will inherit them:

```cpp
UI::HorizontalBox* row = layoutBox->createChild<UI::HorizontalBox>();

// Set default styles for all children added to this row
row->setChildDefaults({
    .modeX = UI::SizeMode::Fixed,
    .modeY = UI::SizeMode::Fixed,
    .fixedWidth = 150.f,
    .fixedHeight = 45.f,
    .margin = UI::Margin{.top = 5.f, .bottom = 5.f}
});

// These buttons automatically inherit the 150x45 fixed sizing and margins!
auto btnA = row->createChild<UI::Button>("A", "regular");
auto btnB = row->createChild<UI::Button>("B", "regular");

// Individual overrides still work!
auto customBtn = row->createChild<UI::Button>("Wide Button", "regular")
    ->setFixedWidth(300.f); // Overrides the default width of 150.f
```

You can also use temporary defaults using scoping methods:
```cpp
container->pushChildDefaults(defaults);
// ... create children ...
container->popChildDefaults();
```

---

## 9. Event & Update Propagation

The base `GameState` automatically handles pipeline propagation. However, if you are creating a custom container class:
- You must forward `draw(target)`, `handleEvent(event)`, `update(dt)`, and `onColorPaletteChanged(palette)` down to your children.
- You must call `computeSize(constraint)` and `setPosition(pos)` recursively.

---

## 10. Common UI Layout Examples

### Example A: A Centered Vertical Main Menu
A title text on top, and a cluster of horizontal buttons centered at the bottom of the screen.

```cpp
MainMenuState::MainMenuState(StateManager& manager) : GameState(manager) {
    // 1. Create vertical flexbox spanning the entire screen
    UI::VerticalBox* screenLayout = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Expanded)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setDistribution(UI::Distribution::SpaceEvenly);

    // 2. Add title
    screenLayout->createChild<UI::Text>("bold", 54)
        ->setString("My Game Title");

    // 3. Add horizontal button box centered in screenLayout
    UI::HorizontalBox* btnBox = screenLayout->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(20.f);

    btnBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 180.f,
        .fixedHeight = 50.f
    });

    // 4. Create buttons
    UI::Button* playBtn = btnBox->createChild<UI::Button>("Play", "regular");
    UI::Button* exitBtn = btnBox->createChild<UI::Button>("Exit", "regular");
    
    // (Bind callbacks in your state manager controller code)
}
```
