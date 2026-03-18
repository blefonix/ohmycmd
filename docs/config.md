# OMC Runtime Configuration

Configuration file path:

- `components/ohmycmd.cfg`

If the file does not exist, OMC creates it with defaults.

## Supported keys

```ini
CaseInsensitivity = true
LegacyOpctSupport = true
LocaleName = "C"
UseCaching = true
LogAmxErrors = true
```

### CaseInsensitivity

- `true` (default): command/alias matching is case-insensitive.
- `false`: matching becomes case-sensitive.

### LegacyOpctSupport

- `true` (default): if `OnPlayerCommandText` exists, OMC calls it before OMC pipeline.
  - If callback returns `1`, command is consumed immediately.
  - If callback returns `0`, OMC continues pipeline processing.
- `false`: skip `OnPlayerCommandText` compatibility call.

### LocaleName

- Locale used for case conversion when `CaseInsensitivity=true`.
- If locale cannot be loaded, OMC falls back to `"C"`.

### UseCaching

- `true` (default): cache resolved public indices for callbacks/handlers.
- `false`: resolve public index on every call.

### LogAmxErrors

- `true` (default): AMX errors from callback/handler invocations are printed.
- `false`: suppress AMX error printouts for those calls.

## Notes

- Runtime command behavior still uses explicit OMC registration and include macros.
- Config toggles affect matching/callback behavior only; they do not auto-register commands.
