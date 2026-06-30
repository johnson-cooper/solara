# Solara — Creative Brief

## Hook
> "Every sound you've ever needed. Smaller than you thought possible."

Solara is a professional-grade rompler VST3 built for producers across every genre — EDM, orchestral, lo-fi, pop, cinematic. It reads a custom compressed soundfont format (`.slr`) derived from industry-standard SF2 and SFZ files, delivering studio-quality sample playback at a fraction of the disk footprint.

---

## Description

Solara is the rompler that gets out of your way. Load a preset, twist a knob, make music. Under the hood, it's a high-performance sample engine written in JUCE 8 with a Visage C++ UI — no web stack, no overhead.

### The Format: `.slr` (Solara Sound Resource)
- Converts SF2/SFZ at import time using a bundled CLI tool (`slr-convert`)
- Lossless or perceptual compression (user-selectable at conversion)
- Stores velocity layers, loop points, sample maps, and modulation envelopes
- Target: 40–70% size reduction vs. raw SF2 with no audible quality loss

### The Engine
- Polyphonic sample playback (up to 128 voices)
- Full ADSR envelope per layer
- Multimode filter (LP12, LP24, HP, BP) with resonance
- Built-in FX chain: Reverb → Chorus → Delay
- Monophonic legato / portamento mode
- Velocity sensitivity and key scaling

### The Philosophy
Premium sound. Minimal size. Instant workflow. Solara is the rompler studios reach for when they need everything and no excuses.

---

## Target Users
- Electronic music producers (EDM, synthwave, lo-fi, pop)
- Film/game composers needing fast orchestral sketching
- Live performers wanting a lightweight, reliable rompler
- Sound designers building and distributing their own `.slr` preset packs

---

## Competitive Positioning
| Feature | Nexus 3 | Kontakt | **Solara** |
|---|---|---|---|
| Custom format | ✗ | .nki | **.slr** |
| SF2/SFZ import | ✗ | ✓ | **✓ (converts)** |
| File size | Large | Large | **Small** |
| UI framework | Proprietary | Proprietary | **Visage C++** |
| Open preset format | ✗ | ✗ | **✓** |
