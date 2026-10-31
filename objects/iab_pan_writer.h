// Writes the pan values of a PanEvent into an IAB object sub-block.

#ifndef CINEIA_OBJECTS_IAB_PAN_WRITER_H
#define CINEIA_OBJECTS_IAB_PAN_WRITER_H

#include "object_types.h"

namespace SMPTE {
namespace ImmersiveAudioBitstream {
class IABObjectSubBlock;
}
}

namespace CineIA {
namespace Objects {

// Sets PanInfoExists, gain, position, snap, zone gains, 1D spread (mode 2, 12 bits, from spreadX) and
// decorrelation (none or maximum) of oSubBlock from iEvent.
void FillObjectSubBlock(PanEvent const &iEvent, SMPTE::ImmersiveAudioBitstream::IABObjectSubBlock &oSubBlock);

} // namespace Objects
} // namespace CineIA

#endif // #ifndef CINEIA_OBJECTS_IAB_PAN_WRITER_H
