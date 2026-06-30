#include "SFZParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdio>

//==============================================================================
// Note name → MIDI number (C4 = 60)
//==============================================================================
int SFZParser::noteNameToMidi (const std::string& s)
{
    // Try plain integer first
    bool isNum = !s.empty();
    for (char c : s) if (!std::isdigit(c)) { isNum = false; break; }
    if (isNum) return std::stoi (s);

    static const char* names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    static const char* flats[] = {"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};

    std::string upper = s;
    for (char& c : upper) c = (char)std::toupper (c);

    // Parse note + optional sharp/flat + octave, e.g. "C#4", "Bb3"
    int noteClass = -1;
    size_t i = 0;
    if (i < upper.size())
    {
        char base = upper[i++];
        std::string noteName;
        noteName += base;
        if (i < upper.size() && (upper[i] == '#' || upper[i] == 'B' || upper[i] == 'b'))
            noteName += upper[i++];

        for (int n = 0; n < 12; ++n)
        {
            if (noteName == names[n] || noteName == flats[n])
            { noteClass = n; break; }
        }
    }
    if (noteClass < 0) return 60;

    // Octave number (may be negative, e.g. "C-1")
    int octave = 4;
    if (i < upper.size())
    {
        bool neg = (upper[i] == '-');
        if (neg) ++i;
        if (i < upper.size() && std::isdigit(upper[i]))
            octave = (neg ? -1 : 1) * (upper[i] - '0');
    }

    return (octave + 1) * 12 + noteClass;
}

//==============================================================================
// Minimal WAV reader (PCM 16-bit or 32-bit float, mono or stereo)
//==============================================================================
bool SFZParser::loadWav (const std::string& path, ConvertZone& zone)
{
    std::FILE* f = std::fopen (path.c_str(), "rb");
    if (!f) return false;

    // RIFF header
    char riff[4]; uint32_t riffSize; char wave[4];
    if (std::fread(riff,1,4,f)!=4 || std::fread(&riffSize,4,1,f)!=1 || std::fread(wave,1,4,f)!=4 ||
        std::string(riff,4)!="RIFF" || std::string(wave,4)!="WAVE")
    { std::fclose(f); return false; }

    uint16_t audioFormat=0, numCh=1, bitsPerSample=16;
    uint32_t sampleRate=44100, dataSize=0;
    bool foundFmt=false, foundData=false;

    while (!foundData)
    {
        char id[4]; uint32_t sz;
        if (std::fread(id,1,4,f)!=4 || std::fread(&sz,4,1,f)!=1) break;
        long next = std::ftell(f) + sz;

        if (std::string(id,4)=="fmt ")
        {
            std::fread(&audioFormat,2,1,f);
            std::fread(&numCh,2,1,f);
            std::fread(&sampleRate,4,1,f);
            uint32_t byteRate; std::fread(&byteRate,4,1,f);
            uint16_t blockAlign; std::fread(&blockAlign,2,1,f);
            std::fread(&bitsPerSample,2,1,f);
            foundFmt = true;
        }
        else if (std::string(id,4)=="data")
        {
            dataSize = sz;
            foundData = true;

            zone.sampleRate  = sampleRate;
            zone.numChannels = numCh;

            const int totalFrames = dataSize / (numCh * (bitsPerSample / 8));
            zone.pcm.resize ((size_t)totalFrames * numCh);

            if (audioFormat == 1 && bitsPerSample == 16)
            {
                std::vector<int16_t> raw (totalFrames * numCh);
                std::fread (raw.data(), 2, raw.size(), f);
                for (size_t k=0; k<raw.size(); ++k)
                    zone.pcm[k] = (float)raw[k] / 32768.0f;
            }
            else if (audioFormat == 1 && bitsPerSample == 24)
            {
                for (size_t k=0; k<zone.pcm.size(); ++k)
                {
                    uint8_t b[3]; std::fread(b,1,3,f);
                    int32_t v = (int32_t)((b[2]<<24)|(b[1]<<16)|(b[0]<<8)) >> 8;
                    zone.pcm[k] = (float)v / 8388608.0f;
                }
            }
            else if ((audioFormat == 3 || audioFormat == 0xFFFE) && bitsPerSample == 32)
            {
                std::fread (zone.pcm.data(), 4, zone.pcm.size(), f);
            }
            else
            {
                // Unsupported format
                std::fclose (f);
                return false;
            }
        }

        std::fseek(f, next, SEEK_SET);
    }

    std::fclose(f);
    return foundFmt && foundData && !zone.pcm.empty();
}

//==============================================================================
void SFZParser::applyOpcode (RegionDef& def, const std::string& key, const std::string& value)
{
    if      (key == "sample")           def.sample            = value;
    else if (key == "lokey")            def.lokey             = noteNameToMidi(value);
    else if (key == "hikey")            def.hikey             = noteNameToMidi(value);
    else if (key == "key")             { def.lokey = def.hikey = def.pitch_keycenter = noteNameToMidi(value); }
    else if (key == "lovel")            def.lovel             = std::stoi(value);
    else if (key == "hivel")            def.hivel             = std::stoi(value);
    else if (key == "pitch_keycenter")  def.pitch_keycenter   = noteNameToMidi(value);
    else if (key == "tune")             def.tune              = std::stof(value);
    else if (key == "transpose")        def.transpose         = std::stoi(value);
    else if (key == "volume")           def.volume            = std::stof(value);
    else if (key == "loop_start")       def.loop_start        = (uint32_t)std::stoul(value);
    else if (key == "loop_end")         def.loop_end          = (uint32_t)std::stoul(value);
    else if (key == "loop_mode")
    {
        def.loopEnabled = (value == "loop_continuous" || value == "loop_sustain");
    }
}

ConvertZone SFZParser::regionToZone (const RegionDef& def)
{
    ConvertZone z;
    z.keyLow    = (uint8_t)std::max(0, std::min(127, def.lokey));
    z.keyHigh   = (uint8_t)std::max(0, std::min(127, def.hikey));
    z.velLow    = (uint8_t)std::max(0, std::min(127, def.lovel));
    z.velHigh   = (uint8_t)std::max(0, std::min(127, def.hivel));
    z.rootKey   = (uint8_t)std::max(0, std::min(127, def.pitch_keycenter + def.transpose));
    z.hasLoop   = def.loopEnabled;
    z.loopStart = def.loop_start;
    z.loopEnd   = def.loop_end;
    z.gain      = (def.volume == 0.0f) ? 1.0f : std::pow(10.0f, def.volume / 20.0f);
    z.tuneCents = def.tune;
    return z;
}

//==============================================================================
bool SFZParser::parse (const std::string& sfzPath, ConvertBank& out)
{
    std::ifstream file (sfzPath);
    if (!file) { error_ = "Cannot open: " + sfzPath; return false; }

    // Resolve directory for relative WAV paths
    const size_t lastSlash = sfzPath.find_last_of ("/\\");
    sfzDir_ = (lastSlash != std::string::npos) ? sfzPath.substr (0, lastSlash + 1) : "./";

    out.sourcePath = sfzPath;

    // Use filename (without .sfz) as the single preset name
    std::string presetName = sfzPath.substr (lastSlash + 1);
    if (presetName.size() > 4)
        presetName = presetName.substr (0, presetName.size() - 4);

    ConvertPreset preset;
    preset.name = presetName;

    RegionDef groupDef;   // defaults from <group>
    RegionDef regionDef;  // current <region>
    bool inRegion = false;

    auto flushRegion = [&]()
    {
        if (!inRegion || regionDef.sample.empty()) return;

        // Resolve sample path
        std::string wavPath = sfzDir_ + regionDef.sample;
        // Normalize backslashes
        for (char& c : wavPath) if (c == '\\') c = '/';

        ConvertZone zone = regionToZone (regionDef);
        if (loadWav (wavPath, zone))
        {
            // Override loop points from WAV if not set in SFZ
            if (zone.hasLoop && zone.loopEnd == 0)
                zone.loopEnd = (uint32_t)zone.pcm.size() / (uint32_t)zone.numChannels - 1;

            preset.zones.push_back (std::move (zone));
        }
        inRegion = false;
    };

    std::string line;
    while (std::getline (file, line))
    {
        // Strip comments
        const size_t commentPos = line.find ("//");
        if (commentPos != std::string::npos)
            line = line.substr (0, commentPos);

        std::istringstream ss (line);
        std::string token;
        while (ss >> token)
        {
            if (token == "<group>")
            {
                flushRegion();
                groupDef  = RegionDef{};
                inRegion  = false;
            }
            else if (token == "<region>")
            {
                flushRegion();
                regionDef = groupDef; // inherit group defaults
                inRegion  = true;
            }
            else if (token.find ('=') != std::string::npos)
            {
                const size_t eq  = token.find ('=');
                std::string key  = token.substr (0, eq);
                std::string val  = token.substr (eq + 1);

                // Value may have spaces — consume rest if empty (e.g. sample=my file.wav)
                if (val.empty()) ss >> val;

                if (inRegion)
                    applyOpcode (regionDef, key, val);
                else
                    applyOpcode (groupDef, key, val);
            }
        }
    }
    flushRegion();

    if (!preset.zones.empty())
        out.presets.push_back (std::move (preset));

    return !out.presets.empty();
}
