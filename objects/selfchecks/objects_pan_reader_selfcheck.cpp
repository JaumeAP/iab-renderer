// Selfcheck of the IAB pan reader and writer: ReadPan, SamePan and FillObjectSubBlock.

#include "iab_pan_reader.h"
#include "iab_pan_writer.h"

#include <common/IABElements.h>

#include <cstdio>

using namespace SMPTE::ImmersiveAudioBitstream;
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

} // namespace

int main()
{
    IABObjectSubBlock sub;
    CartesianPosInUnitCube position;
    // The stream codes: the parser sets positions from 16-bit codes, so use that overload.
    position.setIABObjectPosition(static_cast<uint16_t>(49151), static_cast<uint16_t>(32767),
                                  static_cast<uint16_t>(65535));
    sub.SetObjectPositionFromUnitCube(position);
    IABGain unity;
    unity.setIABGain(1.0f);
    sub.SetObjectGain(unity);

    PanEvent const event = ReadPan(sub, 100);
    Check(event.samplePos == 100, "samplePos is kept");
    Check(event.z == 1.0f, "z code 65535 reads back as exactly 1.0");
    Check(event.x == 0.5f, "x code 49151 reads back as (code - 32767) / 32768");
    Check(event.gain == 1.0f, "unity gain");

    // Two sub-blocks read separately with the same values are the same pan; any single field differing is not.
    IABObjectSubBlock twin;
    twin.SetObjectPositionFromUnitCube(position);
    twin.SetObjectGain(unity);
    Check(SamePan(event, ReadPan(twin, 350)), "two identical sub-blocks are the same pan");
    PanEvent changed = event;
    changed.gain = 0.0f;
    Check(!SamePan(event, changed), "a different gain is a different pan");
    changed = event;
    changed.zoneControl = true;
    Check(!SamePan(event, changed), "different zone control is a different pan");
    changed = event;
    changed.zoneGains[3] = 0.0f;
    Check(!SamePan(event, changed), "a different zone gain is a different pan");
    changed = event;
    changed.decorrelation = 1.0f;
    Check(!SamePan(event, changed), "a different decorrelation is a different pan");
    changed = event;
    changed.spreadX = 0.5f;
    Check(!SamePan(event, changed), "a different spread is a different pan");
    changed = event;
    changed.snap = true;
    Check(!SamePan(event, changed), "a different snap is a different pan");

    // Writing an event into a sub-block and reading it back gives the same pan.
    PanEvent written = PanEvent();
    written.samplePos = 250;
    written.x = 0.25f;
    written.y = 0.5f;
    written.z = 1.0f;
    written.gain = 1.0f;
    for (float &zone : written.zoneGains)
    {
        zone = 1.0f;
    }
    written.spreadMode = 2; // kIABSpreadMode_HighResolution_1D
    IABObjectSubBlock filled;
    FillObjectSubBlock(written, filled);
    Check(SamePan(written, ReadPan(filled, 250)), "FillObjectSubBlock then ReadPan gives the same pan");

    std::printf(gFailures ? "objects_pan_reader_selfcheck: %d failure(s)\n" : "objects_pan_reader_selfcheck: ok\n",
                gFailures);
    return gFailures ? 1 : 0;
}
