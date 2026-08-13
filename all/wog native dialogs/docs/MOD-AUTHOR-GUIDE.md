# WoG option search and locks

WoG supplies the search strings and the base lock object in `Mods/WoG/Lang/wnd.json`. A mod that requires an option value should merge only its `wnd.dlg_wog_options.locks` entries through a JSON file in that mod's root `Lang` directory. Keep behavioral values in root `Lang`; put only translated display text such as `reason` in `Lang/<language>`.

## Lock configuration

Lock keys are WoG/ERM option IDs. The presence of a valid entry means that the option is locked to its `value` and shown disabled.

```json
{
    "wnd": {
        "dlg_wog_options": {
            "locks": {
                "730": {
                    "value": false,
                    "reason": "Single-slot composite artifacts conflict with this mod."
                },
                "731": {
                    "value": true,
                    "reason": "This option is required by this mod."
                },
                "101": {
                    "value": 2,
                    "reason": "The third radio choice is required."
                }
            }
        }
    }
}
```

Checkbox values may be JSON booleans or their numeric equivalents: `false`/`0` locks the option Off and `true`/`1` locks it On. Radio values are zero-based choice indexes. Legacy option IDs 1-4 use their stored logical boolean value; the host alone converts that value to WoG's inverted visual checkmark.

`reason` is optional. Translate it by repeating only `wnd.dlg_wog_options.locks.<id>.reason` in the active locale directory. If absent, the menu uses `wnd.dlg_wog_options.search.locked_hint`.

## Preset behavior

Locked values are enforced in the active setup but preserved transparently in WoG `.dat` presets. Saving over an existing preset retains that preset's raw value for each locked option while writing every unlocked change normally. A new preset uses the pre-lock value captured from startup or the last successfully loaded complete preset. Loading remembers the preset's raw values without applying them while the lock remains active.

If no existing preset value or captured pre-lock value is available, saving is canceled before the file is changed. This behavior is automatic and requires no additional JSON settings.

## Merge behavior

ERA merges Lang JSON from all active mods. Distinct option IDs combine. When multiple files define the same final `value` or `reason`, the later/higher-priority value wins, and the active `Lang/<language>` reason overrides the root reason.

A later mod can replace a lower-priority lock with another valid value. This deliberately simple presence-based schema has no separate unlock entry: removing a lock requires removing it from the mod that introduced it. Do not define behavioral `value` fields in locale-only files.

Malformed values, unknown IDs, and locks for options absent from the current setup are ignored. A radio lock with an out-of-range value is also ignored rather than guessing a choice.

## Scope

This interface controls the WoG setup state: it applies when setup data is initialized and after menu Load, Restore, and preset operations, and prevents the menu from changing the locked option. It is not a permanent ERM hard lock. A later ERM script or plugin can still change a native option; a mod that requires an invariant throughout gameplay must enforce it in its own ERM/plugin logic too.

## External checkbox providers

Native plugins may append a numbered checkbox that belongs to another settings system without adding anything to WoG preset data. Use the packed `WogExternalOptionsApiV2` C ABI in `src/WoGExternalOptionApi.h`, published through the Patcher variable `WoG.NativeDialogs.ExternalOptions.Api.2`. Validate its magic, ABI version, and structure size before calling `RegisterCheckbox`.

Every provider must reserve one unique virtual option ID in the range 1000-9999. IDs 0-999 belong exclusively to WoG's native option array, duplicate virtual IDs are rejected, and unnumbered external rows are not supported.

The provider owns its context, callback code, and returned text for the lifetime of the process. Text must use ERA's active code page. `BeginMenuSession` refreshes provider state and returning zero hides the row for that menu session. A successful begin is paired with `EndMenuSession`. `QueryState` reports visible, enabled, and checked flags; visibility is fixed until the next menu opening, while enabled and checked are refreshed live. Name, hint, popup, and source badge text are required for visible rows; search aliases are optional.

Registration is process-lifetime and keyed by a printable stable key. Result `1` means registered, `2` means the identical descriptor was already registered, and negative results mean invalid (`-1`), conflicting duplicate (`-2`), or registry full (`-3`). Register after the host variable becomes available and before the WoG Options menu opens. A provider loaded earlier may retry registration from a later ERA lifecycle event.

Direct clicks call `SetChecked`; Select All and Clear All use the corresponding change-cause values. Restore Defaults calls `ApplyDefault`. Load, Save, and Multiplayer do not serialize or mutate external values, although state is queried again after the native menu action. External rows are appended after all runtime TXT/ERS rows in their requested zero-based page/group and participate in search, status text, right-click descriptions, page jumps, and highlighting.

A provider checkbox can be locked through the same merged JSON object. For example, `wnd.dlg_wog_options.locks.1000.value: false` locks it Off and `value: true` locks it On. The host calls `SetEnforcedState(context, 1, 0|1)` after plugin loading and whenever lock data is refreshed. The callback changes only the provider's effective runtime policy and returns nonzero only after verifying it. When the entry is absent, the host calls `SetEnforcedState(context, 0, 0)` to clear the lock. It never invokes this callback synchronously from `RegisterCheckbox`, which may run under the Windows loader lock.

External locks never participate in WoG's 1000-value array, shadow values, or 4000-byte `.dat` serialization. While enforcement is verified, the row displays the locked value and suppresses ordinary click, Select/Clear All, and Defaults mutations. If enforcement cannot be verified, the row is shown unavailable rather than falsely claiming an active lock.
