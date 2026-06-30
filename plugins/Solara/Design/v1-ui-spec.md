# Solara UI Specification v1
**Framework:** Visage C++
**Version:** v1

---

## Window Dimensions
- **Width:** 900px
- **Height:** 580px
- **Resizable:** No (fixed aspect ratio for v1)

---

## Layout Overview

```
┌────────────────────────────────────────────────────────────────────┐
│  HEADER BAR  [SOLARA logo]          [Bank ▾]  [Preset ◀ ▶]  [≡]  │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  ┌─ PRESET BROWSER ──────────────────────────────────────────┐    │
│  │  Bank tabs: [USER] [KEYS] [PADS] [LEADS] [STRINGS] [FX]  │    │
│  │  Scrollable preset list (name + category tag)             │    │
│  └───────────────────────────────────────────────────────────┘    │
│                                                                    │
│  ┌─ SYNTH ENGINE ──────────────────────────────┐                  │
│  │                                             │                  │
│  │   MASTER        AMP ENV         FILTER      │                  │
│  │   [VOL] [PAN]   [A][D][S][R]   [CUT][RES]  │                  │
│  │                                [TYPE] [ENV] │                  │
│  │                                             │                  │
│  │   PITCH         LFO             DYNAMICS    │                  │
│  │   [CRS][FIN]   [RATE][DEPTH]   [VEL][KEY]  │                  │
│  │                [SHP] [TGT]                  │                  │
│  │                                             │                  │
│  └─────────────────────────────────────────────┘                  │
│                                                                    │
│  ┌─ ENVELOPE DISPLAY ─────────────────────────────────────┐       │
│  │  Real-time ADSR curve visualizer (amp envelope)        │       │
│  └────────────────────────────────────────────────────────┘       │
│                                                                    │
│  ┌─ FX CHAIN ─────────────────────────────────────────────────┐   │
│  │  REVERB               CHORUS                DELAY           │   │
│  │  [MIX][SIZE][DAMP]   [MIX][RATE][DEPTH]   [MIX][TIME][FB] │   │
│  └────────────────────────────────────────────────────────────┘   │
│                                                                    │
│  ┌─ STATUS BAR ──────────────────────────────────────────────┐    │
│  │  [POLY: 32]  [MONO ○]  [PORT: 0.00s]  [CPU: 0%]          │    │
│  └───────────────────────────────────────────────────────────┘    │
└────────────────────────────────────────────────────────────────────┘
```

---

## Sections & Controls

### 1. Header Bar (900 × 48px)
| Element | Type | Position | Notes |
|---|---|---|---|
| SOLARA wordmark | Label | Left, 16px padding | Inter Bold 18px, white |
| Bank dropdown | Button | Center-right | Graphite panel, chevron icon |
| Prev/Next preset | Buttons | Right cluster | Arrow icons, 28×28px |
| Menu icon | Button | Far right | Hamburger / 3-line |

### 2. Preset Browser (900 × 120px)
| Element | Type | Notes |
|---|---|---|
| Bank tabs | Tab row | 6 tabs: USER / KEYS / PADS / LEADS / STRINGS / FX |
| Preset list | Scrollable list | Name left-aligned, category tag right, 28px row height |
| Search field | Text input | Above list, subtle border, placeholder "Search presets…" |

### 3. Synth Engine Panel (900 × 200px)
Organized in a 3-column × 2-row grid with labeled sub-sections.

**Row 1 — Left: Master**
| Control | Type | Size | Notes |
|---|---|---|---|
| master_volume | BrushedKnob | 52px | Label "VOL" below |
| master_pan | BrushedKnob | 52px | Label "PAN" below, center detent |

**Row 1 — Center: Amp Envelope**
| Control | Type | Size |
|---|---|---|
| amp_attack | BrushedKnob | 44px |
| amp_decay | BrushedKnob | 44px |
| amp_sustain | BrushedKnob | 44px |
| amp_release | BrushedKnob | 44px |

**Row 1 — Right: Filter**
| Control | Type | Size | Notes |
|---|---|---|---|
| filter_cutoff | BrushedKnob | 52px | Largest in section |
| filter_resonance | BrushedKnob | 44px | |
| filter_type | SegmentButton | 4 segs | LP12 / LP24 / HP / BP |
| filter_env_amount | BrushedKnob | 36px | Bipolar, center detent |

**Row 2 — Left: Pitch**
| Control | Type | Notes |
|---|---|---|
| pitch_coarse | BrushedKnob | Bipolar |
| pitch_fine | BrushedKnob | Bipolar |
| pitch_bend_range | SmallKnob | Integer |

**Row 2 — Center: LFO**
| Control | Type | Notes |
|---|---|---|
| lfo_rate | BrushedKnob | |
| lfo_depth | BrushedKnob | |
| lfo_shape | IconButton row | 5 shape icons |
| lfo_target | SegmentButton | PITCH / FILT / VOL |

**Row 2 — Right: Dynamics**
| Control | Type | Notes |
|---|---|---|
| velocity_sensitivity | BrushedKnob | |
| velocity_curve | SegmentButton | LIN / SOFT / HARD / FIX |
| key_tracking | SmallKnob | |

### 4. Envelope Display (900 × 60px)
- Real-time ADSR curve drawn from live param values
- Thin white curve on dark graphite background
- Attack/Decay/Release shown as time segments, Sustain as level plateau
- Smooth animation when parameters move

### 5. FX Chain Panel (900 × 100px)
Three equal sub-panels, separated by subtle dividers.

**Reverb**
| Control | Notes |
|---|---|
| reverb_mix | BrushedKnob 44px |
| reverb_size | BrushedKnob 44px |
| reverb_damp | BrushedKnob 44px |

**Chorus**
| Control | Notes |
|---|---|
| chorus_mix | BrushedKnob 44px |
| chorus_rate | BrushedKnob 44px |
| chorus_depth | BrushedKnob 44px |

**Delay**
| Control | Notes |
|---|---|
| delay_mix | BrushedKnob 44px |
| delay_time | BrushedKnob 44px |
| delay_feedback | BrushedKnob 44px |
| delay_sync | LED toggle button |

### 6. Status Bar (900 × 32px)
Small readout chips — polyphony count, mono toggle, portamento time, CPU %.

---

## Spacing & Grid

- Global padding: 16px
- Section inner padding: 12px
- Knob-to-label gap: 6px
- Section corner radius: 8px
- Sub-section divider: 1px, 15% white opacity
- Row gap between knob groups: 16px

---

## Control Sizing Reference

| Size Class | Diameter | Usage |
|---|---|---|
| XL | 60px | Master volume |
| L | 52px | Cutoff, main send knobs |
| M | 44px | ADSR, FX knobs |
| S | 36px | Mod amount, fine controls |
| XS | 28px | Bend range, key tracking |
