#pragma once
#include "ConvertIR.h"
#include <string>

//==============================================================================
// SF2Parser
// Parses a SoundFont 2 (.sf2) file into ConvertBank.
//
// SF2 file structure (RIFF):
//   RIFF 'sfbk'
//     LIST 'INFO'  — metadata (ignored)
//     LIST 'sdta'  — sample data
//       'smpl'     — 16-bit signed PCM, all samples concatenated
//       'sm24'     — optional low bytes for 24-bit (ignored)
//     LIST 'pdta'  — preset/instrument/sample headers
//       'phdr'     — sfPresetHeader[]   presets
//       'pbag'     — sfBag[]            preset zones
//       'pmod'     — sfMod[]            preset modulators (ignored)
//       'pgen'     — sfGen[]            preset generators
//       'inst'     — sfInst[]           instruments
//       'ibag'     — sfBag[]            instrument zones
//       'imod'     — sfMod[]            instrument modulators (ignored)
//       'igen'     — sfGen[]            instrument generators
//       'shdr'     — sfSampleHeader[]   sample headers
//==============================================================================
class SF2Parser
{
public:
    // Returns true on success. On failure, error() has the message.
    bool parse (const std::string& path, ConvertBank& out);

    const std::string& error() const { return error_; }

private:
    // RIFF primitives
    struct ChunkHeader { char   id[4]; uint32_t size; };
    struct RiffHeader  { char   id[4]; uint32_t size; char type[4]; };

    // SF2 pdta structures (packed to match on-disk layout)
#pragma pack(push, 1)
    struct sfPresetHeader
    {
        char     achPresetName[20];
        uint16_t wPreset;       // MIDI program number
        uint16_t wBank;         // MIDI bank number
        uint16_t wPresetBagNdx;
        uint32_t dwLibrary;
        uint32_t dwGenre;
        uint32_t dwMorphology;
    };

    struct sfBag
    {
        uint16_t wGenNdx;
        uint16_t wModNdx;
    };

    struct sfGenAmount
    {
        union {
            struct { uint8_t lo, hi; } range;
            int16_t  shAmount;
            uint16_t wAmount;
        };
    };

    struct sfGenList
    {
        uint16_t   sfGenOper;
        sfGenAmount genAmount;
    };

    struct sfInst
    {
        char     achInstName[20];
        uint16_t wInstBagNdx;
    };

    struct sfSampleHeader
    {
        char     achSampleName[20];
        uint32_t dwStart;
        uint32_t dwEnd;
        uint32_t dwStartloop;
        uint32_t dwEndloop;
        uint32_t dwSampleRate;
        uint8_t  byOriginalPitch;
        int8_t   chPitchCorrection;
        uint16_t wSampleLink;
        uint16_t sfSampleType; // 1=mono, 2=right, 4=left, 8=linked, 0x8001=ROM mono
    };
#pragma pack(pop)

    // SF2 generator opcodes we care about
    enum SFGen : uint16_t
    {
        StartAddrsOffset    = 0,
        EndAddrsOffset      = 1,
        StartloopAddrsOffset= 2,
        EndloopAddrsOffset  = 3,
        ModLfoToPitch       = 5,
        KeyRange            = 43,
        VelRange            = 44,
        InitialAttenuation  = 48,
        OverridingRootKey   = 58,
        SampleModes         = 54,
        ScaleTuning         = 56,
        SampleID            = 53,
        FineTune            = 52,
        CoarseTune          = 51,
    };

    // Parsed tables
    std::vector<sfPresetHeader>  phdr_;
    std::vector<sfBag>           pbag_;
    std::vector<sfGenList>       pgen_;
    std::vector<sfInst>          inst_;
    std::vector<sfBag>           ibag_;
    std::vector<sfGenList>       igen_;
    std::vector<sfSampleHeader>  shdr_;
    std::vector<int16_t>         smpl_; // raw 16-bit PCM, all samples

    std::string error_;

    bool readChunks (std::FILE* f);
    bool readListChunk (std::FILE* f, uint32_t listSize, const char* expectedType);

    template<typename T>
    bool readVector (std::FILE* f, uint32_t bytes, std::vector<T>& out);

    void buildPresets (ConvertBank& out) const;
    ConvertZone buildZone (int izonIdx, const sfSampleHeader& shdr) const;

    static float attenuationToGain (int16_t cB) noexcept; // centibels → linear
    static bool  idEq (const char* a, const char* b) noexcept;
};
