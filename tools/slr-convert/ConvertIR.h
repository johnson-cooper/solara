#pragma once
#include <string>
#include <vector>
#include <cstdint>

//==============================================================================
// Intermediate representation shared by SF2Parser and SFZParser.
// SLRWriter consumes this to produce a .slr file.
//==============================================================================

struct ConvertZone
{
    uint8_t  keyLow     = 0;
    uint8_t  keyHigh    = 127;
    uint8_t  velLow     = 0;
    uint8_t  velHigh    = 127;
    uint8_t  rootKey    = 60;
    bool     hasLoop    = false;
    uint32_t loopStart  = 0;
    uint32_t loopEnd    = 0;
    float    gain       = 1.0f;
    float    tuneCents  = 0.0f;
    int      numChannels = 1;
    uint32_t sampleRate  = 44100;

    // Decoded float32 PCM (interleaved if stereo: L0 R0 L1 R1 ...)
    std::vector<float> pcm;
};

struct ConvertPreset
{
    std::string name;
    int         bank    = 0;
    int         program = 0;
    std::vector<ConvertZone> zones;
};

struct ConvertBank
{
    std::string              sourcePath;
    std::vector<ConvertPreset> presets;
};
