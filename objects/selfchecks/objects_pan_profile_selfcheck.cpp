// Selfcheck of the writer profiles for object pans: standard IAB and Dolby.

#include "pan_profile.h"

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

PanEvent Event(uint64_t iSamplePos, float iGain)
{
    PanEvent event = PanEvent();
    event.samplePos = iSamplePos;
    event.x = event.y = event.z = 0.5f;
    event.gain = iGain;
    for (float &zone : event.zoneGains)
    {
        zone = 1.0f;
    }
    event.spreadMode = 2; // kIABSpreadMode_HighResolution_1D
    return event;
}

ObjectTrack Track(float iGain)
{
    ObjectTrack track;
    track.metaId = 7;
    track.events.push_back(Event(0, iGain));
    return track;
}

bool Validate(PanProfile const &iProfile, ObjectTrack const &iTrack, std::string &oError)
{
    return ValidateObjectTrack(iProfile, iTrack, 2000, 4000, 250, oError);
}

} // namespace

int main()
{
    std::string error;
    PanProfile const &standard = StandardIabPanProfile();
    PanProfile const &dolby = DolbyPanProfile();

    Check(Validate(standard, Track(1.0f), error), "standard accepts unity gain");
    Check(Validate(dolby, Track(1.0f), error), "dolby accepts unity gain");

    Check(!Validate(standard, Track(0.0f), error), "standard refuses gain 0.0");
    Check(error.find("RDD 57 O5") != std::string::npos, "the standard error names RDD 57 O5");
    Check(Validate(dolby, Track(0.0f), error), "dolby accepts gain 0.0");

    Check(!Validate(standard, Track(0.5f), error), "standard refuses gain 0.5");
    Check(!Validate(dolby, Track(0.5f), error), "dolby refuses gain 0.5");

    ObjectTrack twice = Track(1.0f);
    twice.events.push_back(Event(0, 1.0f));
    Check(!Validate(standard, twice, error), "two events at the same sample are refused");
    Check(error.find("not in increasing order") != std::string::npos, "the error says the order");

    std::printf(gFailures ? "objects_pan_profile_selfcheck: %d failure(s)\n" : "objects_pan_profile_selfcheck: ok\n",
                gFailures);
    return gFailures ? 1 : 0;
}
