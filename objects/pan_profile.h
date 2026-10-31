// What the writer of object pans asks of the input, per profile: the standard IAB profile (ST 2098-2 within
// RDD 57, IAB Application Profile 1) and the Dolby profile (what Dolby's storage rel-2.1 cinema writer does,
// mxf/docs/dolby-cinema-mxf-writer.md). Input that a profile does not allow is refused with an error; it is
// never changed to fit.

#ifndef CINEIA_OBJECTS_PAN_PROFILE_H
#define CINEIA_OBJECTS_PAN_PROFILE_H

#include "object_types.h"

#include <cstdint>
#include <string>

namespace CineIA {
namespace Objects {

struct PanProfile {
    char const *name;
    // Checks the gain rule of this profile on one event. The checks both profiles share have run before it.
    bool (*validateEvent)(ObjectTrack const &iObject, PanEvent const &iEvent, std::string &oError);
};

PanProfile const &StandardIabPanProfile();
PanProfile const &DolbyPanProfile();

// Checks one object against what every profile needs (events present and in time order, at the start of a
// frame and of a pan sub-block, values in range, RDD 57 spread, decorrelation and zone sets) and then each
// event against iProfile. iFrameSamples is the samples of one frame, iTotalSamples those of the whole input,
// iSubBlockSamples those of one pan sub-block.
bool ValidateObjectTrack(PanProfile const &iProfile, ObjectTrack const &iObject, uint32_t iFrameSamples,
                         uint64_t iTotalSamples, uint32_t iSubBlockSamples, std::string &oError);

} // namespace Objects
} // namespace CineIA

#endif // #ifndef CINEIA_OBJECTS_PAN_PROFILE_H
