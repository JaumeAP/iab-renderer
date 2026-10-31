// Reads the pan values of an IAB object sub-block into the neutral PanEvent.

#ifndef CINEIA_OBJECTS_IAB_PAN_READER_H
#define CINEIA_OBJECTS_IAB_PAN_READER_H

#include "object_types.h"

#include <cstdint>

namespace SMPTE {
namespace ImmersiveAudioBitstream {
class IABObjectSubBlock;
}
}

namespace CineIA {
namespace Objects {

// True when every pan value of a equals that of b (samplePos is not compared).
bool SamePan(PanEvent const &a, PanEvent const &b);

// The pan values of iSubBlock, at sample position iSamplePos.
PanEvent ReadPan(SMPTE::ImmersiveAudioBitstream::IABObjectSubBlock const &iSubBlock, uint64_t iSamplePos);

} // namespace Objects
} // namespace CineIA

#endif // #ifndef CINEIA_OBJECTS_IAB_PAN_READER_H
