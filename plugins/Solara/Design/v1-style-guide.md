# Solara Style Guide v1
**Framework:** Visage C++

---

## Color Palette

### Base / Structure
| Role | Name | Hex | Usage |
|---|---|---|---|
| Background | Void Black | `#111213` | Window background, deepest layer |
| Panel | Graphite Dark | `#1A1C1E` | Section panel fills |
| Panel Raised | Graphite Mid | `#222528` | Nested sub-panels, header |
| Divider | Graphite Line | `#FFFFFF18` | 1px section borders |
| Surface | Graphite Light | `#2C3035` | Knob face, button backgrounds |

### Text
| Role | Hex | Notes |
|---|---|---|
| Primary | `#E8E8EA` | Labels, values |
| Secondary | `#8A8D91` | Section headers, status text |
| Disabled | `#4A4D52` | Inactive controls |
| Accent Label | `#C8C9CC` | Active knob value readout |

### Accent (used sparingly)
| Role | Hex | Usage |
|---|---|---|
| Primary Accent | `#5B9CF6` | Active control ring, selected preset |
| Accent Dim | `#3A6BB5` | Hover state ring |
| Warm Accent | `#D4955A` | Vintage warmth — portamento, mono LED |
| FX Active | `#4FC38A` | FX on indicators |

### Knob Colors
| Element | Hex | Notes |
|---|---|---|
| Knob body | `#2A2D32` | Matte dark graphite |
| Knob cap | `#3C4046` | Slightly lighter brushed center |
| Knob ring track | `#FFFFFF14` | Dim arc (full travel range) |
| Knob ring value | `#5B9CF6` | Active arc (0 → current value) |
| Knob indicator line | `#E8E8EA` | White tick mark, 2px |
| Knob shadow | `#00000066` | Drop shadow, 4px blur |

---

## Typography

**Primary Font:** Inter (embedded as binary asset)
**Fallback:** System sans-serif

| Role | Weight | Size | Tracking |
|---|---|---|---|
| Plugin name (wordmark) | Bold 700 | 18px | +0.05em |
| Section header | Medium 500 | 10px | +0.12em uppercase |
| Control label | Regular 400 | 10px | +0.06em |
| Value readout | Mono (JetBrains Mono) | 11px | 0 |
| Preset name | Regular 400 | 13px | 0 |
| Bank tab | Medium 500 | 11px | +0.08em |
| Status bar | Regular 400 | 10px | 0 |

---

## Knob Design: `BrushedKnob`

### Structure (drawn via `visage::Frame::draw()`)
```
Outer ring track    — thin arc, 270° sweep, dim graphite
Outer ring value    — same arc, 0° to current value, accent blue
Knob body           — circle, matte graphite (#2A2D32)
Knob cap highlight  — inner circle 60% diameter, slightly lighter
Brushed texture     — horizontal noise lines at 10% opacity (shader or bitmap)
Indicator line      — 2px white line from center to 75% radius
Drop shadow         — offset 0,2px, blur 4px, 40% black
```

### Knob Sweep
- Travel: 270° (135° left → 135° right, zero at 7 o'clock)
- Bipolar controls: zero at 12 o'clock (pan, coarse, fine, filter env)
- Interaction: click-drag vertical (up = increase), double-click to reset
- Value tooltip: appears 8px above knob on hover, fades after 1.5s

### Animation
- Value arc: smooth interpolation, 16ms easing
- Value tooltip: fade in 80ms, fade out 300ms

---

## Panel Design

### Section Panels
```
Background fill:    #1A1C1E
Border:             1px #FFFFFF18
Corner radius:      8px
Inner padding:      12px
Drop shadow:        0 2px 8px #00000044
```

### Section Header Label
```
Text:       uppercase, 10px, Inter Medium, #8A8D91
Position:   top-left of panel, 12px from edges
Underline:  none
```

---

## Buttons

### Segment Buttons (Filter Type, LFO Target, Vel Curve)
```
Height:             22px
Corner radius:      4px
Inactive fill:      #2C3035
Active fill:        #3A4A6A
Active text:        #E8E8EA
Inactive text:      #6A6D72
Border:             1px #FFFFFF14
Font:               Inter Medium 10px, +0.08em tracking
```

### LFO Shape Icon Buttons
```
Size:               24×24px
Icon:               SVG path drawn via Visage (sine wave, triangle, saw, square, stepped)
Active:             icon fill #5B9CF6
Inactive:           icon fill #4A4D52
Hover:              icon fill #8A9FC4
```

### LED Toggle (Delay Sync, Mono Mode)
```
Size:               10px circle
Active color:       #4FC38A (green)
Inactive color:     #2C3035
Glow on active:     radial gradient, 4px radius, 30% opacity green
Label:              right-aligned 10px text
```

---

## Preset Browser

### Bank Tabs
```
Height:             28px
Active tab:         #222528 fill, top border 2px #5B9CF6
Inactive tab:       transparent, no border
Font:               Inter Medium 11px, +0.08em
Active text:        #E8E8EA
Inactive text:      #8A8D91
Hover text:         #C8C9CC
```

### Preset List Row
```
Height:             28px
Normal fill:        transparent
Hover fill:         #FFFFFF08
Selected fill:      #1E2C4A
Selected border:    left 2px #5B9CF6
Name text:          Inter Regular 13px, #E8E8EA
Category tag:       right-aligned, 10px, #8A8D91
```

---

## Envelope Display

```
Background:         #141618
Curve color:        #E8E8EA at 80% opacity
Curve width:        1.5px
Fill below curve:   gradient, #5B9CF6 at 15% → transparent
Grid lines:         4 vertical (time), 4 horizontal (level), #FFFFFF08
Corner radius:      4px
```

---

## FX Panel Sub-panels

```
Dividers between sections: 1px #FFFFFF14 vertical line
Sub-section label: centered above knob group, 10px uppercase #6A6D72
Active FX indicator: small green LED top-right corner when mix > 0
```

---

## Header Bar

```
Height:             48px
Fill:               #1A1C1E
Bottom border:      1px #FFFFFF18
Logo font:          Inter Bold 18px, #E8E8EA, letter-spacing +0.05em
Logo accent:        "SOL" in #E8E8EA, "ARA" in #5B9CF6 (subtle)
```

---

## Status Bar

```
Height:             32px
Fill:               #141618
Top border:         1px #FFFFFF10
Chip fill:          #222528
Chip border:        1px #FFFFFF12
Chip corner:        4px
Chip font:          JetBrains Mono Regular 10px, #8A8D91
Chip active value:  #C8C9CC
Spacing between chips: 12px
```

---

## Shadows & Elevation

| Layer | Shadow |
|---|---|
| Window | none (host handles) |
| Section panels | 0 2px 8px #00000044 |
| Raised controls | 0 1px 4px #00000055 |
| Active preset row | 0 0 0 1px #5B9CF620 |
| Knobs | 0 2px 4px #00000066 |

---

## Motion & Animation

| Interaction | Duration | Easing |
|---|---|---|
| Knob value arc | 16ms | Linear |
| Knob value tooltip appear | 80ms | Ease-out |
| Knob value tooltip fade | 300ms | Ease-in |
| Preset selection highlight | 100ms | Ease-out |
| Envelope curve redraw | 32ms | Ease-out |
| Bank tab switch | 120ms | Ease-in-out |
| LED glow pulse (active) | None (static) | — |

---

## Do / Don't

| ✅ Do | ❌ Don't |
|---|---|
| Use accent blue only for active states | Use blue as a fill color |
| Keep section labels tiny and secondary | Make labels compete with control labels |
| Use subtle shadows for depth | Add heavy outlines or glow effects |
| Draw knobs with brushed texture | Use flat circles with no detail |
| Keep spacing generous (≥12px gap) | Crowd controls together |
| Animate curves smoothly | Snap values without transition |
