#include "object_block_reader.h"

#include <cmath>

namespace CineIA {
namespace Objects {

namespace {

uint32_t const kIabSampleRate = 48000;
uint32_t const kSubBlockSamples = 250; // 8 sub-blocks per frame of 24 fps
uint32_t const kSubBlocksPerFrame = 8;
uint64_t const kFrameSamples = kSubBlockSamples * kSubBlocksPerFrame;

// X and Y: same arithmetic as the reference file, float32, floor, not rounding.
uint16_t QuantiseAxis(float iAdm)
{
    return static_cast<uint16_t>(std::floor(16383.75f * iAdm + 49151.25f));
}

uint16_t QuantiseY(float iAdm)
{
    return static_cast<uint16_t>(std::floor(49151.25f - 16383.75f * iAdm));
}

// Z: rounded to the nearest code, as the Conversion Tool v2.1.2 writes it (round(Z * 0xffff)). Dolby's real IAB
// file matches floor instead; the Tool is the reference (decision of the user, 2026-10-29).
uint16_t QuantiseZ(float iAdm)
{
    return static_cast<uint16_t>(std::floor(65535.0f * iAdm + 0.5f));
}

} // namespace

bool SampleObjectBlocks(std::string const &iObjectId, std::vector<ObjectBlock> const &iBlocks, uint32_t iSampleRate,
                        uint32_t iFrameCount, std::vector<IabSubBlock> &oSubBlocks, std::string &oError)
{
    if (iSampleRate != kIabSampleRate && iSampleRate != 2 * kIabSampleRate)
    {
        oError = "object " + iObjectId + ": IAB is 48 kHz and a 96 kHz file is converted, the file is " +
                 std::to_string(iSampleRate) + " Hz";
        return false;
    }
    if (iBlocks.empty())
    {
        oError = "object " + iObjectId + " has no audioBlockFormat";
        return false;
    }

    std::vector<uint64_t> starts;
    for (ObjectBlock const &block : iBlocks)
    {
        if (block.z < 0)
        {
            oError = "object " + iObjectId + ": Z below 0 has no established IAB value";
            return false;
        }
        uint64_t const start = static_cast<uint64_t>(std::llround(block.rtime * kIabSampleRate));
        if (!starts.empty() && start < starts.back())
        {
            oError = "object " + iObjectId + ": audioBlockFormat blocks are not in time order";
            return false;
        }
        starts.push_back(start);
    }

    oSubBlocks.clear();
    oSubBlocks.reserve(static_cast<size_t>(iFrameCount) * kSubBlocksPerFrame);
    size_t block = 0;
    for (uint32_t frame = 0; frame < iFrameCount; ++frame)
    {
        for (uint32_t sub = 0; sub < kSubBlocksPerFrame; ++sub)
        {
            uint64_t const last = frame * kFrameSamples + sub * kSubBlockSamples + kSubBlockSamples - 1;
            if (starts[0] > last)
            {
                oError = "object " + iObjectId + ": no block is active at sample " + std::to_string(last);
                return false;
            }
            while (block + 1 < starts.size() && starts[block + 1] <= last)
            {
                ++block;
            }
            ObjectBlock const &active = iBlocks[block];
            IabSubBlock out;
            out.x = QuantiseAxis(active.x);
            out.y = QuantiseY(active.y);
            out.z = QuantiseZ(active.z);
            out.spread = static_cast<uint16_t>(std::floor(active.size * 4095.0f));
            out.decorrelation = active.diffuse;
            out.inactive = active.inactive;
            out.gain = active.gain;
            oSubBlocks.push_back(out);
        }
    }
    return true;
}

} // namespace Objects
} // namespace CineIA
