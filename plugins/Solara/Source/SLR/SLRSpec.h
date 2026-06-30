#pragma once
#include <cstdint>
#include <cstring>

//==============================================================================
// Solara Sound Resource (.slr) Binary Format Specification
// Version 1
//
// File layout:
//   [SLRHeader]
//   [SLRPresetEntry × header.presetCount]
//   [SLRSampleZone  × (sum of all preset zone counts)]
//   [SLRLoopPoint   × (one per sample zone, may be null)]
//   [Sample PCM data blocks — interleaved float32, aligned to 4 bytes]
//==============================================================================

namespace SLR
{

static constexpr uint32_t kMagic   = 0x31524C53; // "SLR1" little-endian
static constexpr uint32_t kVersion = 2;

// Sample block compression modes stored in Header::compressionType
enum class Compression : uint32_t
{
    None      = 0,  // raw float32 interleaved (v1 compat)
    ZlibInt16 = 1,  // zlib-deflate over int16 interleaved PCM
};

//==============================================================================
// File header — 32 bytes
//==============================================================================
struct alignas(4) Header
{
    uint32_t magic;          // Must equal kMagic
    uint32_t version;        // Must equal kVersion
    uint32_t presetCount;    // Number of SLRPresetEntry records
    uint32_t zoneCount;      // Total SLRSampleZone records (all presets)
    uint32_t sampleRate;           // Native sample rate of stored PCM data
    uint32_t numChannels;          // 1 = mono, 2 = stereo
    uint32_t compressionType;      // SLR::Compression enum value
    uint32_t uncompressedDataSize; // Byte size of decompressed sample data (0 if None)

    bool isValid() const noexcept
    {
        return magic == kMagic && (version == kVersion || version == 1) && sampleRate > 0;
    }
};
static_assert (sizeof (Header) == 32, "Header size must be 32 bytes");

//==============================================================================
// Preset entry — 96 bytes
//==============================================================================
struct alignas(4) PresetEntry
{
    char     name[64];        // Null-terminated preset name
    uint32_t zoneOffset;      // Index of first SLRSampleZone for this preset
    uint32_t zoneCount;       // Number of zones in this preset
    uint32_t category;        // Category tag (see PresetCategory enum)
    uint32_t reserved;

    const char* getName() const noexcept { return name; }
};
static_assert (sizeof (PresetEntry) == 80, "PresetEntry size must be 80 bytes");

//==============================================================================
// Sample zone — 48 bytes
// One zone = one sample region covering a key range + velocity range.
//==============================================================================
struct alignas(4) SampleZone
{
    uint8_t  keyLow;          // MIDI note low (0–127)
    uint8_t  keyHigh;         // MIDI note high (0–127)
    uint8_t  velLow;          // Velocity low (0–127)
    uint8_t  velHigh;         // Velocity high (0–127)
    uint8_t  rootKey;         // MIDI root note of sample
    uint8_t  hasLoop;         // 1 if loop points are defined
    uint8_t  numChannels;     // 1 or 2
    uint8_t  reserved0;
    uint32_t sampleOffset;    // Byte offset into sample data block
    uint32_t sampleFrames;    // Number of audio frames (per channel)
    uint32_t loopStart;       // Loop start frame (if hasLoop)
    uint32_t loopEnd;         // Loop end frame (if hasLoop)
    float    gain;            // Linear gain applied at playback (default 1.0)
    float    tune;            // Fine tune in cents (-100..+100)
    uint32_t reserved1[3];
};
static_assert (sizeof (SampleZone) == 44, "SampleZone size must be 44 bytes");

//==============================================================================
// Preset categories
//==============================================================================
enum class PresetCategory : uint32_t
{
    User    = 0,
    Keys    = 1,
    Pads    = 2,
    Leads   = 3,
    Strings = 4,
    FX      = 5,
    Unknown = 0xFFFFFFFF
};

//==============================================================================
// Offsets helper — computed from header at load time
//==============================================================================
struct FileOffsets
{
    size_t presetTableOffset;  // Byte offset of first PresetEntry
    size_t zoneTableOffset;    // Byte offset of first SampleZone
    size_t sampleDataOffset;   // Byte offset of first sample byte

    static FileOffsets compute (const Header& h) noexcept
    {
        FileOffsets o;
        o.presetTableOffset = sizeof (Header);
        o.zoneTableOffset   = o.presetTableOffset + h.presetCount * sizeof (PresetEntry);
        o.sampleDataOffset  = o.zoneTableOffset   + h.zoneCount   * sizeof (SampleZone);
        return o;
    }
};

} // namespace SLR
