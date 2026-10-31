# Objects: design (2026-10-29)

Status: proposed, waiting for approval. No code is moved or written before it is approved.

## Intent

Object pans, movements and automation are today split between `adm/` (read ADM blocks, convert to IAB
sub-blocks) and `mxf/` (read and write IAB pan sub-blocks). The user wants all of it in `objects/`, so
that `adm/` and `mxf/` stay about their own subject (ADM BWF files, MXF track files) and share one
implementation of the object movement logic.

## Goals

1. One neutral object type: an object is a list of blocks in time; a block has start time, duration,
   position (unit cube), gain, size and spread, diffuse or decorrelation, importance, snap, zone gains,
   jump or interpolation length, and an inactive flag.
2. A reader in `objects/` with two inputs: ADM object blocks (`AdmObject`, from `adm_bwf_reader`) and IAB
   pan events (`PanEvent`, from `iab_extract`). Both give the neutral type.
3. A writer in `objects/` that turns the neutral type into IAB pan sub-blocks (position, spread, gain,
   snap, zone gains, decorrelation) per sub-block of 250 samples at 48 kHz, with two profiles: standard
   IAB (SMPTE ST 2098-2) and Dolby (what Dolby's cinema writer does, as in
   `mxf/docs/dolby-cinema-mxf-writer.md`).
4. `adm/` and `mxf/` depend on `objects/`, never on each other.

## Non-goals and anti-goals

1. `objects/` does not read or write ADM files, MXF files, IAFrames, beds or DLC audio. It produces and
   consumes pan values only; `mxf/` packs them.
2. No change of behaviour. The existing selfchecks must give the same results after the move.
3. `adm/` and `mxf/` must not end up including each other's headers through `objects/`.

## Constraints

1. The code builds only inside New-Renderer (IABLib and asdcplib are siblings through
   `NEW_RENDERER_THIRD_PARTY`). IABLib cannot be built in the session that wrote this document, so the
   user compiles and runs the selfchecks.
2. The ADM `interpolationLength` is not used today; the move keeps that.
3. A block with Z below 0 and an ADM sample rate other than 48000 are errors today; the move keeps them.

## Approaches

A. `objects/` as a library: neutral type, reader with both inputs, writer with two profiles. Chosen by
the user. It matches the request, and the dependency runs one way. Cost: touches `adm_iab_convert` and
parts of `iab_write` and `iab_extract`.
B. Neutral type and writer only; readers stay in `adm/` and `mxf/`. Less code moves, but the reader is
not in the new part. Not chosen.
C. Neutral type and profiles only. Not chosen.

## Decided

1. The writer produces IAB pan sub-block values per profile. `mxf/` keeps packing them into IAFrames.
2. Standard profile = IAB. Dolby profile = Dolby rules: gain only 0.0 or 1.0 (unity when active, silence
   otherwise), RDD 29 reserved fields read by IAB as ObjectGainPrefix 0 and ObjectSnapTolExists 0,
   unit-cube z coded over 65535.

## Left to the builder

1. File and CMake layout of `objects/` (suggested: `objects/` with its own CMake project, linked by `adm/`
   and `mxf/`).
2. Exact names of the neutral type and the reader and writer functions.
3. The order of the move, one existing function at a time with its selfcheck: `ConvertObject` first
   (`adm_iab_convert_selfcheck`), then the `mxf/` pan code (`iab_extract_selfcheck`,
   `iab_write_selfcheck`).
4. Where the Dolby rules in `mxf/profile_dolby.cpp` that concern pans (gain) move; the rules about
   channel count stay in `mxf/`.
