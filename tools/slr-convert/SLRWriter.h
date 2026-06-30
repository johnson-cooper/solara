#pragma once
#include "ConvertIR.h"
#include <string>

//==============================================================================
// SLRWriter
// Converts a ConvertBank into a binary .slr file as defined by SLRSpec.h.
//
// Layout written:
//   Header (32 bytes)
//   PresetEntry × N
//   SampleZone  × M  (all zones across all presets, flattened)
//   float32 PCM data (interleaved per zone, concatenated)
//==============================================================================
class SLRWriter
{
public:
    struct Options
    {
        float normalizeGain = 1.0f; // global gain multiplier applied during write
        bool  verbose       = false;
    };

    bool write (const ConvertBank& bank,
                const std::string& outputPath,
                const Options& opts = {});

    const std::string& error() const { return error_; }

private:
    std::string error_;
};
