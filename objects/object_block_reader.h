// Samples the blocks of one object, as an ADM audioObject gives them, into IAB pan values: one unit-cube
// position, a 1D spread and a decorrelation flag per sub-block of 250 samples at 48 kHz (ST 2098-2 Table 8:
// 8 sub-blocks per 24 fps frame). Formulas and sampling rule are the ones measured on Dolby's own IAB track
// file for the same ADM (docs/adm-to-iab-mxf-design.md, "Real Dolby reference"). Nothing here reads an ADM
// file or writes a file.

#ifndef CINEIA_OBJECTS_OBJECT_BLOCK_READER_H
#define CINEIA_OBJECTS_OBJECT_BLOCK_READER_H

#include "object_types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace CineIA {
namespace Objects {

// Converts iBlocks over iFrameCount frames of 24 fps from sample 0 at iSampleRate. Sub-block j of frame N
// takes the values of the block active at its last sample (N * 2000 + 250 * j + 249). A block is active
// from round(rtime * iSampleRate) on. The ADM interpolationLength is not used. Returns false and fills
// oError (naming iObjectId) when iSampleRate is not 48000 or 96000 (block times are seconds, so a 96 kHz file gives
// the same sub-blocks), when the blocks do not start in order, when the
// first block starts after a sub-block's last sample, or when a block has Z below 0 (its IAB value is not
// established).
bool SampleObjectBlocks(std::string const &iObjectId, std::vector<ObjectBlock> const &iBlocks, uint32_t iSampleRate,
                        uint32_t iFrameCount, std::vector<IabSubBlock> &oSubBlocks, std::string &oError);

} // namespace Objects
} // namespace CineIA

#endif // #ifndef CINEIA_OBJECTS_OBJECT_BLOCK_READER_H
