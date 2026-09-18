# CBot

Cross-platform Geometry Dash macro foundation for Windows and Android.

This first version establishes the macro data model, command controller, compact overlay, and a frame counter. The actual input hooks, file format, and deterministic playback/analyzer are intentionally separated so they can be added without rebuilding the UI architecture.


## Current architecture
`State.hpp` exposes the shared CBot runtime state to UI code. `Main.cpp` owns the singleton instance and installs the controller callback, avoiding cross-translation-unit references to a private `State` class.
