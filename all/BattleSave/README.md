# BattleSave

`igrik.BattleSave` creates the logical `BATTLE!` autosave for a local human
attacker. Saving occurs only when `advManager::DoCombat` confirms that combat
will begin. `advManager::MoveHero` only captures small in-memory snapshots and
never calls `SaveGame` or performs file I/O. A matching `advManager::DoEvent`
scope carries the final movement transaction into deferred object combats.

## Correctness

- AI, remote and recorded movement cannot arm a local movement transaction.
- During the confirmed battle save, adventure replay records created by the
  triggering step are temporarily excluded. Earlier opponent-turn history is
  retained, so Replay Opponent Turn cannot move the hero onto a garrison.
- Object combat is often deferred until after `MoveHero` returns. The matching
  returned map cell and event call keep the snapshot alive only for that event;
  unrelated or later scripted combats cannot consume stale movement state.
- With Fly or Water Walk, the plugin retains the most recent route position
  accepted by the game's own `hero::can_land` rule. A battle save therefore
  never places a flying hero on an obstacle crossed during the final steps.
- A small `BattleSave.Repair.v1` ERA section records any moved hero anchor.
  Saving no longer spoofs hero coordinates while the live map-cell stack still
  belongs to the combat tile.
- After that `BATTLE!` is loaded, engine Hide/Show behavior moves only the
  marked hero back to the valid anchor and rebuilds the map-cell object. For a
  boat landing, the captured boat is resolved by its validated pool identity;
  both visible-empty and hidden-occupied load representations are accepted and
  normalized to the occupied-boat topology. Unrelated heroes and boats are
  never inferred or modified.
- Load repair accepts both normal map-visible heroes and the selected hero's
  engine-managed mobile representation. The early repair is provisional and
  remains idempotent through `OnGameEnter`; its marker is consumed only after
  the complete hero/boat postcondition has been verified.
- Join, flee and WoG/ERM-cancelled paths do not reach `DoCombat`, so they do
  not replace the battle save.

This restores a boat-to-land battle to the occupied boat position, restores a
garrison attacker to the last landable route tile, and fixes the garrison
Replay Opponent Turn jump at its serialized replay-event source.

## Transactional file replacement

The resolved physical save is written to a same-directory `.new` file. The
existing `BATTLE!` is never opened or truncated during serialization. The
plugin requires success from the inner game serializer and the gzip close
operation (the outer game routine otherwise masks a short-write failure).
BattleSave then reads the staged gzip stream to its declared end and checks its
file identity before using `ReplaceFile` to install the new save while retaining
the previous file as `.BattleSave.old`. `MoveFileEx` is used when no previous
slot exists.

The validation pass occurs only for a confirmed battle save, never during
ordinary hero movement. An exception, out-of-memory failure, short write,
truncated gzip stream, re-entrant save, or failed replacement therefore leaves
the previous logical save untouched.

The logical name is passed in a writable 352-byte buffer, and the actual
runtime extension is discovered from the path produced by the game.

After a staged save is validated and committed successfully, BattleSave marks
the logical `BATTLE!` slot as the selected item for the next Save/Load dialog.
The native ERA + HD Mod filename lookup resolves the actual `.GMn`, `.CGM`, or
`.TGM` entry and positions the list. A failed save or failed replacement leaves
the previously selected item unchanged.

## Compatibility and build

The plugin validates the expected PE32 executable layout and all six hook
contracts before installation. The known `HD.WoG` `DoCombat` HiHook chain is
allowed and BattleSave inserts itself at the safe end of that chain.
Incompatible layouts fail silently.

Build `BattleSave.sln` as `Release|Win32` with the `v141_xp` toolset. The
Release configuration targets Windows XP SP3 (`WINVER` and `_WIN32_WINNT`
0x0501), uses `/MT`, and compiles the ERA SDK integration unit for ERA-managed
allocator support. The output is `BattleSave.dll`.
