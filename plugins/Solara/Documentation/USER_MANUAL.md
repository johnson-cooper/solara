# Solara — User Manual
**Version 1.0.0**

---

## Overview

Solara is a professional-grade rompler VST3 that reads `.slr` soundfont files — a custom compressed format converted from industry-standard SF2 and SFZ soundfonts. It delivers studio-quality sample playback at 40–70% smaller file sizes than raw SF2.

---

## Installation

### Windows
1. Run `Solara-Setup.exe` and follow the installer prompts.
2. The VST3 is installed to `C:\Program Files\Common Files\VST3\`.
3. Restart your DAW and scan for new plugins.

### macOS
1. Open `Solara-macOS.pkg` and follow the prompts.
2. VST3 goes to `/Library/Audio/Plug-Ins/VST3/`, AU to `/Library/Audio/Plug-Ins/Components/`.
3. Restart your DAW.

### Linux
1. Copy `Solara.vst3` to `~/.vst3/` or `/usr/lib/vst3/`.
2. Restart your DAW and scan for new plugins.

---

## Adding Soundfonts

Solara reads `.slr` files — convert your SF2/SFZ files using the bundled `slr-convert` tool:

```
slr-convert piano.sf2
slr-convert strings.sfz strings.slr --verbose
```

Place the resulting `.slr` files in:

| Platform | Preset Folder |
|---|---|
| Windows | `Documents\Solara\Presets\` |
| macOS | `~/Documents/Solara/Presets/` |
| Linux | `~/Documents/Solara/Presets/` |

Solara scans this folder on startup. Presets appear in the browser immediately after loading the plugin.

---

## Interface

### Preset Browser
- **Bank tabs** — filter presets by category (USER / KEYS / PADS / LEADS / STRINGS / FX)
- **Arrow buttons** — step through presets
- **Search** — type to filter preset names

### Amp Envelope (ADSR)
Controls the amplitude shape of each note.
- **ATK** — time to reach full volume after key press (0.001s – 10s)
- **DEC** — time to fall from peak to sustain level (0.001s – 10s)
- **SUS** — volume held while key is held (0–100%)
- **REL** — time to fade after key release (0.001s – 20s)

### Filter
- **CUTOFF** — filter frequency (20Hz – 20kHz, log scale)
- **RES** — resonance / Q (adds harmonic emphasis at cutoff)
- **ENV** — how much the filter envelope modulates cutoff (bipolar)
- **Type** — LP12 / LP24 / HP / BP

### Pitch
- **COARSE** — semitone offset (–24 to +24)
- **FINE** — cent offset (–100 to +100)
- **BEND** — pitch wheel range in semitones (0–24)

### LFO
- **RATE** — oscillation speed (0.1 – 20 Hz)
- **DEPTH** — modulation amount (0–100%)
- **Shape** — Sine / Triangle / Saw / Square / S&H
- **Target** — PITCH / FILTER / VOLUME

### FX Chain
Three global effects applied after all voices mix.

| Section | Controls |
|---|---|
| Reverb | Mix, Room Size, Damping |
| Chorus | Mix, Rate, Depth |
| Delay | Mix, Time, Feedback, Tempo Sync |

### Dynamics
- **VEL** — how much MIDI velocity affects volume (0 = fixed, 1 = full range)
- **Curve** — velocity response shape: Linear / Soft / Hard / Fixed
- **KEY** — filter key tracking (0 = none, 1 = full, cutoff follows note pitch)

### Master
- **VOL** — global output level
- **PAN** — stereo panning
- **POLY** — maximum simultaneous voices (1–128)
- **MONO** — monophonic legato mode
- **PORT** — portamento glide time (mono mode only)

---

## Tips

- Use **LP24** for warmer, more synth-like filter sweeps.
- **S&H** LFO on Filter creates classic sample-and-hold arpeggio effects.
- Set **ENV** amount to a negative value for filter closing on attack (reversed sweep).
- Lower **VEL sensitivity** for more consistent dynamics across MIDI controllers.
- **Tempo Sync** on Delay locks delay time to your DAW's BPM.

---

## Troubleshooting

**No presets appear in the browser**
→ Check that `.slr` files exist in `Documents/Solara/Presets/`. Rescan by reloading the plugin.

**No sound when pressing keys**
→ Verify MIDI input is routed to Solara in your DAW. Check that the active preset has loaded (preset name shown in header bar).

**Noise or crackling**
→ Increase your DAW's audio buffer size. Solara uses up to 128 voices; reduce **POLY** if CPU is high.

---

## Credits

Built with JUCE 8 and Visage.
Solara is part of the Audio Plugin Coder (APC) system.

© 2026 — All rights reserved.
