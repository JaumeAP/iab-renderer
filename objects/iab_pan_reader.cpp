#include "iab_pan_reader.h"

#include <common/IABElements.h>

#include <algorithm>

namespace CineIA {
namespace Objects {

using namespace SMPTE::ImmersiveAudioBitstream;

bool SamePan(PanEvent const &a, PanEvent const &b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z && a.gain == b.gain && a.snap == b.snap &&
           a.zoneControl == b.zoneControl && std::equal(a.zoneGains, a.zoneGains + 9, b.zoneGains) &&
           a.spreadMode == b.spreadMode && a.spreadX == b.spreadX && a.spreadY == b.spreadY &&
           a.spreadZ == b.spreadZ && a.decorrelation == b.decorrelation;
}

PanEvent ReadPan(IABObjectSubBlock const &iSubBlock, uint64_t iSamplePos)
{
    PanEvent event;
    event.samplePos = iSamplePos;
    CartesianPosInUnitCube position;
    iSubBlock.GetObjectPositionToUnitCube(position);
    position.getIABObjectPosition(event.x, event.y, event.z);
    IABGain gain;
    iSubBlock.GetObjectGain(gain);
    event.gain = gain.getIABGain();
    IABObjectSnap snap;
    iSubBlock.GetObjectSnap(snap);
    event.snap = snap.objectSnapPresent_ != 0;
    IABObjectZoneGain9 zones;
    iSubBlock.GetObjectZoneGains9(zones);
    event.zoneControl = zones.objectZoneControl_ != 0;
    for (int i = 0; i < 9; ++i)
    {
        event.zoneGains[i] = event.zoneControl ? zones.zoneGains_[i].getIABZoneGain() : 1.0f;
    }
    IABObjectSpread spread;
    iSubBlock.GetObjectSpread(spread);
    event.spreadMode = spread.getIABObjectSpreadMode();
    spread.getIABObjectSpread(event.spreadX, event.spreadY, event.spreadZ);
    IABDecorCoeff decor;
    iSubBlock.GetDecorCoef(decor);
    event.decorrelation = decor.decorCoefPrefix_ == kIABDecorCoeffPrefix_MaxDecor      ? 1.0f
                          : decor.decorCoefPrefix_ == kIABDecorCoeffPrefix_DecorCoeffInStream ? decor.decorCoef_ / 255.0f
                                                                                            : 0.0f;
    return event;
}

} // namespace Objects
} // namespace CineIA
