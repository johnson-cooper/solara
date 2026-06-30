#pragma once
#include "ConvertIR.h"
#include <string>

//==============================================================================
// SFZParser
// Parses a SoundFont Z (.sfz) file into ConvertBank.
//
// SFZ is a plain-text format. Structure:
//   <group>          — default values for following regions
//   <region>         — one sample mapping
//   opcode=value     — key-value pairs (e.g. sample=piano_c4.wav lokey=60 hikey=62)
//
// Relevant opcodes parsed:
//   sample           — path to WAV file (relative to .sfz)
//   lokey / hikey    — MIDI key range (also accepts note names)
//   lovel / hivel    — velocity range
//   pitch_keycenter  — root MIDI note
//   tune             — fine tune in cents
//   volume           — gain in dB
//   loop_mode        — no_loop / loop_continuous / loop_sustain / one_shot
//   loop_start       — loop start sample
//   loop_end         — loop end sample
//   transpose        — semitone offset
//==============================================================================
class SFZParser
{
public:
    bool parse (const std::string& sfzPath, ConvertBank& out);
    const std::string& error() const { return error_; }

private:
    struct RegionDef
    {
        std::string sample;
        int   lokey          = 0;
        int   hikey          = 127;
        int   lovel          = 0;
        int   hivel          = 127;
        int   pitch_keycenter = 60;
        float tune           = 0.0f;   // cents
        float volume         = 0.0f;   // dB
        int   transpose      = 0;      // semitones
        bool  loopEnabled    = false;
        uint32_t loop_start  = 0;
        uint32_t loop_end    = 0;
    };

    std::string error_;
    std::string sfzDir_;  // directory of the .sfz file, for resolving relative sample paths

    static int  noteNameToMidi (const std::string& s);
    static bool loadWav (const std::string& path, ConvertZone& zone);

    void applyOpcode (RegionDef& def, const std::string& key, const std::string& value);
    ConvertZone regionToZone (const RegionDef& def);
};
