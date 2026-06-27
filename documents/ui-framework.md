# SFML UI Framework — Technical Reference

> A Tailwind CSS–inspired, tree-based UI framework built on top of SFML. Components are arranged in a hierarchy; sizing, alignment, and spacing are resolved top-down and rendered in insertion order.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Abstract Base — `UI::Component`](#2-abstract-base--uicomponent)
   - 2.1 [Sizing Modes](#21-sizing-modes)
   - 2.2 [Color Palette Observer](#22-color-palette-observer)
3. [Text — `UI::Text`](#3-text--uitext)
4. [Widgets](#4-widgets)
   - 4.1 [`UI::Button`](#41-uibutton)
   - 4.2 [`UI::Slider`](#42-uislider)
   - 4.3 [`UI::TextInput`](#43-uitextinput)
5. [Containers](#5-containers)
   - 5.1 [`UI::Container`](#51-uicontainer)
     - 5.1.1 [Sizing Modes](#511-sizing-modes)
     - 5.1.2 [Alignment](#512-alignment)
     - 5.1.3 [Box Model — Padding & Margin](#513-box-model--padding--margin)
     - 5.1.4 [Size Resolution Algorithm](#514-size-resolution-algorithm)
   - 5.2 [`UI::FlexBox`](#52-uiflexbox)
     - 5.2.1 [Template Instantiation & Type Aliases](#521-template-instantiation--type-aliases)
     - 5.2.2 [Main-Axis Distribution (no `Expanded` children)](#522-main-axis-distribution-no-expanded-children)
     - 5.2.3 [Main-Axis Distribution (with `Expanded` children)](#523-main-axis-distribution-with-expanded-children)
     - 5.2.4 [Cross-Axis Alignment](#524-cross-axis-alignment)
     - 5.2.5 [Overflow Handling](#525-overflow-handling)
     - 5.2.6 [Child Position Computation](#526-child-position-computation)
6. [Notation & Symbol Reference](#6-notation--symbol-reference)

---

## 1. Overview

The framework models UI as a **rooted tree** where every node is a `UI::Component`. Leaf nodes are widgets (buttons, sliders, text inputs) or bare text; internal nodes are containers (`UI::Container`, `UI::FlexBox`). The tree is traversed:

- **Top-down** to resolve sizes and positions (parents constrain children).
- **Bottom-up** to compute `Contained` sizes (children inform parents).
- **In insertion order** for rendering and hit-testing.

Every frame the root calls `update(dt)`, `handleEvent(event)`, and `draw(target)`, all of which propagate recursively to every descendant.

---

## 2. Abstract Base — `UI::Component`

`UI::Component` is a **pure abstract class**. All framework types inherit from it and must implement its virtual interface.

### Virtual Interface

| Method | Signature | Description |
|---|---|---|
| `draw` | `virtual void draw(sf::RenderTarget&) const = 0` | Render self and (for containers) children |
| `handleEvent` | `virtual void handleEvent(const sf::Event&) = 0` | Propagate SFML events |
| `update` | `virtual void update(float dt) = 0` | Per-frame logic (animations, cursor blink, etc.) |
| `onPaletteChanged` | `virtual void onPaletteChanged(const ColorPalette&) = 0` | React to theme changes |
| `computeSize` | `virtual void computeSize(sf::Vector2f availableSize) = 0` | Resolve own size given parent constraint |
| `setPosition` | `virtual void setPosition(sf::Vector2f pos) = 0` | Absolute top-left position in canvas space |

### Data Members

| Member | Type | Description |
|---|---|---|
| `parent` | `UI::Component*` | Raw (non-owning) pointer to parent; `nullptr` for root |
| `position` | `sf::Vector2f` | Resolved top-left position in canvas space |
| `size` | `sf::Vector2f` | Resolved size after `computeSize` |
| `modeX` | `SizeMode` | Sizing mode along the x-axis |
| `modeY` | `SizeMode` | Sizing mode along the y-axis |

---

### 2.1 Sizing Modes

The `SizeMode` enum applies **independently** to each axis.

```
enum class SizeMode { Fixed, Contained, Expanded };
```

For `UI::Component` (non-container leaf nodes):

| Mode | Behaviour |
|---|---|
| `Fixed` | The length along that axis is set to a user-specified value $L_\text{fixed}$ |
| `Contained` | The length is forced to **0** (a bare component has no children to contain) |
| `Expanded` | The length is set to the **available space** $A$ passed by the parent |

> `Contained` on a leaf is intentionally degenerate — it is meaningful only when overridden by `UI::Container` (see §5.1.1).

---

### 2.2 Color Palette Observer

`UI::Component` implements the **Observer** side of an observer pattern. A global (or scene-scoped) `ColorPaletteSubject` holds the active `ColorPalette`; when the palette changes it iterates every registered component and calls `onPaletteChanged(newPalette)`.

```
class ColorPaletteObserver {
public:
    virtual void onPaletteChanged(const ColorPalette& palette) = 0;
};
```

`UI::Component` inherits from `ColorPaletteObserver` and registers/deregisters itself in its constructor/destructor.

---

### 2.3 Draw Order

Children are drawn in **insertion order** — the first registered child is drawn first (behind), the last is drawn on top. To control layering, register children in the desired back-to-front order.

Rendering pseudocode:

```cpp
void Container::draw(sf::RenderTarget& target) const {
    // draw self background first
    for (auto& child : children)
        child->draw(target);
}
```

---

## 3. Text — `UI::Text`

`UI::Text` is a thin wrapper around `sf::Text` that sources its font from `AssetManager` rather than requiring the caller to manage `sf::Font` lifetimes.

```cpp
class Text : public Component {
public:
    explicit Text(std::string_view fontKey, unsigned int characterSize = 16);
    void setString(const sf::String& str);
    void setCharacterSize(unsigned int size);
    // ... forwarded sf::Text setters
private:
    sf::Text text;       // underlying SFML text object
    std::string fontKey; // key used to look up font in AssetManager
};
```

**Size computation:** Both `modeX` and `modeY` default to `Fixed`. When resolved, the component queries `text.getLocalBounds()` and uses those dimensions if no explicit fixed size is provided.

---

## 4. Widgets

All widgets inherit from `UI::Component` and implement the full virtual interface. They follow standard GUI conventions for event handling.

### 4.1 `UI::Button`

A pressable region with optional label text.

**States:** `Normal`, `Hovered`, `Pressed`, `Disabled`.

**Event handling:**
- `MouseMoved` → update hover state.
- `MouseButtonPressed` (left) → if pointer is inside bounds, transition to `Pressed`; fire `onPressed` callback.
- `MouseButtonReleased` (left) → if still inside bounds, fire `onClick` callback; transition to `Hovered` or `Normal`.

**Default sizing:** `Fixed` on both axes; width and height set by the user. The internal label `UI::Text` is centered within the button bounds.

---

### 4.2 `UI::Slider`

A draggable track-and-thumb widget representing a continuous value in $[v_\text{min},\, v_\text{max}]$.

**Layout:**

```
|<---- track length T ---->|
[========|=================]
         ^ thumb at pixel offset p
```

Let $v$ be the current value. The **thumb pixel offset** from the track start is:

$$p = \frac{v - v_{\min}}{v_{\max} - v_{\min}} \cdot T$$

Conversely, when the user drags the thumb to pixel offset $p'$:

$$v' = v_{\min} + \frac{p'}{T} \cdot (v_{\max} - v_{\min}), \quad v' \in [v_{\min},\, v_{\max}]$$

**Event handling:**
- `MouseButtonPressed` (left) on thumb → begin drag; record $\Delta x = p_\text{mouse} - p_\text{thumb}$.
- `MouseMoved` while dragging → update $p' = p_\text{mouse} - \Delta x$, clamp to $[0, T]$, recompute $v'$.
- `MouseButtonReleased` → end drag.

---

### 4.3 `UI::TextInput`

A single-line editable text field with a blinking text cursor.

**Cursor:** An `sf::RectangleShape` of width 1–2 px and height equal to the character size. Its blink period is configurable (default 530 ms on / 530 ms off). The cursor is only visible when the widget has **keyboard focus**.

**Cursor position:** Let the string be $S$ with $n$ characters. The cursor sits between characters $i$ and $i+1$ (where $i = 0$ means before the first character). Its x-offset is obtained from `sf::Text::findCharacterPos(i)`.

**Event handling:**
- `MouseButtonPressed` (left) inside bounds → acquire focus; set cursor to the character nearest the click x.
- `TextEntered` (while focused) → insert `sf::Event::text.unicode` at cursor; advance cursor.
- `KeyPressed` while focused:
  - `Left` / `Right` → move cursor by 1 character.
  - `Home` / `End` → jump to start/end.
  - `Backspace` → delete character before cursor.
  - `Delete` → delete character after cursor.
  - `Escape` → lose focus.
- Click outside bounds → lose focus.

---

## 5. Containers

Containers are the structural backbone of the UI tree. They own their children (via `std::vector<std::unique_ptr<UI::Component>>`), propagate all virtual calls, and implement size/position resolution logic.

---

## 5.1 `UI::Container`

### 5.1.1 Sizing Modes

`UI::Container` overrides the meaning of `Contained`:

| Mode | Behaviour |
|---|---|
| `Fixed` | Length along that axis = $L_\text{fixed}$ (user-set; default `50.f`) |
| `Contained` | Length = minimum size to wrap all children (see §5.1.4). If any child is `Expanded` in that axis, or if the container has no children, the length for that axis is `0`. |
| `Expanded` | Length = available space $A$ passed by the parent. **Disabled at root level:** calling `setModeExpanded` on the root container is a no-op. |

**Root constraint:** The root container's available size is the canvas/window size $W \times H$.

---

### 5.1.2 Alignment

Alignment applies only when the container's mode is `Fixed` or `Expanded` (i.e., the container's size is determined externally and children may be smaller).

**X-axis alignment:**

| Enum | Offset formula |
|---|---|
| `Left` | $x_\text{child} = x_\text{container} + \text{paddingLeft}$ |
| `Center` | $x_\text{child} = x_\text{container} + \dfrac{W_\text{container} - W_\text{child}}{2}$ |
| `Right` | $x_\text{child} = x_\text{container} + W_\text{container} - W_\text{child} - \text{paddingRight}$ |

**Y-axis alignment:**

| Enum | Offset formula |
|---|---|
| `Top` | $y_\text{child} = y_\text{container} + \text{paddingTop}$ |
| `Middle` | $y_\text{child} = y_\text{container} + \dfrac{H_\text{container} - H_\text{child}}{2}$ |
| `Bottom` | $y_\text{child} = y_\text{container} + H_\text{container} - H_\text{child} - \text{paddingBottom}$ |

> Alignment is **ignored** when the mode is `Contained` because the container shrinks to fit its children exactly.

---

### 5.1.3 Box Model — Padding & Margin

The box model for each axis follows the standard pattern:

```
|<---------- margin -------->|
         |<-- container -->|
         |  pad  |content| pad  |
```

- **Padding** is internal: it reduces the content area available to children.
- **Margin** is external: it creates space between this container and its siblings or parent edges.

For a container with a known outer size $L$ and padding $(p_\text{start}, p_\text{end})$:

$$L_\text{content} = L - p_\text{start} - p_\text{end}$$

For `Contained` mode, the outer size is computed from content and then padding is added:

$$L = L_\text{content} + p_\text{start} + p_\text{end}$$

---

### 5.1.4 Size Resolution Algorithm

Size resolution is a two-pass process (bottom-up for `Contained`, top-down for `Expanded`):

**Pass 1 — Top-down (Fixed & Expanded):**

```
function resolveSize(container C, availableSize A):
    if C.modeX == Fixed:
        C.width  = C.fixedW
    elif C.modeX == Expanded:
        C.width  = A.x - C.marginLeft - C.marginRight
    // Contained: deferred to Pass 2

    if C.modeY == Fixed:
        C.height = C.fixedH
    elif C.modeY == Expanded:
        C.height = A.y - C.marginTop - C.marginBottom
    // Contained: deferred to Pass 2

    contentArea = (C.width  - C.padLeft - C.padRight,
                   C.height - C.padTop  - C.padBottom)

    for each child in C.children:
        resolveSize(child, contentArea)
```

**Pass 2 — Bottom-up (Contained):**

```
function resolveContained(container C):
    for each child in C.children:
        if child is Container:
            resolveContained(child)

    if C.modeX == Contained:
        if any child has modeX == Expanded or C has no children:
            C.width = 0
        else:
            C.width  = max(child.width  + child.marginLeft + child.marginRight  for child in C.children)
                       + C.padLeft + C.padRight

    if C.modeY == Contained:
        if any child has modeY == Expanded or C has no children:
            C.height = 0
        else:
            C.height = max(child.height + child.marginTop  + child.marginBottom for child in C.children)
                       + C.padTop + C.padBottom
```

> `UI::Container` stacks children on top of each other (no automatic layout flow). `UI::FlexBox` (§5.2) applies directional layout.

---

## 5.2 `UI::FlexBox`

`UI::FlexBox` extends `UI::Container` with a **directional layout** that arranges children sequentially along a **main axis**, with alignment along the **cross axis**.

### 5.2.1 Template Instantiation & Type Aliases

```cpp
template <Axis MainAxis>
class FlexBox : public Container { ... };

using HorizontalBox = FlexBox<Axis::Horizontal>; // main axis = X, cross axis = Y
using VerticalBox   = FlexBox<Axis::Vertical>;   // main axis = Y, cross axis = X
```

`Axis::Horizontal` → children are placed left-to-right.  
`Axis::Vertical` → children are placed top-to-bottom.

The formulas below use **main-axis** length $M$ and **cross-axis** length $C$ to stay axis-agnostic.

---

### 5.2.2 Main-Axis Distribution (no `Expanded` children)

Let:

- $n$ = number of children
- $m_i$ = resolved main-axis length of child $i$ (including its own margins along the main axis: $m_i = \text{size}_i + \text{marginStart}_i + \text{marginEnd}_i$)
- $M_\text{content}$ = main-axis content area of the FlexBox (`FlexBox.mainLength - padStart - padEnd`)
- $S_\text{total} = \sum_{i=0}^{n-1} m_i$ = sum of all children's main-axis extents

**Available gap space:**

$$G = M_\text{content} - S_\text{total}$$

The distribution mode determines how $G$ is allocated as gaps $g_0, g_1, \ldots, g_n$ (there are $n+1$ gap slots: before child 0, between consecutive children, and after child $n-1$).

| Mode | Gap formula | Result |
|---|---|---|
| `FlexStart` | $g_0 = 0,\quad g_i = 0\ (i > 0),\quad g_n = G$ | All children packed to the start |
| `FlexEnd` | $g_0 = G,\quad g_i = 0\ (i > 0),\quad g_n = 0$ | All children packed to the end |
| `SpaceBetween` | $g_0 = 0,\quad g_i = \dfrac{G}{n-1}\ (0 < i < n),\quad g_n = 0$ | Equal gaps between children; no outer gaps |
| `SpaceAround` | $u = \dfrac{G}{n},\quad g_0 = g_n = \dfrac{u}{2},\quad g_i = u\ (0 < i < n)$ | Equal gaps; outer gaps are half of inner |
| `SpaceEvenly` | $g_i = \dfrac{G}{n+1}\quad \forall i$ | All $n+1$ gaps equal |

> **Edge case:** `SpaceBetween` with $n = 1$ is treated as `FlexStart` to avoid division by zero.

---

### 5.2.3 Main-Axis Distribution (with `Expanded` children)

When **at least one** child has `Expanded` mode along the main axis, gap-based distribution is replaced by **proportional expansion**. All `Expanded` children share the available free space equally.

Let:

- $E$ = set of children with `Expanded` mode on the main axis, $|E| = k$
- $F_i$ = fixed main-axis length of non-expanded child $i$ (with margins)
- $s$ = `spacing` attribute of the FlexBox (minimum gap between children)
- $M_\text{content}$ = main-axis content area

**Free space available for expanded children:**

$$F_\text{total} = \sum_{i \notin E} F_i$$

$$\text{SpacingTotal} = (n - 1) \cdot s$$

$$\text{Free} = M_\text{content} - F_\text{total} - \text{SpacingTotal}$$

**Size of each `Expanded` child** (before its own margins):

$$m_\text{expanded} = \frac{\text{Free} - \sum_{j \in E}(\text{marginStart}_j + \text{marginEnd}_j)}{k}$$

> If $m_\text{expanded} < 0$, it is clamped to `0` (overflow condition; see §5.2.5).

Gaps between all adjacent children are set to $s$ (the `spacing` attribute); no outer padding gap is added beyond the container's own `padStart`.

---

### 5.2.4 Cross-Axis Alignment

Each child is aligned independently along the cross axis within the FlexBox's cross-axis content area $C_\text{content}$.

Let $c_i$ = cross-axis length of child $i$ (with its cross-axis margins: $c_i = \text{crossSize}_i + \text{crossMarginStart}_i + \text{crossMarginEnd}_i$).

**For `HorizontalBox` (cross axis = Y), options are `Top`, `Middle`, `Bottom`:**

| Option | Child y-offset from content area start |
|---|---|
| `Top` | $\text{crossMarginStart}_i$ |
| `Middle` | $\dfrac{C_\text{content} - c_i}{2} + \text{crossMarginStart}_i$ |
| `Bottom` | $C_\text{content} - c_i + \text{crossMarginStart}_i$ |

**For `VerticalBox` (cross axis = X), options are `Left`, `Center`, `Right`:**

| Option | Child x-offset from content area start |
|---|---|
| `Left` | $\text{crossMarginStart}_i$ |
| `Center` | $\dfrac{C_\text{content} - c_i}{2} + \text{crossMarginStart}_i$ |
| `Right` | $C_\text{content} - c_i + \text{crossMarginStart}_i$ |

---

### 5.2.5 Overflow Handling

**Overflow** occurs when $S_\text{total} > M_\text{content}$ (or when `Expanded` children compute a negative size).

| Mode | Behaviour |
|---|---|
| `Overflow::Visible` | Children are laid out as normal; they extend beyond the FlexBox's boundary. No clipping occurs. |
| `Overflow::Hidden` | An `sf::View` (or `glScissor`-based scissor rect) is applied to the FlexBox's bounds before rendering children. Any pixels outside the rect are discarded. |

When `Overflow::Hidden` is active:

```cpp
// pseudocode for draw()
sf::View clip = mapToView(position, size, target);
sf::View original = target.getView();
target.setView(clip);
for (auto& child : children)
    child->draw(target);
target.setView(original);
```

---

### 5.2.6 Child Position Computation

After sizes are resolved, positions are assigned in a single left-to-right (or top-to-bottom) pass.

**Notation:**

- $\text{origin}$ = `FlexBox.position + (padStart, crossPadStart)` (top-left of content area)
- $\text{cursor}$ = running offset along the main axis, initialised to $g_0$ (the first gap from §5.2.2, or `0` in `Expanded` mode)

For each child $i$ in order:

$$\text{mainPos}_i = \text{origin}_\text{main} + \text{cursor} + \text{marginStart}_i$$

$$\text{cursor} \mathrel{+}= \text{marginStart}_i + \text{mainSize}_i + \text{marginEnd}_i + g_{i+1}$$

(where $g_{i+1}$ is the gap **after** child $i$, either from §5.2.2 or the fixed `spacing` value $s$)

$$\text{crossPos}_i = \text{origin}_\text{cross} + \text{crossOffset}_i$$

(where $\text{crossOffset}_i$ is derived from §5.2.4)

The final absolute position of child $i$:

$$\text{pos}_i = \begin{cases}
(\text{mainPos}_i,\ \text{crossPos}_i) & \text{HorizontalBox} \\
(\text{crossPos}_i,\ \text{mainPos}_i) & \text{VerticalBox}
\end{cases}$$

---

## 6. Notation & Symbol Reference

| Symbol | Meaning |
|---|---|
| $A$ | Available size vector passed from parent to child during `computeSize` |
| $L$ | Generic length along a single axis |
| $L_\text{fixed}$ | User-specified length for `Fixed` mode |
| $L_\text{content}$ | Length of the content area (after subtracting padding) |
| $M$ | Main-axis length (of FlexBox or child) |
| $C$ | Cross-axis length (of FlexBox or child) |
| $M_\text{content}$ | Main-axis content area of the FlexBox |
| $C_\text{content}$ | Cross-axis content area of the FlexBox |
| $n$ | Number of children in a container |
| $k$ | Number of `Expanded` children in a FlexBox |
| $m_i$ | Main-axis outer extent of child $i$ (size + margins) |
| $c_i$ | Cross-axis outer extent of child $i$ (size + margins) |
| $S_\text{total}$ | $\sum m_i$: total main-axis space consumed by all children |
| $G$ | Free gap space available on the main axis ($M_\text{content} - S_\text{total}$) |
| $g_i$ | Gap allocated before child $i$ (and $g_n$ after the last child) |
| $s$ | `spacing` attribute: minimum gap used when `Expanded` children are present |
| $p_\text{start/end}$ | Padding at the start / end of an axis |
| $v,\, v_{\min},\, v_{\max}$ | Slider value and its bounds |
| $T$ | Slider track length in pixels |
| $p$ | Slider thumb pixel offset along the track |

---

*End of SFML UI Framework Technical Reference.*
