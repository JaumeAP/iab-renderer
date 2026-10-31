#include "iab_pan_writer.h"

#include <common/IABElements.h>

namespace CineIA {
namespace Objects {

using namespace SMPTE::ImmersiveAudioBitstream;

void FillObjectSubBlock(PanEvent const &iEvent, IABObjectSubBlock &oSubBlock)
{
    IABGain gain;
    gain.setIABGain(iEvent.gain);
    CartesianPosInUnitCube point;
    point.setIABObjectPosition(iEvent.x, iEvent.y, iEvent.z);
    IABObjectSnap snap = IABObjectSnap();
    snap.objectSnapPresent_ = iEvent.snap ? 1 : 0;
    IABObjectZoneGain9 zones = IABObjectZoneGain9();
    zones.objectZoneControl_ = iEvent.zoneControl ? 1 : 0;
    for (int z = 0; z < 9 && iEvent.zoneControl; ++z)
    {
        zones.zoneGains_[z].setIABZoneGain(iEvent.zoneGains[z]);
    }
    IABObjectSpread spread;
    spread.setIABObjectSpread(kIABSpreadMode_HighResolution_1D, iEvent.spreadX, iEvent.spreadX, iEvent.spreadX);
    IABDecorCoeff decor = IABDecorCoeff();
    decor.decorCoefPrefix_ = iEvent.decorrelation == 1.0f ? kIABDecorCoeffPrefix_MaxDecor : kIABDecorCoeffPrefix_NoDecor;
    oSubBlock.SetPanInfoExists(1);
    oSubBlock.SetObjectGain(gain);
    oSubBlock.SetObjectPositionFromUnitCube(point);
    oSubBlock.SetObjectSnap(snap);
    oSubBlock.SetObjectZoneGains9(zones);
    oSubBlock.SetObjectSpread(spread);
    oSubBlock.SetDecorCoef(decor);
}

} // namespace Objects
} // namespace CineIA
