#include "SLRWriter.h"
#include "../../plugins/Solara/Source/SLR/SLRSpec.h"
#include <miniz.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>

static inline int16_t floatToInt16 (float s)
{
    float clamped = s < -1.0f ? -1.0f : (s > 1.0f ? 1.0f : s);
    return static_cast<int16_t> (clamped * 32767.0f);
}

bool SLRWriter::write (const ConvertBank& bank,
                       const std::string& outputPath,
                       const Options& opts)
{
    // --- Count totals ---
    uint32_t totalZones = 0;
    for (const auto& p : bank.presets)
        totalZones += (uint32_t)p.zones.size();

    if (bank.presets.empty())
    {
        error_ = "No presets to write";
        return false;
    }

    // --- Step 1: Build int16 sample blob + compute sampleOffset per zone ---
    // sampleOffset = byte offset into the int16 decompressed buffer.
    // sampleFrames = number of audio frames (unchanged).

    struct ZoneRecord { SLR::SampleZone zone; const std::vector<float>* pcm; };
    std::vector<ZoneRecord> flatZones;
    flatZones.reserve (totalZones);

    // Interleaved int16 blob (all zones concatenated)
    std::vector<int16_t> int16Blob;
    int16Blob.reserve (1 << 22); // 8 MB initial reservation

    uint32_t int16ByteOffset = 0;

    for (const auto& preset : bank.presets)
    {
        for (const auto& cz : preset.zones)
        {
            SLR::SampleZone sz {};
            sz.keyLow      = cz.keyLow;
            sz.keyHigh     = cz.keyHigh;
            sz.velLow      = cz.velLow;
            sz.velHigh     = cz.velHigh;
            sz.rootKey     = cz.rootKey;
            sz.hasLoop     = cz.hasLoop ? 1 : 0;
            sz.numChannels = (uint8_t)std::max(1, std::min(2, cz.numChannels));
            sz.sampleOffset= int16ByteOffset;
            sz.sampleFrames= (uint32_t)(cz.pcm.size() / sz.numChannels);
            sz.loopStart   = cz.loopStart;
            sz.loopEnd     = cz.loopEnd;
            sz.gain        = cz.gain * opts.normalizeGain;
            sz.tune        = cz.tuneCents;

            // Convert float32 → int16 and append to blob
            for (float sample : cz.pcm)
                int16Blob.push_back (floatToInt16 (sample));

            const uint32_t zoneInt16Bytes = (uint32_t)(cz.pcm.size() * sizeof(int16_t));
            int16ByteOffset += zoneInt16Bytes;

            flatZones.push_back ({ sz, &cz.pcm });
        }
    }

    const uint32_t uncompressedSize = (uint32_t)(int16Blob.size() * sizeof(int16_t));

    // --- Step 2: Compress the int16 blob with miniz (zlib format, best compression) ---
    mz_ulong compressedBound = mz_compressBound ((mz_ulong)uncompressedSize);
    std::vector<uint8_t> compressedBlob (compressedBound);
    mz_ulong compressedSize = compressedBound;

    int mzStatus = mz_compress2 (compressedBlob.data(), &compressedSize,
                                 reinterpret_cast<const unsigned char*> (int16Blob.data()),
                                 (mz_ulong)uncompressedSize,
                                 MZ_BEST_COMPRESSION);
    if (mzStatus != MZ_OK)
    {
        error_ = "Compression failed (miniz error " + std::to_string(mzStatus) + ")";
        return false;
    }
    compressedBlob.resize ((size_t)compressedSize);

    // --- Step 3: Build preset table ---
    std::vector<SLR::PresetEntry> presetEntries;
    presetEntries.reserve (bank.presets.size());
    uint32_t zoneOffset = 0;
    for (const auto& preset : bank.presets)
    {
        SLR::PresetEntry pe {};
        std::strncpy (pe.name, preset.name.c_str(), sizeof(pe.name) - 1);
        pe.zoneOffset = zoneOffset;
        pe.zoneCount  = (uint32_t)preset.zones.size();
        pe.category   = 0;
        zoneOffset   += pe.zoneCount;
        presetEntries.push_back (pe);
    }

    // --- Step 4: Write file ---
    std::FILE* f = std::fopen (outputPath.c_str(), "wb");
    if (!f) { error_ = "Cannot create: " + outputPath; return false; }

    SLR::Header header {};
    header.magic              = SLR::kMagic;
    header.version            = SLR::kVersion;
    header.presetCount        = (uint32_t)bank.presets.size();
    header.zoneCount          = totalZones;
    header.sampleRate         = flatZones.empty() ? 44100u
                                    : (uint32_t)bank.presets[0].zones[0].sampleRate;
    header.numChannels        = flatZones.empty() ? 1u : flatZones[0].zone.numChannels;
    header.compressionType    = (uint32_t)SLR::Compression::ZlibInt16;
    header.uncompressedDataSize = uncompressedSize;

    std::fwrite (&header,          sizeof(header),          1,                    f);
    std::fwrite (presetEntries.data(), sizeof(SLR::PresetEntry), presetEntries.size(), f);
    for (const auto& zr : flatZones)
        std::fwrite (&zr.zone, sizeof(SLR::SampleZone), 1, f);
    std::fwrite (compressedBlob.data(), 1, compressedBlob.size(), f);
    std::fclose (f);

    if (opts.verbose)
    {
        const double uncompMB = uncompressedSize / (1024.0 * 1024.0);
        const double compMB   = compressedSize   / (1024.0 * 1024.0);
        std::printf ("Written: %s\n", outputPath.c_str());
        std::printf ("  Presets:       %u\n", header.presetCount);
        std::printf ("  Zones:         %u\n", totalZones);
        std::printf ("  Uncompressed:  %.2f MB (int16 PCM)\n", uncompMB);
        std::printf ("  Compressed:    %.2f MB (zlib)\n", compMB);
        std::printf ("  Ratio:         %.1f%%\n", 100.0 * compressedSize / uncompressedSize);
    }

    return true;
}
