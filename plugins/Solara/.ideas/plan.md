# Solara — Implementation Plan

## Complexity Score: 4 / 5 (Expert)

## UI Framework: Visage C++
**Rationale:** User confirmed Visage. Premium hardware-aesthetic, brushed metal knobs, matte panels — all custom-drawn via `visage::Frame`. No HTML/CSS overhead; real-time parameter binding is tighter in native C++.

---

## Implementation Strategy: Phased (Score ≥ 3)

Solara is too large for a single implementation pass. We split it into five self-contained implementation sub-phases, each buildable and testable in isolation.

---

## Phase 4.1: SLR Format & Codec

**Goal:** Define and implement the `.slr` binary format and the offline converter tool.

- [ ] Write `SLRSpec.h` — structs for Header, PresetEntry, SampleZone, LoopPoint
- [ ] Write `SLRWriter.cpp` — SF2 parser → `.slr` encoder (FLAC blocks via JUCE)
- [ ] Write `SLRParser.cpp` — `.slr` reader, validates magic, loads preset table
- [ ] Write `SampleCache.cpp` — memory-mapped PCM pool, LRU eviction
- [ ] Write `PresetDatabase.cpp` — scans `.slr` files in preset dir, builds index
- [ ] CLI tool `slr-convert` — wraps SLRWriter, accepts SF2/SFZ path + output path
- [ ] Unit tests: SF2 round-trip, SFZ round-trip, corrupt file rejection

**Deliverable:** `slr-convert.exe` produces valid `.slr` from any SF2/SFZ. SLRParser reads it back cleanly.

---

## Phase 4.2: Voice Engine (Core DSP)

**Goal:** Polyphonic sample playback with envelopes, filter, and pitch.

- [ ] `SamplePlayer` — reads decoded PCM, handles loop, linear interpolation
- [ ] `AmpEnvelope` — ADSR with smoothed transitions (`juce::SmoothedValue`)
- [ ] `FilterEnvelope` — ADSR driving filter cutoff offset
- [ ] `MultimodeFilter` — wraps `juce::dsp::StateVariableTPTFilter`, all four modes
- [ ] `PitchShifter` — coarse + fine detune, pitch bend, portamento glide
- [ ] `LFOEngine` — Sine/Tri/Saw/Square/S&H, routable to Pitch/Filter/Volume
- [ ] `VelocityScaler` — applies curve (Linear/Soft/Hard/Fixed) to incoming velocity
- [ ] `SolaraVoice` — composes all above per-voice components
- [ ] `SolaraSynth` — 128-voice pool, oldest-note stealing, mono mode

**Deliverable:** MIDI in → polyphonic audio out. All envelopes and filter responsive.

---

## Phase 4.3: FX Chain & APVTS

**Goal:** Global effects, parameter system, and DAW integration.

- [ ] FX chain: `juce::Reverb`, `juce::dsp::Chorus`, `juce::dsp::DelayLine`
- [ ] Tempo sync: `juce::AudioPlayHead` → delay time quantization
- [ ] Master gain + pan with `juce::SmoothedValue`
- [ ] APVTS: register all 40 parameters, wire to DSP components
- [ ] State save/load: `getStateInformation` / `setStateInformation` via APVTS XML
- [ ] Thread safety audit: confirm zero allocations in `processBlock`

**Deliverable:** Full audio path, all 40 parameters automatable in host, state survives DAW session.

---

## Phase 4.4: Visage UI

**Goal:** Complete Visage C++ UI matching the design spec.

- [ ] `VisageControls.h` — shared widget base classes
- [ ] `BrushedKnob` — custom `visage::Frame` draw: outer matte ring, inner metal cap, value arc
- [ ] `PresetBrowser` — scrollable list with bank tabs, search field
- [ ] `EnvelopeDisplay` — real-time ADSR curve drawn from live parameter values
- [ ] `FilterSection` — cutoff + resonance knobs, type selector buttons
- [ ] `LFOSection` — rate + depth knobs, shape + target selectors
- [ ] `FXPanel` — three labeled sub-panels (Reverb / Chorus / Delay)
- [ ] `MasterSection` — volume, pan, polyphony, portamento
- [ ] `PluginEditor` — root frame, APVTS attachments for all controls
- [ ] Animation pass: knob momentum, panel transitions, ADSR curve animation

**Deliverable:** Full UI renders in Visage preview, all knobs bind to APVTS.

---

## Phase 4.5: Integration & Polish

**Goal:** Wire everything together; stability and performance pass.

- [ ] `PluginProcessor` — owns `SolaraSynth`, FX chain, APVTS, `SLRParser`/`PresetDatabase`
- [ ] Preset browser → `PresetDatabase` → `SolaraSynth.loadPreset()`
- [ ] Background preset loading: off-thread decode, lock-free handoff to audio thread
- [ ] Voice-level profiling: ensure <1% CPU at 128 voices, 44.1kHz, buffer 256
- [ ] Memory audit: confirm no leaks, SampleCache respects configured RAM budget
- [ ] Edge cases: empty bank, corrupt `.slr`, host tempo not available, mono mode with chords

**Deliverable:** Shippable VST3. Passes all sanity checks.

---

## Class Map

```
PluginProcessor
├── SolaraSynth (juce::Synthesiser)
│   └── SolaraVoice × 128 (juce::SynthesiserVoice)
│       ├── SamplePlayer
│       ├── AmpEnvelope
│       ├── FilterEnvelope
│       ├── MultimodeFilter
│       ├── PitchShifter
│       ├── LFOEngine
│       └── VelocityScaler
├── SLRParser
├── SampleCache
├── PresetDatabase
├── FX Chain (Reverb, Chorus, Delay)
└── juce::AudioProcessorValueTreeState

PluginEditor (visage::Frame root)
├── PresetBrowser
├── BrushedKnob × N
├── EnvelopeDisplay
├── FilterSection
├── LFOSection
├── FXPanel
└── MasterSection
```

---

## Source File Plan

```
plugins/Solara/Source/
├── PluginProcessor.h / .cpp
├── PluginEditor.h / .cpp
├── VisageControls.h              ← all custom Visage widgets
├── SLR/
│   ├── SLRSpec.h                 ← binary format structs
│   ├── SLRParser.h / .cpp
│   ├── SLRWriter.h / .cpp
│   └── SampleCache.h / .cpp
├── Engine/
│   ├── SolaraSynth.h / .cpp
│   ├── SolaraVoice.h / .cpp
│   ├── SamplePlayer.h / .cpp
│   ├── AmpEnvelope.h / .cpp
│   ├── FilterEnvelope.h / .cpp
│   ├── MultimodeFilter.h / .cpp
│   ├── PitchShifter.h / .cpp
│   ├── LFOEngine.h / .cpp
│   └── VelocityScaler.h / .cpp
├── FX/
│   ├── ReverbUnit.h / .cpp
│   ├── ChorusUnit.h / .cpp
│   └── DelayUnit.h / .cpp
└── Presets/
    └── PresetDatabase.h / .cpp
```

---

## Timeline Estimate (reference only)

| Sub-Phase | Estimated Sessions |
|---|---|
| 4.1 SLR Format | 2–3 |
| 4.2 Voice Engine | 3–4 |
| 4.3 FX + APVTS | 1–2 |
| 4.4 Visage UI | 2–3 |
| 4.5 Integration | 1–2 |
| **Total** | **9–14 sessions** |

---

## CMake Notes

```cmake
# Root CMakeLists.txt — Solara target
juce_add_plugin(Solara
    FORMATS VST3
    PRODUCT_NAME "Solara"
    PLUGIN_MANUFACTURER_CODE Slra
    PLUGIN_CODE Slr1
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
)

target_sources(Solara PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SLR/SLRParser.cpp
    Source/SLR/SLRWriter.cpp
    Source/SLR/SampleCache.cpp
    Source/Engine/SolaraSynth.cpp
    # ... all engine files
)

target_link_libraries(Solara PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    visage  # vendored
)
```
