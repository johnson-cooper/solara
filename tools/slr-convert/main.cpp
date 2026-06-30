#include "SF2Parser.h"
#include "SFZParser.h"
#include "SLRWriter.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>

static void printUsage (const char* argv0)
{
    std::printf (
        "slr-convert — Solara Sound Resource Converter\n"
        "\n"
        "Usage:\n"
        "  %s <input.sf2|input.sfz> [output.slr] [--verbose] [--gain <dB>]\n"
        "\n"
        "Examples:\n"
        "  %s piano.sf2\n"
        "  %s strings.sfz strings.slr --verbose\n"
        "  %s drums.sf2 drums.slr --gain -3\n"
        "\n"
        "If output.slr is omitted, the .slr is written next to the input file.\n"
        "Converted .slr files go in Documents/Solara/Presets/ to be loaded by Solara.\n",
        argv0, argv0, argv0, argv0);
}

static std::string inferOutputPath (const std::string& input)
{
    const size_t dot = input.rfind ('.');
    const std::string base = (dot != std::string::npos) ? input.substr(0, dot) : input;
    return base + ".slr";
}

static std::string toLower (std::string s)
{
    for (char& c : s) c = (char)std::tolower ((unsigned char)c);
    return s;
}

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage (argv[0]);
        return 1;
    }

    std::string inputPath;
    std::string outputPath;
    bool        verbose = false;
    float       gainDB  = 0.0f;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--verbose" || arg == "-v")
        {
            verbose = true;
        }
        else if ((arg == "--gain" || arg == "-g") && i + 1 < argc)
        {
            gainDB = std::stof (argv[++i]);
        }
        else if (arg[0] == '-')
        {
            std::fprintf (stderr, "Unknown option: %s\n", arg.c_str());
            return 1;
        }
        else if (inputPath.empty())
        {
            inputPath = arg;
        }
        else if (outputPath.empty())
        {
            outputPath = arg;
        }
    }

    if (inputPath.empty())
    {
        printUsage (argv[0]);
        return 1;
    }

    if (outputPath.empty())
        outputPath = inferOutputPath (inputPath);

    // Determine format from extension
    const std::string ext = toLower (inputPath.substr (inputPath.rfind('.') + 1));

    ConvertBank bank;
    bool ok = false;

    if (ext == "sf2")
    {
        std::printf ("Parsing SF2: %s\n", inputPath.c_str());
        SF2Parser parser;
        ok = parser.parse (inputPath, bank);
        if (!ok)
        {
            std::fprintf (stderr, "SF2 parse error: %s\n", parser.error().c_str());
            return 2;
        }
    }
    else if (ext == "sfz")
    {
        std::printf ("Parsing SFZ: %s\n", inputPath.c_str());
        SFZParser parser;
        ok = parser.parse (inputPath, bank);
        if (!ok)
        {
            std::fprintf (stderr, "SFZ parse error: %s\n", parser.error().c_str());
            return 2;
        }
    }
    else
    {
        std::fprintf (stderr, "Unsupported format: .%s (expected .sf2 or .sfz)\n", ext.c_str());
        return 1;
    }

    std::printf ("Parsed: %d preset(s)\n", (int)bank.presets.size());
    if (verbose)
    {
        for (const auto& p : bank.presets)
            std::printf ("  [%d:%d] %s — %d zones\n",
                         p.bank, p.program, p.name.c_str(), (int)p.zones.size());
    }

    SLRWriter::Options writerOpts;
    writerOpts.verbose     = verbose;
    writerOpts.normalizeGain = (gainDB == 0.0f) ? 1.0f
                                                : std::pow (10.0f, gainDB / 20.0f);

    std::printf ("Writing SLR: %s\n", outputPath.c_str());
    SLRWriter writer;
    if (!writer.write (bank, outputPath, writerOpts))
    {
        std::fprintf (stderr, "Write error: %s\n", writer.error().c_str());
        return 3;
    }

    std::printf ("Done. Copy %s to Documents/Solara/Presets/ to use in Solara.\n",
                 outputPath.c_str());
    return 0;
}
