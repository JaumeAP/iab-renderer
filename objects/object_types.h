// Neutral types of the objects part: what an object does over time, independent of ADM files and MXF files.

#ifndef CINEIA_OBJECTS_OBJECT_TYPES_H
#define CINEIA_OBJECTS_OBJECT_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

namespace CineIA {
namespace Objects {

// One pan sub-block with PanInfoExists set, kept only when it differs from
// the previous one of the same object.
struct PanEvent {
    uint64_t samplePos;
    float x, y, z;          // unit cube of ST 2098-2 11.1: x left to right, y front to back, z floor to ceiling
    float gain;             // linear
    bool snap;
    bool zoneControl;
    float zoneGains[9];     // linear, ST 2098-2 / RDD 29 zone order; all 1 when zoneControl is false
    int spreadMode;         // IABSpreadModeType
    float spreadX, spreadY, spreadZ;
    float decorrelation;    // 0 none, 1 maximum, else ObjectDecorCoef / 255
};

struct ObjectTrack {
    uint32_t metaId;
    std::string file;       // WAV file name inside the output folder
    std::vector<PanEvent> events;
};

// One block of an object's movement as an ADM audioBlockFormat gives it.
struct ObjectBlock {
    double rtime = 0;           // seconds from the start of the object
    double duration = 0;        // seconds
    float x = 0, y = 0, z = 0;  // cartesian, X left to right, Y back to front, Z down to up, in [-1, 1]
    float size = 0;             // width = height = depth, in [0, 1]; 0 when absent
    bool diffuse = false;
    bool inactive = false;      // gain 0.0 and importance 0 are present: the object is silent
    float gain = 1.0f;          // linear, the ADM gain; 1 when absent
};

// The IAB pan values of one sub-block of 250 samples.
struct IabSubBlock {
    uint16_t x = 0, y = 0, z = 0; // ST 2098-2 unit cube, 16 bits: x left to right, y front to back, z floor to ceiling
    uint16_t spread = 0;          // 1D ObjectSpread, 12 bits; 0 when the ADM object has no width
    bool decorrelation = false;   // ADM diffuse 1
    bool inactive = false;        // ADM gain 0.0 and importance 0 in the block that is active here. No IAB field
                                  // carries it yet: the mapping of an inactive block is not established.
    float gain = 1.0f;            // linear, the ADM gain of the block that is active here; 1 when it has none
};

} // namespace Objects
} // namespace CineIA

#endif // #ifndef CINEIA_OBJECTS_OBJECT_TYPES_H
