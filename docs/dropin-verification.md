# Drop-in Verification Checklist

This checklist tracks the parity gate for Phase 6.

## Fixtures

- `tests/fixtures/dropin_gamemode.pwn`
- `tests/fixtures/dropin_filterscript.pwn`

## What is validated

- Macro-driven declaration flow (`cmd:`, `alias:`, `flags:`, `description:`)
- Callback pipeline hooks:
  - `OMC_OnInit`
  - `OnPlayerCommandReceived`
  - `OnPlayerCommandPerformed`
- Emulation path (`OMC_Execute`)
- Include behavior in both gamemode and filterscript mode

## Local verification command

```bash
tests/scripts/verify_dropin.sh
```

Expected output:

```text
drop-in fixtures compiled: OK
```

## Runtime sanity (optional but recommended)

1. Deploy latest `build/ohmycmd.so` to server components.
2. Restart server.
3. Check logs for lifecycle lines and absence of native-registration errors.
