#include "SLRParser.h"

bool SLRParser::load (const juce::File& file)
{
    unload();

    if (!file.existsAsFile())
    {
        lastError_ = "File not found: " + file.getFullPathName();
        return false;
    }

    juce::FileInputStream stream (file);
    if (stream.failedToOpen())
    {
        lastError_ = "Cannot open file: " + file.getFullPathName();
        return false;
    }

    // --- Header ---
    if (stream.read (&header_, sizeof (SLR::Header)) != sizeof (SLR::Header))
    {
        lastError_ = "Truncated header";
        return false;
    }

    if (!header_.isValid())
    {
        lastError_ = "Invalid .slr file (bad magic or version)";
        return false;
    }

    // --- Preset table ---
    presets_.resize (header_.presetCount);
    const size_t presetBytes = header_.presetCount * sizeof (SLR::PresetEntry);
    if (stream.read (presets_.data(), (int) presetBytes) != (int) presetBytes)
    {
        lastError_ = "Truncated preset table";
        return false;
    }

    // --- Zone table ---
    zones_.resize (header_.zoneCount);
    const size_t zoneBytes = header_.zoneCount * sizeof (SLR::SampleZone);
    if (stream.read (zones_.data(), (int) zoneBytes) != (int) zoneBytes)
    {
        lastError_ = "Truncated zone table";
        return false;
    }

    // --- Sample data ---
    const int64_t remaining = stream.getTotalLength() - stream.getPosition();
    if (remaining <= 0)
    {
        lastError_ = "No sample data found";
        return false;
    }

    const SLR::Compression comp = (SLR::Compression)header_.compressionType;

    if (comp == SLR::Compression::ZlibInt16)
    {
        // Read compressed block
        juce::MemoryBlock compressedBlock;
        compressedBlock.setSize ((size_t) remaining);
        if (stream.read (compressedBlock.getData(), (int) remaining) != (int) remaining)
        {
            lastError_ = "Truncated compressed sample data";
            return false;
        }

        // Decompress with JUCE's built-in zlib (zlibFormat = RFC 1950, matches miniz output)
        const uint32_t uncompSize = header_.uncompressedDataSize;
        juce::MemoryBlock int16Block;
        int16Block.setSize (uncompSize);

        {
            juce::MemoryInputStream compStream (compressedBlock, false);
            juce::GZIPDecompressorInputStream decompressor (
                &compStream, false,
                juce::GZIPDecompressorInputStream::zlibFormat);

            const int bytesRead = decompressor.read (int16Block.getData(), (int) uncompSize);
            if (bytesRead != (int) uncompSize)
            {
                lastError_ = "Decompression failed or truncated";
                return false;
            }
        }

        // Convert int16 → float32 and store in sampleData_
        const size_t numSamples = uncompSize / sizeof (int16_t);
        sampleData_.setSize (numSamples * sizeof (float));

        const auto* src = reinterpret_cast<const int16_t*> (int16Block.getData());
        auto*       dst = reinterpret_cast<float*>         (sampleData_.getData());

        for (size_t i = 0; i < numSamples; ++i)
            dst[i] = (float) src[i] / 32767.0f;
    }
    else
    {
        // v1 or Compression::None: raw float32
        sampleData_.setSize ((size_t) remaining);
        if (stream.read (sampleData_.getData(), (int) remaining) != (int) remaining)
        {
            lastError_ = "Truncated sample data";
            return false;
        }
    }

    loaded_     = true;
    loadedFile_ = file;
    lastError_  = {};
    return true;
}

void SLRParser::unload() noexcept
{
    loaded_     = false;
    loadedFile_ = juce::File();
    presets_.clear();
    zones_.clear();
    sampleData_.reset();
    header_ = {};
}

const SLR::SampleZone* SLRParser::findZone (int presetIndex, int midiNote, int velocity) const noexcept
{
    if (!loaded_)
        return nullptr;

    const auto* preset = getPreset (presetIndex);
    if (!preset)
        return nullptr;

    const uint32_t zoneEnd = preset->zoneOffset + preset->zoneCount;
    const SLR::SampleZone* best = nullptr;

    for (uint32_t i = preset->zoneOffset; i < zoneEnd && i < zones_.size(); ++i)
    {
        const auto& z = zones_[i];
        if (midiNote >= z.keyLow  && midiNote <= z.keyHigh &&
            velocity >= z.velLow  && velocity <= z.velHigh)
        {
            best = &z;
            break;
        }
    }

    return best;
}

const float* SLRParser::getSampleData (const SLR::SampleZone& zone) const noexcept
{
    if (!loaded_)
        return nullptr;

    size_t byteOffset;
    const SLR::Compression comp = (SLR::Compression)header_.compressionType;

    if (comp == SLR::Compression::ZlibInt16)
    {
        // sampleOffset is a byte offset into the int16 decompressed buffer.
        // After conversion to float32, each int16 sample → 1 float32 sample.
        // So the float32 byte offset = sampleOffset * 2.
        byteOffset = (size_t)zone.sampleOffset * 2u;
    }
    else
    {
        byteOffset = (size_t)zone.sampleOffset;
    }

    const size_t needed = (size_t)zone.sampleFrames * zone.numChannels * sizeof (float);
    if (byteOffset + needed > sampleData_.getSize())
        return nullptr;

    return reinterpret_cast<const float*> (
        static_cast<const char*> (sampleData_.getData()) + byteOffset);
}
