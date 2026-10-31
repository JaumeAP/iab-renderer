// Selfcheck of the ADM block reader: SampleObjectBlocks.

#include "object_block_reader.h"

#include <cstdio>

using namespace CineIA::Objects;

namespace {

int gFailures = 0;

void Check(bool iCondition, char const *iWhat)
{
    if (!iCondition)
    {
        std::printf("FAIL: %s\n", iWhat);
        ++gFailures;
    }
}

ObjectBlock Block(double iRtime, float iX, float iZ = 0.0f)
{
    ObjectBlock block;
    block.rtime = iRtime;
    block.x = iX;
    block.z = iZ;
    return block;
}

} // namespace

int main()
{
    std::vector<IabSubBlock> subs;
    std::string error;

    // The second block starts at sample 504: sub-blocks 0 and 1 (last samples 249 and 499) take the first,
    // sub-block 2 (last sample 749) the second.
    std::vector<ObjectBlock> two = {Block(0.0, 1.0f), Block(0.0105, -1.0f)};
    Check(SampleObjectBlocks("AO_1001", two, 48000, 1, subs, error), "two blocks convert");
    Check(subs.size() == 8, "one frame is 8 sub-blocks");
    if (subs.size() == 8)
    {
        Check(subs[0].x == 65535 && subs[1].x == 65535, "x = 1 quantises to 65535");
        Check(subs[2].x == 32767 && subs[7].x == 32767, "x = -1 quantises to 32767");
    }

    // Z is rounded to the nearest code, as the Conversion Tool v2.1.2 writes it.
    {
        std::vector<IabSubBlock> zs;
        Check(SampleObjectBlocks("AO_1001", {Block(0.0, 0.0f, 0.25f)}, 48000, 1, zs, error) && zs[0].z == 0x4000,
              "Z 0.25 is 0x4000");
        Check(SampleObjectBlocks("AO_1001", {Block(0.0, 0.0f, 0.5f)}, 48000, 1, zs, error) && zs[0].z == 0x8000,
              "Z 0.5 is 0x8000");
        Check(SampleObjectBlocks("AO_1001", {Block(0.0, 0.0f, 0.75f)}, 48000, 1, zs, error) && zs[0].z == 0xbfff,
              "Z 0.75 is 0xbfff");
        Check(SampleObjectBlocks("AO_1001", {Block(0.0, 0.0f, 1.0f)}, 48000, 1, zs, error) && zs[0].z == 0xffff,
              "Z 1 is 0xffff");
    }

    // A 96 kHz file gives the same sub-blocks as a 48 kHz one: block times are seconds, sub-blocks are 48 kHz IAB.
    {
        std::vector<IabSubBlock> at96;
        Check(SampleObjectBlocks("AO_1001", two, 96000, 1, at96, error), "96 kHz converts");
        Check(at96.size() == subs.size() && at96.size() == 8 && at96[2].x == subs[2].x && at96[1].x == subs[1].x,
              "96 kHz gives the sub-blocks of 48 kHz");
    }

    // The block's gain reaches the sub-blocks where it is active; a block without one gives 1.
    {
        std::vector<ObjectBlock> gained = {Block(0.0, 1.0f), Block(0.0105, -1.0f)};
        gained[1].gain = 0.5f;
        std::vector<IabSubBlock> g;
        Check(SampleObjectBlocks("AO_1001", gained, 48000, 1, g, error) && g[0].gain == 1.0f && g[1].gain == 1.0f &&
                  g[2].gain == 0.5f && g[7].gain == 0.5f,
              "the gain of the active block reaches each sub-block");
    }

    Check(!SampleObjectBlocks("AO_1001", two, 44100, 1, subs, error), "44100 Hz is refused");
    Check(!SampleObjectBlocks("AO_1001", {Block(0.0, 0.0f, -0.1f)}, 48000, 1, subs, error), "Z below 0 is refused");
    Check(!SampleObjectBlocks("AO_1001", {Block(0.01, 0.0f)}, 48000, 1, subs, error),
          "a first block after the first sub-block's last sample is refused");
    Check(error.find("AO_1001") != std::string::npos, "the error names the object");

    std::printf(gFailures ? "objects_block_reader_selfcheck: %d failure(s)\n" : "objects_block_reader_selfcheck: ok\n",
                gFailures);
    return gFailures ? 1 : 0;
}
