# Objects: state (2026-11-01)

Moved here from Cinema-MXF (`JaumeAP/Cinema-MXF`, PR 99) on 2026-11-01, on request of the owner: it sits on top of
IABLib and both sides use it, so it lives with IABLib. Nothing in IABLib depends on this folder. It holds the object
pan and movement code that New-Renderer's `adm/` (ADM BWF) and Cinema-MXF's `mxf/` (MXF track files) share,
independent of both: `objects/` includes no header of `adm/` or `mxf/`. Design: `design.md`. Plan: `plan.md` (both
written when the folder was in Cinema-MXF; their paths `adm/` and `mxf/` are those repositories). The selfchecks are in
`selfchecks/`.

## What exists

1. `object_types.h`: the neutral types. `PanEvent` and `ObjectTrack` (pan sub-blocks of an object as read from or
   written to IAB), `ObjectBlock` (one block of an object's movement as an ADM audioBlockFormat gives it) and
   `IabSubBlock` (the quantised IAB pan values of one sub-block of 250 samples).
2. Reader:
   - `iab_pan_reader` (`ReadPan`, `SamePan`): the pan values of an IAB object sub-block, from IABLib.
   - `object_block_reader` (`SampleObjectBlocks`): ADM-style blocks to one `IabSubBlock` per 250 samples at
     48 kHz; a 96 kHz file gives the same sub-blocks. Z is rounded (`round(Z * 0xffff)`), X and Y floored. The ADM
     `interpolationLength` is not used. `adm/` converts its `AdmObject` to `ObjectBlock`
     (`ConvertObject`).
3. Writer:
   - `iab_pan_writer` (`FillObjectSubBlock`): a `PanEvent` into an IAB object sub-block.
   - `pan_profile` (`StandardIabPanProfile`, `DolbyPanProfile`, `ValidateObjectTrack`): what each profile asks
     of the pans. Both: events in time order at the start of a pan sub-block, positions in the unit cube, RDD 57
     spread mode 2, decorrelation none or maximum, zone gains in the sets of RDD 57 table 4. Standard IAB: gain
     1.0 only (RDD 57 O5). Dolby: gain 0.0 or 1.0 only.

## Build and checks

`adm/` and `mxf/` add this folder with `add_subdirectory` after IABLib and link the library `objects`; it is
compiled with their C++ standard (11 for `mxf/`, 14 for `adm/`). The selfchecks are built with either part:
`objects_pan_reader_selfcheck` (reader, writer round trip), `objects_block_reader_selfcheck`,
`objects_pan_profile_selfcheck`. Run `<build>/objects/<name>`.

Verified on 2026-10-29: `adm/` and `mxf/` build and all their selfchecks plus the three above pass. Building
IABLib needs googletest, which its CMake downloads; offline, point `FETCHCONTENT_SOURCE_DIR_GOOGLETEST` at a
googletest checkout. libadm needs the Boost headers.

## Not moved

The ADM-side block reading (`AdmObjectBlock` in `adm_bwf_reader`), the ADM writing of blocks
(`adm_model_build`, `adm_bwf_writer`) and the reading of the pan events out of the MXF (`iab_extract.cpp`
calls `ReadPan`) stay in `adm/` and `mxf/`: they are about their files, not about the object pans.
