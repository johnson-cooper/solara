# Solara — Parameter Specification

## Section 1: Master / Global
| ID | Name | Type | Range | Default | Unit | Notes |
|---|---|---|---|---|---|---|
| `master_volume` | Master Volume | Float | 0.0 – 1.0 | 0.8 | dB | Global output gain |
| `master_pan` | Master Pan | Float | -1.0 – 1.0 | 0.0 | — | Stereo pan |
| `polyphony` | Polyphony | Int | 1 – 128 | 32 | voices | Voice count cap |
| `mono_mode` | Mono Mode | Bool | 0 / 1 | 0 | — | Legato mono playback |
| `portamento_time` | Portamento | Float | 0.0 – 2.0 | 0.0 | sec | Glide time (mono only) |

## Section 2: Preset Browser
| ID | Name | Type | Range | Default | Unit | Notes |
|---|---|---|---|---|---|---|
| `preset_index` | Preset | Int | 0 – N | 0 | — | Selected preset slot |
| `bank_index` | Bank | Int | 0 – N | 0 | — | .slr file/bank index |

## Section 3: Amplitude Envelope (ADSR)
| ID | Name | Type | Range | Default | Unit | Notes |
|---|---|---|---|---|---|---|
| `amp_attack` | Attack | Float | 0.001 – 10.0 | 0.01 | sec | |
| `amp_decay` | Decay | Float | 0.001 – 10.0 | 0.3 | sec | |
| `amp_sustain` | Sustain | Float | 0.0 – 1.0 | 0.8 | — | Level (not time) |
| `amp_release` | Release | Float | 0.001 – 20.0 | 0.5 | sec | |

## Section 4: Filter
| ID | Name | Type | Range | Default | Unit | Notes |
|---|---|---|---|---|---|---|
| `filter_type` | Filter Type | Enum | LP12/LP24/HP/BP | LP12 | — | Multimode |
| `filter_cutoff` | Cutoff | Float | 20.0 – 20000.0 | 18000.0 | Hz | Log scale |
| `filter_resonance` | Resonance | Float | 0.0 – 1.0 | 0.0 | — | Q factor |
| `filter_env_amount` | Env Amount | Float | -1.0 – 1.0 | 0.0 | — | Filter envelope depth |
| `filter_attack` | F. Attack | Float | 0.001 – 10.0 | 0.01 | sec | Filter env attack |
| `filter_decay` | F. Decay | Float | 0.001 – 10.0 | 0.5 | sec | Filter env decay |
| `filter_sustain` | F. Sustain | Float | 0.0 – 1.0 | 0.5 | — | Filter env sustain |
| `filter_release` | F. Release | Float | 0.001 – 10.0 | 0.5 | sec | Filter env release |

## Section 5: Pitch
| ID | Name | Type | Range | Default | Unit | Notes |
|---|---|---|---|---|---|---|
| `pitch_coarse` | Coarse Tune | Int | -24 – +24 | 0 | semitones | |
| `pitch_fine` | Fine Tune | Float | -100.0 – 100.0 | 0.0 | cents | |
| `pitch_bend_range` | Bend Range | Int | 0 – 24 | 2 | semitones | |

## Section 6: Modulation (LFO)
| ID | Name | Type | Range | Default | Unit | Notes |
|---|---|---|---|---|---|---|
| `lfo_rate` | LFO Rate | Float | 0.1 – 20.0 | 5.0 | Hz | |
| `lfo_depth` | LFO Depth | Float | 0.0 – 1.0 | 0.0 | — | |
| `lfo_target` | LFO Target | Enum | Pitch/Filter/Volume | Pitch | — | Destination |
| `lfo_shape` | LFO Shape | Enum | Sine/Tri/Saw/Square/S&H | Sine | — | |

## Section 7: Effects Chain
### Reverb
| ID | Name | Type | Range | Default | Unit |
|---|---|---|---|---|---|
| `reverb_mix` | Reverb Mix | Float | 0.0 – 1.0 | 0.0 | — |
| `reverb_size` | Room Size | Float | 0.0 – 1.0 | 0.5 | — |
| `reverb_damp` | Damping | Float | 0.0 – 1.0 | 0.5 | — |

### Chorus
| ID | Name | Type | Range | Default | Unit |
|---|---|---|---|---|---|
| `chorus_mix` | Chorus Mix | Float | 0.0 – 1.0 | 0.0 | — |
| `chorus_rate` | Chorus Rate | Float | 0.1 – 5.0 | 1.0 | Hz |
| `chorus_depth` | Chorus Depth | Float | 0.0 – 1.0 | 0.3 | — |

### Delay
| ID | Name | Type | Range | Default | Unit |
|---|---|---|---|---|---|
| `delay_mix` | Delay Mix | Float | 0.0 – 1.0 | 0.0 | — |
| `delay_time` | Delay Time | Float | 0.01 – 2.0 | 0.25 | sec |
| `delay_feedback` | Feedback | Float | 0.0 – 0.95 | 0.3 | — |
| `delay_sync` | Tempo Sync | Bool | 0 / 1 | 0 | — |

## Section 8: Velocity & Dynamics
| ID | Name | Type | Range | Default | Unit | Notes |
|---|---|---|---|---|---|---|
| `velocity_sensitivity` | Vel. Sensitivity | Float | 0.0 – 1.0 | 0.7 | — | How much velocity affects volume |
| `velocity_curve` | Vel. Curve | Enum | Linear/Soft/Hard/Fixed | Linear | — | |
| `key_tracking` | Key Tracking | Float | 0.0 – 1.0 | 0.0 | — | Filter cutoff key scaling |

## Total Parameter Count: 40
