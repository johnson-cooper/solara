# Solara — DSP Architecture Specification

## Complexity Score: 4 / 5
**Rationale:** Custom binary file format with a streaming decoder, polyphonic voice manager (128 voices), per-voice ADSR + filter envelopes, LFO modulation bus, 3-stage FX chain, and SF2/SFZ conversion tool. Each sub-system is individually manageable, but the combination of format engineering + real-time sample streaming + voice stealing + modulation makes this an Expert-level build.

---

## Core Components

### A. SLR Format Layer (offline + runtime)
| Component | Role |
|---|---|
| `SLRConverter` | Standalone CLI tool: reads SF2/SFZ → writes `.slr` |
| `SLRParser` | Runtime reader: parses `.slr` header, sample map, loop points |
| `SampleCache` | Memory-mapped sample pool with LRU eviction |
| `PresetDatabase` | Index of all presets across loaded `.slr` banks |

#### `.slr` File Structure (binary)
```
[Header]       4 bytes magic "SLR1" | uint32 version | uint32 preset_count
[PresetTable]  Array of PresetEntry { name[64], offset, size }
[SampleMap]    Per-preset key/velocity zone table
[LoopTable]    Loop start/end points per sample region
[SampleData]   Interleaved stereo PCM or FLAC-compressed blocks
```
Compression: FLAC lossless per block (uses JUCE `FlacAudioFormat`). Target: 50–65% size reduction vs. raw SF2 PCM.

---

### B. Voice Engine
| Component | Role |
|---|---|
| `SolaraVoice` | Extends `juce::SynthesiserVoice` — one active note |
| `SolaraSynth` | Extends `juce::Synthesiser` — voice pool, stealing policy |
| `SamplePlayer` | Reads decoded PCM, handles loop, interpolation (linear/sinc) |
| `AmpEnvelope` | Per-voice ADSR (amplitude) with smoothed stage transitions |
| `FilterEnvelope` | Per-voice ADSR (filter modulation depth) |
| `MultimodeFilter` | LP12 / LP24 / HP / BP via `juce::dsp::StateVariableTPTFilter` |
| `LFOEngine` | Sine/Tri/Saw/Square/S&H, routes to Pitch/Filter/Volume |
| `PitchShifter` | Coarse/fine detune + pitch bend + portamento glide |

#### Voice Signal Flow (per note)
```
MIDI Note On
    → SolaraSynth (voice steal / assign)
        → SamplePlayer (decode + loop PCM region)
            → PitchShifter (detune + bend + portamento)
                → AmpEnvelope (ADSR gain)
                    → MultimodeFilter (LP/HP/BP + FilterEnvelope + KeyTracking)
                        → LFO modulation applied
                            → VelocityScaler
                                → Voice Output Buffer
```

---

### C. Polyphony & Voice Management
- Pool: 128 `SolaraVoice` instances pre-allocated at startup
- Stealing policy: **Oldest note first** (standard rompler behavior)
- Mono mode: single voice, legato retriggering, portamento glide
- Velocity curve applied at note-on: Linear / Soft / Hard / Fixed

---

### D. FX Chain (post-mixer, global)
```
Voice Mix Bus → Reverb → Chorus → Delay → Master Pan → Master Volume → Output
```

| Stage | Implementation |
|---|---|
| Reverb | `juce::Reverb` (Freeverb algorithm) |
| Chorus | `juce::dsp::Chorus` |
| Delay | `juce::dsp::DelayLine` with tempo-sync via `juce::AudioPlayHead` |
| Pan | Linear stereo pan law |
| Volume | Smoothed gain (`juce::SmoothedValue<float>`) |

---

### E. Parameter System
- All 40 parameters registered via `juce::AudioProcessorValueTreeState` (APVTS)
- Preset save/load: APVTS XML serialized into DAW state
- Real-time safe: parameter reads via `std::atomic` in process block

---

### F. UI Layer (Visage C++)
| Widget | Visage Base | Notes |
|---|---|---|
| `PresetBrowser` | `visage::Frame` | Scrollable list, bank selector tabs |
| `BrushedKnob` | `visage::Frame` | Custom draw: matte ring + metal cap |
| `FilterSection` | `visage::Frame` | Hosts cutoff, resonance, type selector |
| `EnvelopeDisplay` | `visage::Frame` | Real-time ADSR curve visualization |
| `FXPanel` | `visage::Frame` | Three sub-panels: Reverb, Chorus, Delay |
| `MasterSection` | `visage::Frame` | Volume, pan, poly, portamento |
| `LFOSection` | `visage::Frame` | Rate, depth, shape, target |

---

## Full Processing Chain

```
[MIDI Input]
    │
    ▼
[SolaraSynth — Voice Pool (128 voices)]
    │
    ├─ [SamplePlayer → PitchShifter → AmpEnvelope → MultimodeFilter] × N voices
    │
    ▼
[Voice Mix Accumulator]
    │
    ▼
[FX Chain: Reverb → Chorus → Delay]
    │
    ▼
[Master Pan + Volume]
    │
    ▼
[Audio Output]
```

---

## Parameter → Component Mapping

| Parameter | Component | Function |
|---|---|---|
| `master_volume` | Master gain | Output level |
| `master_pan` | Stereo pan | L/R balance |
| `polyphony` | SolaraSynth | Voice cap |
| `mono_mode` | SolaraSynth | Legato mode |
| `portamento_time` | PitchShifter | Glide time |
| `preset_index` | PresetDatabase | Active preset |
| `bank_index` | SLRParser | Active bank |
| `amp_attack/decay/sustain/release` | AmpEnvelope | Amplitude shaping |
| `filter_type` | MultimodeFilter | LP12/LP24/HP/BP |
| `filter_cutoff` | MultimodeFilter | Frequency cutoff |
| `filter_resonance` | MultimodeFilter | Q factor |
| `filter_env_amount` | FilterEnvelope | Mod depth |
| `filter_attack/decay/sustain/release` | FilterEnvelope | Filter shaping |
| `pitch_coarse/fine` | PitchShifter | Semitone + cent detune |
| `pitch_bend_range` | PitchShifter | Bend wheel range |
| `lfo_rate/depth/target/shape` | LFOEngine | Modulation |
| `reverb_mix/size/damp` | juce::Reverb | Room effect |
| `chorus_mix/rate/depth` | juce::dsp::Chorus | Chorus effect |
| `delay_mix/time/feedback/sync` | juce::dsp::DelayLine | Echo effect |
| `velocity_sensitivity` | VelocityScaler | Vel → gain curve |
| `velocity_curve` | VelocityScaler | Shape |
| `key_tracking` | MultimodeFilter | Key → cutoff |

---

## Required JUCE Modules
- `juce_audio_basics`
- `juce_audio_processors`
- `juce_audio_formats` (FLAC decode for `.slr` sample blocks)
- `juce_dsp` (filter, chorus, delay, gain)
- `juce_audio_plugin_client`
- `juce_gui_basics` (Visage integration layer)

## External Dependencies
- **Visage** (vendored, C++ UI framework)
- **libFLAC** (via JUCE's built-in FLAC support)
- No other third-party libs required

---

## Risk Assessment

| Risk | Level | Mitigation |
|---|---|---|
| `.slr` format design (loop points, velocity zones) | High | Define binary spec tightly before codec; write unit tests for round-trip |
| 128-voice real-time safety (no allocations in process block) | High | Pre-allocate all buffers at `prepareToPlay`; use lock-free queues for preset load |
| FLAC decode latency on note-on | Medium | Decode to PCM on preset load, cache in `SampleCache`; never decode in audio thread |
| Portamento in polyphonic voice pool | Medium | Track per-voice previous pitch; only apply glide in mono mode |
| Tempo-sync delay (`juce::AudioPlayHead`) | Medium | Graceful fallback to fixed time if host provides no transport |
| Visage custom knob drawing performance | Low | Draw to offscreen bitmap; invalidate only on value change |
