#include "SF2Parser.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

//==============================================================================
bool SF2Parser::idEq (const char* a, const char* b) noexcept
{
    return a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3];
}

float SF2Parser::attenuationToGain (int16_t cB) noexcept
{
    // SF2 attenuation: 0 = max gain, positive = quieter.  Unit: centibels.
    // gain = 10^(-cB / 200)
    if (cB <= 0) return 1.0f;
    return std::pow (10.0f, -(float)cB / 200.0f);
}

template<typename T>
bool SF2Parser::readVector (std::FILE* f, uint32_t bytes, std::vector<T>& out)
{
    if (bytes % sizeof(T) != 0) return false;
    out.resize (bytes / sizeof(T));
    return std::fread (out.data(), sizeof(T), out.size(), f) == out.size();
}

//==============================================================================
bool SF2Parser::parse (const std::string& path, ConvertBank& out)
{
    std::FILE* f = std::fopen (path.c_str(), "rb");
    if (!f) { error_ = "Cannot open: " + path; return false; }

    // RIFF header
    RiffHeader riff {};
    if (std::fread (&riff, sizeof(riff), 1, f) != 1 ||
        !idEq (riff.id, "RIFF") || !idEq (riff.type, "sfbk"))
    {
        std::fclose (f);
        error_ = "Not a valid SF2 file (bad RIFF/sfbk header)";
        return false;
    }

    if (!readChunks (f))
    {
        std::fclose (f);
        return false;
    }

    std::fclose (f);

    if (phdr_.empty() || shdr_.empty() || smpl_.empty())
    {
        error_ = "SF2 missing required tables";
        return false;
    }

    out.sourcePath = path;
    buildPresets (out);
    return true;
}

//==============================================================================
bool SF2Parser::readChunks (std::FILE* f)
{
    ChunkHeader ch {};
    while (std::fread (&ch, sizeof(ch), 1, f) == 1)
    {
        const long dataStart = std::ftell (f);
        const long nextChunk = dataStart + (long)ch.size;

        if (idEq (ch.id, "LIST"))
        {
            char listType[4];
            std::fread (listType, 4, 1, f);

            if (idEq (listType, "sdta"))
            {
                if (!readListChunk (f, ch.size - 4, "sdta")) return false;
            }
            else if (idEq (listType, "pdta"))
            {
                if (!readListChunk (f, ch.size - 4, "pdta")) return false;
            }
            // Skip INFO and any other LIST chunks
        }

        std::fseek (f, nextChunk, SEEK_SET);
    }
    return true;
}

bool SF2Parser::readListChunk (std::FILE* f, uint32_t listSize, const char* expectedType)
{
    const long listEnd = std::ftell (f) + (long)listSize;

    ChunkHeader ch {};
    while (std::ftell (f) < listEnd && std::fread (&ch, sizeof(ch), 1, f) == 1)
    {
        const long dataStart = std::ftell (f);
        const long nextChunk = dataStart + (long)ch.size;

        if (idEq (expectedType, "sdta"))
        {
            if (idEq (ch.id, "smpl"))
            {
                if (!readVector (f, ch.size, smpl_))
                { error_ = "Failed to read smpl chunk"; return false; }
            }
            // sm24 (24-bit extension) intentionally skipped
        }
        else if (idEq (expectedType, "pdta"))
        {
            if      (idEq (ch.id, "phdr")) { if (!readVector (f, ch.size, phdr_)) return false; }
            else if (idEq (ch.id, "pbag")) { if (!readVector (f, ch.size, pbag_)) return false; }
            else if (idEq (ch.id, "pgen")) { if (!readVector (f, ch.size, pgen_)) return false; }
            else if (idEq (ch.id, "inst")) { if (!readVector (f, ch.size, inst_)) return false; }
            else if (idEq (ch.id, "ibag")) { if (!readVector (f, ch.size, ibag_)) return false; }
            else if (idEq (ch.id, "igen")) { if (!readVector (f, ch.size, igen_)) return false; }
            else if (idEq (ch.id, "shdr")) { if (!readVector (f, ch.size, shdr_)) return false; }
        }

        std::fseek (f, nextChunk, SEEK_SET);
    }
    return true;
}

//==============================================================================
ConvertZone SF2Parser::buildZone (int izonIdx, const sfSampleHeader& shdr) const
{
    ConvertZone zone;
    zone.sampleRate  = shdr.dwSampleRate;
    zone.rootKey     = shdr.byOriginalPitch;
    zone.tuneCents   = (float)shdr.chPitchCorrection;
    zone.numChannels = 1; // SF2 samples are always mono; stereo is two linked samples

    // Scan instrument generators for this zone
    for (int g = ibag_[izonIdx].wGenNdx; g < ibag_[izonIdx + 1].wGenNdx; ++g)
    {
        const auto& gen = igen_[g];
        switch (gen.sfGenOper)
        {
            case KeyRange:
                zone.keyLow  = gen.genAmount.range.lo;
                zone.keyHigh = gen.genAmount.range.hi;
                break;
            case VelRange:
                zone.velLow  = gen.genAmount.range.lo;
                zone.velHigh = gen.genAmount.range.hi;
                break;
            case OverridingRootKey:
                if (gen.genAmount.shAmount >= 0 && gen.genAmount.shAmount <= 127)
                    zone.rootKey = (uint8_t)gen.genAmount.shAmount;
                break;
            case InitialAttenuation:
                zone.gain = attenuationToGain (gen.genAmount.shAmount);
                break;
            case FineTune:
                zone.tuneCents += (float)gen.genAmount.shAmount;
                break;
            case SampleModes:
                zone.hasLoop = (gen.genAmount.wAmount & 1) != 0;
                break;
            default: break;
        }
    }

    // Copy PCM from smpl_ (16-bit → float32)
    const int32_t start = (int32_t)shdr.dwStart;
    const int32_t end   = (int32_t)shdr.dwEnd;
    if (start >= 0 && end > start && end <= (int32_t)smpl_.size())
    {
        const int frames = end - start;
        zone.pcm.resize (frames);
        for (int i = 0; i < frames; ++i)
            zone.pcm[i] = (float)smpl_[start + i] / 32768.0f;

        zone.loopStart = shdr.dwStartloop - shdr.dwStart;
        zone.loopEnd   = shdr.dwEndloop   - shdr.dwStart;
    }

    return zone;
}

//==============================================================================
void SF2Parser::buildPresets (ConvertBank& out) const
{
    // phdr_ has a terminal "EOP" entry — iterate count-1
    for (int pi = 0; pi + 1 < (int)phdr_.size(); ++pi)
    {
        const auto& ph = phdr_[pi];

        ConvertPreset preset;
        preset.name    = std::string (ph.achPresetName, strnlen (ph.achPresetName, 20));
        preset.bank    = ph.wBank;
        preset.program = ph.wPreset;

        // Walk preset bags → generators → instrument references
        for (int pb = ph.wPresetBagNdx; pb < phdr_[pi+1].wPresetBagNdx; ++pb)
        {
            // Find instrument generator in this preset bag
            int instIdx = -1;
            for (int pg = pbag_[pb].wGenNdx; pg < pbag_[pb+1].wGenNdx; ++pg)
            {
                if (pgen_[pg].sfGenOper == 41) // instrument
                {
                    instIdx = pgen_[pg].genAmount.wAmount;
                    break;
                }
            }
            if (instIdx < 0 || instIdx + 1 >= (int)inst_.size()) continue;

            // Walk instrument zones
            for (int ib = inst_[instIdx].wInstBagNdx;
                 ib < inst_[instIdx+1].wInstBagNdx; ++ib)
            {
                // Find sampleID in this instrument zone
                int sampleId = -1;
                for (int ig = ibag_[ib].wGenNdx; ig < ibag_[ib+1].wGenNdx; ++ig)
                {
                    if (igen_[ig].sfGenOper == SampleID)
                    {
                        sampleId = igen_[ig].genAmount.wAmount;
                        break;
                    }
                }
                if (sampleId < 0 || sampleId >= (int)shdr_.size()) continue;

                const auto& shdr = shdr_[sampleId];
                // Skip terminal and ROM samples
                if (shdr.sfSampleType == 0 || (shdr.sfSampleType & 0x8000)) continue;
                // Skip right-channel of stereo pair (we'll handle mono only for now)
                if (shdr.sfSampleType == 2) continue;

                ConvertZone zone = buildZone (ib, shdr);
                if (!zone.pcm.empty())
                    preset.zones.push_back (std::move (zone));
            }
        }

        if (!preset.zones.empty())
            out.presets.push_back (std::move (preset));
    }
}
