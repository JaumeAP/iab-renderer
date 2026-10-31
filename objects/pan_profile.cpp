#include "pan_profile.h"

#include <IABDataTypes.h>


namespace CineIA {
namespace Objects {

using namespace SMPTE::ImmersiveAudioBitstream;

namespace {

// RDD 57 table 4 (the same sets as IABLib's Dolby cinema presets): zone gains 1 to 9 in ST 2098-2 order.
bool ZoneSetAllowed(float const iGains[9])
{
    static bool const kSets[11][9] = {
        {1, 1, 1, 1, 1, 0, 0, 1, 1},  // 1a
        {1, 1, 1, 1, 1, 0, 0, 0, 0},  // 1b
        {1, 1, 1, 0, 0, 1, 1, 1, 1},  // 2a
        {1, 1, 1, 0, 0, 1, 1, 0, 0},  // 2b
        {0, 1, 0, 0, 0, 1, 1, 1, 1},  // 3a
        {0, 1, 0, 0, 0, 1, 1, 0, 0},  // 3b
        {1, 1, 1, 0, 0, 0, 0, 1, 1},  // 4a
        {1, 1, 1, 0, 0, 0, 0, 0, 0},  // 4b
        {0, 0, 0, 1, 1, 1, 1, 1, 1},  // 5a
        {0, 0, 0, 1, 1, 1, 1, 0, 0},  // 5b
        {1, 1, 1, 1, 1, 1, 1, 0, 0},  // 6
    };
    for (auto const &set : kSets)
    {
        bool same = true;
        for (int z = 0; z < 9; ++z)
        {
            same = same && iGains[z] == (set[z] ? 1.0f : 0.0f);
        }
        if (same)
        {
            return true;
        }
    }
    return false;
}

std::string Where(ObjectTrack const &iObject, PanEvent const &iEvent)
{
    return "object " + std::to_string(iObject.metaId) + " at sample " + std::to_string(iEvent.samplePos);
}

// RDD 57 O5: ObjectGainPrefix is 0 (unity) and the bitstream has no ObjectGain.
bool ValidateStandardEvent(ObjectTrack const &iObject, PanEvent const &iEvent, std::string &oError)
{
    if (iEvent.gain != 1.0f)
    {
        oError = "RDD 57 O5 allows only unity object gain; object " + std::to_string(iObject.metaId) + " has gain " +
                 std::to_string(iEvent.gain) + " at sample " + std::to_string(iEvent.samplePos);
        return false;
    }
    return true;
}

// Dolby's writer codes an object's gain as unity when the object is active and as silence otherwise;
// it writes no other gain.
bool ValidateDolbyEvent(ObjectTrack const &iObject, PanEvent const &iEvent, std::string &oError)
{
    if (iEvent.gain != 0.0f && iEvent.gain != 1.0f)
    {
        oError = "Dolby's writer codes object gain as unity or silence only; object " + std::to_string(iObject.metaId) +
                 " has gain " + std::to_string(iEvent.gain) + " at sample " + std::to_string(iEvent.samplePos);
        return false;
    }
    return true;
}

PanProfile const kStandard = {"standard IAB", ValidateStandardEvent};
PanProfile const kDolby = {"Dolby", ValidateDolbyEvent};

} // namespace

PanProfile const &StandardIabPanProfile()
{
    return kStandard;
}

PanProfile const &DolbyPanProfile()
{
    return kDolby;
}

bool ValidateObjectTrack(PanProfile const &iProfile, ObjectTrack const &object, uint32_t iFrameSamples,
                         uint64_t totalSamples, uint32_t step, std::string &oError)
{
    if (object.events.empty())
    {
        oError = "object " + std::to_string(object.metaId) + " has no pan events";
        return false;
    }
    if (object.events.front().samplePos % iFrameSamples != 0)
    {
        oError = "the first pan event of object " + std::to_string(object.metaId) +
                 " is not at the start of a frame (sample " + std::to_string(object.events.front().samplePos) + ")";
        return false;
    }
    for (std::size_t e = 0; e < object.events.size(); ++e)
    {
        PanEvent const &event = object.events[e];
        if (e > 0 && event.samplePos <= object.events[e - 1].samplePos)
        {
            oError = "pan events of object " + std::to_string(object.metaId) + " are not in increasing order";
            return false;
        }
        if (event.samplePos >= totalSamples || event.samplePos % step != 0)
        {
            oError = "pan event of " + Where(object, event) + " is not at the start of a pan sub-block";
            return false;
        }
        if (event.x < 0 || event.x > 1 || event.y < 0 || event.y > 1 || event.z < 0 || event.z > 1)
        {
            oError = "position outside the unit cube in " + Where(object, event);
            return false;
        }
        if (event.gain < 0 || event.gain > 1)
        {
            oError = "gain outside 0 to 1 in " + Where(object, event);
            return false;
        }
        // RDD 57 O6 and IABLib's Dolby cinema set: ObjectSpreadMode 0x02 (1D, 12 bits), always.
        if (event.spreadMode != kIABSpreadMode_HighResolution_1D || event.spreadX < 0 || event.spreadX > 1)
        {
            oError = "RDD 57 O6 allows spread mode 2 only; " + Where(object, event) + " has mode " +
                     std::to_string(event.spreadMode);
            return false;
        }
        // RDD 57 O8: decorrelation prefix 0 or 1, no coefficient.
        if (event.decorrelation != 0.0f && event.decorrelation != 1.0f)
        {
            oError = "RDD 57 O8 allows no decorrelation or the maximum only; " + Where(object, event) + " has " +
                     std::to_string(event.decorrelation);
            return false;
        }
        // RDD 57 O4: zone gains 0 or 1, in the combinations of table 4.
        if (event.zoneControl && !ZoneSetAllowed(event.zoneGains))
        {
            oError = "the zone gains of " + Where(object, event) + " are not one of the sets of RDD 57 table 4";
            return false;
        }
    }
    for (PanEvent const &event : object.events)
    {
        if (!iProfile.validateEvent(object, event, oError))
        {
            return false;
        }
    }
    return true;
}

} // namespace Objects
} // namespace CineIA
