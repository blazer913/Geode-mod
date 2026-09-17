# Engine adapter notes

`IGameplayProbe` is deliberately isolated because this is the part that depends on the exact Geode/GD 2.2081 checkpoint APIs.

The adapter must provide:

- `beginRun(level, timeline)`
- `saveCheckpoint(frame)`
- `restoreCheckpoint()`
- `runUntil(targetFrame)`
- `survived()`

For a candidate offset:

    restore checkpoint
    modify only the selected input timing
    replay forward
    inspect the real PlayLayer/player state
    return whether the run survived

A press/release pair should move together when shifting a held input. The adapter should also preserve the input's action type and player stream.

Do not classify a candidate from a single frame of player position. A candidate is valid only if the run reaches the same survival boundary used by the analysis policy.
