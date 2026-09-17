# Toasty Window Analyzer v0.2.0

This is a Geode source project for Geometry Dash 2.2081.

## What this build contains

- TTRL file validation and a conservative binary reader.
- Internal frame/input timeline representation.
- Per-input window search from -10 to +10 frame offsets.
- Contiguous-survival window calculation.
- 1-10 frame histogram.
- Draggable top-left overlay with the requested colors.
- Saved overlay position.
- A gameplay-probe interface where the real GD checkpoint/replay integration belongs.
- A standalone TTRL inspection tool for reverse-engineering/verification.

## Important

This repository is intentionally source-only. A `.geode` binary is not included because the Geode SDK and a matching Geometry Dash build environment are required to compile and test it.

The hard part is the engine adapter: a TTRL parser alone cannot determine whether an offset survives. The analyzer therefore uses `IGameplayProbe`, so the replay/checkpoint code can be tested independently from the UI.

The included probe currently reports "not implemented" rather than pretending that a result is valid.

## Build

Set `GEODE_SDK` to your local SDK, then:

    cmake -B build -G Ninja
    cmake --build build

Use the matching Geode SDK for your installed Geometry Dash 2.2081 build.

## Intended workflow

1. Load a `.ttrl`.
2. Decode its input events into `InputTimeline`.
3. Attach the timeline to the active level.
4. For each input, save a gameplay checkpoint immediately before the input.
5. Replay with the input shifted earlier/later by one frame.
6. Stop when the shifted run no longer survives the probe target.
7. The contiguous valid range around the original input becomes that input's window.
8. Clamp the displayed window to 1-10 frames.
9. Add it to the histogram.

## Display

Requested format:

    INPUT WINDOWS

    10 : 10
     9 :  4
     8 :  3
     ...
     1 : 67

Colors:

- 10-7: blue
- 6-4: green
- 3-2: yellow
- 1: red
