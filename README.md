# Solara

> Every sound you've ever needed. Smaller than you thought possible.

Solara is a professional-grade rompler VST3 plugin built for producers across every genre — EDM, orchestral, lo-fi, pop, cinematic. It reads a custom compressed soundfont format (`.slr`) derived from industry-standard SF2 and SFZ files, delivering studio-quality sample playback at a fraction of the disk footprint.

## What is it?

Solara is a rompler (sample-playback synth) that gets out of your way: load a preset, twist a knob, make music. Under the hood it's a high-performance sample engine written in JUCE 8 with a native Visage C++ UI — no web stack, no overhead.

### The `.slr` format (Solara Sound Resource)

- Converts SF2/SFZ soundfonts at import time using a bundled CLI tool (`slr-convert`)
- Lossless or perceptual compression (user-selectable at conversion)
- Stores velocity layers, loop points, sample maps, and modulation envelopes
- Targets a 40–70% size reduction vs. raw SF2 with no audible quality loss

### The engine

- Polyphonic sample playback (up to 128 voices)
- Full ADSR envelope per layer
- Multimode filter (LP12, LP24, HP, BP) with resonance
- Built-in FX chain: Reverb → Chorus → Delay
- Monophonic legato / portamento mode
- Velocity sensitivity and key scaling

### Who it's for

- Electronic music producers (EDM, synthwave, lo-fi, pop)
- Film/game composers needing fast orchestral sketching
- Live performers wanting a lightweight, reliable rompler
- Sound designers building and distributing their own `.slr` preset packs

## Download

Grab the latest build from the [Releases](https://github.com/johnson-cooper/solara/releases) page. Each release includes `Solara.vst3` (Windows, macOS, Linux) and a factory preset library.

### Installation

**Windows**
1. Unzip the release archive
2. Copy `Solara.vst3` → `C:\Program Files\Common Files\VST3\`
3. Copy the `Solara Presets` folder → `Documents\Solara\Presets\`

**macOS**
1. Unzip the release archive
2. Copy `Solara.vst3` → `/Library/Audio/Plug-Ins/VST3/`
3. Copy the `Solara Presets` folder → `~/Documents/Solara/Presets/`

**Linux**
1. Unzip the release archive
2. Copy `Solara.vst3` → `~/.vst3/` or `/usr/lib/vst3/`
3. Copy the `Solara Presets` folder → `~/Documents/Solara/Presets/`

## License

MIT
