# ohmycmd

`ohmycmd` is an upcoming command processor plugin for open.mp servers.

## Goals

- Work as an **open.mp component** (`components/ohmycmd.so`)
- Be built on top of the [open.mp SDK](https://github.com/openmultiplayer/open.mp-sdk)
- Provide a modern, reliable alternative for Pawn command handling

## Status

v0.x. Early development.

## Build (Linux)

```bash
git clone --recursive git@github.com:blefonix/ohmycmd.git
cd ohmycmd
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
```

Output:

- `build/ohmycmd.so`

## License

MIT © 2026-present Nazarii Korniienko

The repository itself is MIT licensed, but the project modules may have different licenses.
