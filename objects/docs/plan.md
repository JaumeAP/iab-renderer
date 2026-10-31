# Objects Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move object pan and movement code of `adm/` and `mxf/` into `objects/`, with a reader (ADM blocks, IAB pan sub-blocks) and a writer (standard IAB and Dolby profiles), with no change of behaviour.

**Architecture:** `objects/` is a static library `objects` linked by `adm/` and `mxf/`; it never includes a header of `adm/` or `mxf/`. A neutral `ObjectBlock` stands between the ADM reader (adapter stays in `adm/`) and the sampling code (moves to `objects/`), so `objects/` does not depend on `adm/`. This is a call of the plan: the design says the reader takes `AdmObject`, which would make `objects/` depend on `adm/`.

**Tech Stack:** C++14 (libadm needs it), CMake, IABLib from `NEW_RENDERER_THIRD_PARTY` (default `../..`, New-Renderer's `third_party/`).

**Spec:** `objects/docs/design.md`

## Global Constraints

1. Everything written in the repository is English.
2. C++14. Namespaces: `CineIA::Objects` for the new code; `CineIA::ADM` and `IABRender::MXF` keep theirs.
3. `objects/` includes no header of `adm/` or `mxf/`; `adm/` and `mxf/` include none of each other.
4. Behaviour unchanged: every error message that exists today keeps its text; every existing selfcheck passes unmodified.
5. `objects/` does not read or write ADM files, MXF files, IAFrames, beds or DLC audio.
6. Builds out of tree, in the session scratchpad: `cmake -S <part> -B <build dir> -G Ninja`, with `NEW_RENDERER_THIRD_PARTY` left at its default when the repository is checked out as `New-Renderer/third_party/Cinema-MXF`.

## Review Focus

1. Two identical consecutive pan sub-blocks give one `PanEvent` (`SamePan`): Task 3.
2. Z = 65535 reads back as exactly 1.0 (z-decode fix of the IABLib fork): Task 3.
3. A block starting after the first sub-block's last sample, and Z below 0, stay errors: Task 4.
4. Sample rate other than 48000 stays an error: Task 4.
5. Gain 0.0 is refused by the standard IAB profile and accepted by the Dolby profile; gain 0.5 is refused by both: Task 5.

---

### Task 1: Baseline and `objects` library skeleton

**Files:**
- Create: `objects/CMakeLists.txt`
- Modify: `adm/CMakeLists.txt`, `mxf/CMakeLists.txt` (one `add_subdirectory` each, after IABLib is added)

**Interfaces:**
- Produces: CMake target `objects` (STATIC, links `IABLib`, public include dir `objects/`).

- [ ] **Step 1: Baseline.** Build `adm` and `mxf` and run all selfchecks before touching anything: `adm_bed_mapping_selfcheck`, `adm_bwf_reader_selfcheck`, `adm_iab_convert_selfcheck`, `adm_model_build_selfcheck`, `adm_bwf_writer_selfcheck`, `adm_profile_selfcheck`, `adm_libs_selfcheck`, `iab_extract_selfcheck`, `iab_preamble_selfcheck`, `iab_write_selfcheck`. Record which pass. Expected: all pass (`docs/state.md` of each part); a failure here is reported to the user, not fixed.
- [ ] **Step 2:** `objects/CMakeLists.txt`: `add_library(objects STATIC object_types.h)` with `target_link_libraries(objects PUBLIC IABLib)` and `target_include_directories(objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${IABLib_HEADER_PATH})`; `message(FATAL_ERROR ...)` when `TARGET IABLib` does not exist. `objects` has no `project()`; it is always added from `adm/` or `mxf/`.
- [ ] **Step 3:** In `adm/CMakeLists.txt` and `mxf/CMakeLists.txt`, right after the `IABLib_HEADER_PATH` lines: `add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../objects objects)`. Link `objects` into `adm2iab`, `iab_extract` and `iab_write` (`target_link_libraries(... PUBLIC objects)`).
- [ ] **Step 4:** Rebuild both parts. Expected: builds, same selfchecks pass.
- [ ] **Step 5:** Commit `build: add the objects library skeleton`.

### Task 2: Neutral types

**Files:**
- Create: `objects/object_types.h`
- Modify: `mxf/iab_extract.h` (remove `PanEvent` and `ObjectTrack`, add `using Objects::PanEvent; using Objects::ObjectTrack;` inside `IABRender::MXF`)

**Interfaces:**
- Produces, in `CineIA::Objects`: `struct PanEvent` and `struct ObjectTrack` with the exact fields of `mxf/iab_extract.h:27-47` today; `struct ObjectBlock { double rtime = 0, duration = 0; float x = 0, y = 0, z = 0; float size = 0; bool diffuse = false; bool inactive = false; };` (ADM-style values, X/Y/Z in [-1, 1]); `struct IabSubBlock` with the fields of `adm/adm_iab_convert.h:20-27` (moved there in Task 4).

- [ ] **Step 1: Write the types**; the builder copies `PanEvent`/`ObjectTrack` verbatim.
- [ ] **Step 2: Verify.** Rebuild `mxf`; run `iab_extract_selfcheck` and `iab_write_selfcheck`. Expected: pass, unmodified.
- [ ] **Step 3: Commit** `objects: move PanEvent and ObjectTrack to the neutral types`.

### Task 3: IAB pan reader

**Files:**
- Create: `objects/iab_pan_reader.h`, `objects/iab_pan_reader.cpp`, `objects/objects_pan_reader_selfcheck.cpp`
- Modify: `objects/CMakeLists.txt`, `mxf/iab_extract.cpp:215-245` (remove `SamePan`, `ReadPan`; call the moved ones)

**Interfaces:**
- Produces: `bool Objects::SamePan(PanEvent const &, PanEvent const &);` and `PanEvent Objects::ReadPan(IABObjectSubBlock const &iSubBlock, uint64_t iSamplePos);` with the bodies of today's functions.

- [ ] **Step 1: Write the failing selfcheck** `objects_pan_reader_selfcheck`: build an `IABObjectSubBlock`, set position `CartesianPosInUnitCube::setIABObjectPosition(0.25f, 0.5f, 1.0f)` (float setter), read it with `ReadPan(sub, 100)`. Asserts: `event.samplePos == 100`, `event.z == 1.0f` (Review Focus 2), `event.gain == 1.0f`; `SamePan(event, event)` is true; a copy with `gain = 0.0f` is not `SamePan`.
- [ ] **Step 2: Run:** build target `objects_pan_reader_selfcheck`. Expected: fails to link (functions missing).
- [ ] **Step 3: Move** `SamePan` and `ReadPan` to the new files, unchanged; `iab_extract.cpp` includes `iab_pan_reader.h` and calls `Objects::`.
- [ ] **Step 4: Run** `objects_pan_reader_selfcheck` and `iab_extract_selfcheck`. Expected: pass.
- [ ] **Step 5: Commit** `objects: move the IAB pan reader`.

### Task 4: ADM block reader (sampling to sub-blocks)

**Files:**
- Create: `objects/object_block_reader.h`, `objects/object_block_reader.cpp`, `objects/objects_block_reader_selfcheck.cpp`
- Modify: `adm/adm_iab_convert.h`, `adm/adm_iab_convert.cpp`, `objects/object_types.h` (add `IabSubBlock`), `objects/CMakeLists.txt`

**Interfaces:**
- Produces: `bool Objects::SampleObjectBlocks(std::string const &iObjectId, std::vector<ObjectBlock> const &iBlocks, uint32_t iSampleRate, uint32_t iFrameCount, std::vector<IabSubBlock> &oSubBlocks, std::string &oError);` with the body of `ConvertObject` (`adm/adm_iab_convert.cpp:33-94`), where `iObject.id` becomes `iObjectId` and `block.rtime/x/y/z/size/diffuse/inactive` come from `ObjectBlock`.
- Keeps: `bool CineIA::ADM::ConvertObject(AdmObject const &, uint32_t, uint32_t, std::vector<IabSubBlock> &, std::string &)` as an adapter: maps each `AdmObjectBlock` to `ObjectBlock` and calls `SampleObjectBlocks`. `CineIA::ADM::IabSubBlock` becomes `using Objects::IabSubBlock;`.

- [ ] **Step 1: Write the failing selfcheck** `objects_block_reader_selfcheck` with two blocks, `{rtime 0, x 1}` and `{rtime 0.0105, x -1}` (second starts at sample 504), 1 frame, 48000 Hz. Asserts: 8 sub-blocks; `x` of sub-blocks 0 and 1 is 65535 (`floor(16383.75 + 49151.25)`), of sub-block 2 on is 32767. Further asserts, each its own case: sample rate 44100 gives false (Review Focus 4); a block with `z = -0.1` gives false; one block with `rtime = 0.01` (first sub-block ends at sample 249) gives false (Review Focus 3).
- [ ] **Step 2: Run:** build `objects_block_reader_selfcheck`. Expected: fails to link.
- [ ] **Step 3: Move the code** as in Interfaces; the quantiser helpers move too.
- [ ] **Step 4: Run** `objects_block_reader_selfcheck` and the unmodified `adm_iab_convert_selfcheck`. Expected: pass.
- [ ] **Step 5: Commit** `objects: move the ADM block sampling`.

### Task 5: Writer profiles (standard IAB and Dolby)

**Files:**
- Create: `objects/pan_profile.h`, `objects/pan_profile.cpp`, `objects/objects_pan_profile_selfcheck.cpp`
- Modify: `mxf/iab_write.cpp` (pan checks of `ValidateInput`, today around lines 988-1050, and `ZoneSetAllowed`), `mxf/profile_iab.cpp` and `mxf/profile_dolby.cpp` (their per-event gain loops), `objects/CMakeLists.txt`

**Interfaces:**
- Produces: `struct Objects::PanProfile { char const *name; bool (*validateEvent)(ObjectTrack const &, PanEvent const &, std::string &); };` `PanProfile const &Objects::StandardIabPanProfile();` `PanProfile const &Objects::DolbyPanProfile();` `bool Objects::ValidateObjectTrack(PanProfile const &, ObjectTrack const &, uint32_t iFrameSamples, std::string &oError);`
- `ValidateObjectTrack` holds the checks common to both profiles that `ValidateInput` does for object events today (events present, first at a frame start, increasing order, value ranges, zone sets of RDD 57 table 4); `validateEvent` holds the gain rule. Standard: gain must be 1.0 (message of `profile_iab.cpp`, "RDD 57 O5 allows only unity object gain..."). Dolby: gain 0.0 or 1.0 (message of `profile_dolby.cpp`). Texts stay as today.
- Bed and channel checks (one bed, 128 channels) stay in `mxf/profile_*.cpp`.

- [ ] **Step 1: Write the failing selfcheck** `objects_pan_profile_selfcheck`: an `ObjectTrack` with one event at sample 0, gain 1.0, zone control off. `ValidateObjectTrack` true for both profiles; gain 0.0 false for standard with message containing `RDD 57 O5`, true for Dolby; gain 0.5 false for both (Review Focus 5); two events at the same sample false with a message containing `not in increasing order`.
- [ ] **Step 2: Run:** expected fails to link.
- [ ] **Step 3: Move the checks** as in Interfaces; `mxf/profile_*.cpp` `Validate` call `ValidateObjectTrack` per object, then do their bed checks; `ValidateInput` calls the standard checks through the profile passed in `ProfileRules` (add `Objects::PanProfile const &panProfile;` to `ProfileRules`).
- [ ] **Step 4: Run** `objects_pan_profile_selfcheck` and the unmodified `iab_write_selfcheck`. Expected: pass.
- [ ] **Step 5: Commit** `objects: move the pan validation into the IAB and Dolby profiles`.

### Task 6: IAB pan writer (event to sub-block)

**Files:**
- Create: `objects/iab_pan_writer.h`, `objects/iab_pan_writer.cpp`
- Modify: `objects/objects_pan_reader_selfcheck.cpp`, `mxf/iab_write.cpp` (the pan part of the frame loop, today lines 646-700), `objects/CMakeLists.txt`

**Interfaces:**
- Produces: `void Objects::FillObjectSubBlock(PanEvent const &iEvent, IABObjectSubBlock &oSubBlock);` setting position, gain, snap, zone gains, spread and decorrelation exactly as lines 646-700 do now.

- [ ] **Step 1: Extend** `objects_pan_reader_selfcheck`: for an event with `x 0.25, y 0.5, z 1.0, gain 1, snap false, zoneControl false, spreadMode 0, decorrelation 0` fill a fresh `IABObjectSubBlock`, read it back with `ReadPan`; assert `SamePan` with the original.
- [ ] **Step 2: Run:** expected fails to link.
- [ ] **Step 3: Move the code** from `iab_write.cpp`; the frame loop keeps IAFrame and element packing and calls `FillObjectSubBlock`.
- [ ] **Step 4: Run** `objects_pan_reader_selfcheck` and the unmodified `iab_write_selfcheck` (which also writes `reference/dolby_universe_trailer.mxf` when present). Expected: pass.
- [ ] **Step 5: Commit** `objects: move the IAB pan writer`.

### Task 7: Documents and pin

**Files:**
- Modify: `objects/docs/state.md`, `adm/docs/state.md`, `mxf/docs/state.md`, `README.md` (Cinema-MXF); `docs/continuity-notes.md` and the `third_party/Cinema-MXF` pin (New-Renderer)

- [ ] **Step 1:** `objects/docs/state.md`: replace "No code has been moved" and the inventory with what exists in `objects/` now (the five files' groups above), the profile rules, the build and the selfchecks. `adm/` and `mxf/` state documents: say which code now lives in `objects/`. Keep ADM and MXF text separate.
- [ ] **Step 2:** Merge the Cinema-MXF branch by PR; in New-Renderer, pin `third_party/Cinema-MXF` to the merge commit and add one line to `docs/continuity-notes.md`; PR and merge.
- [ ] **Step 3: Verify.** All ten selfchecks of Task 1 Step 1 plus the three `objects_*_selfcheck` pass.
- [ ] **Step 4: Commit** `docs: state of the objects part`.
