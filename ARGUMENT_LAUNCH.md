# Starting C64 media from a launcher

This fork completes the startup argument hook in
[GimliDS](https://github.com/wavemotion-dave/GimliDS) by wavemotion-dave and its
credited contributors. It reuses upstream's loaders, per-game configuration,
drive preferences and keyboard-command queue.

```text
argv[0] = sd:/apps/GimliDS.nds
argv[1] = sd:/roms/Commodore 64/My Game.d64
```

| Extension | Startup action |
| --- | --- |
| `.prg` | Reset, then use the existing delayed PRG loader/run path |
| `.crt` | Reset, then use the existing delayed cartridge insertion path |
| `.d64` | Mount drive 8 and queue the disk menu's `LOAD"*",8,1` / `RUN` command |

Extensions are matched without case sensitivity. Pass a path containing spaces in
one argv entry. Startup waits 60 emulated keyboard/frame polls before applying the
selection so the C64 has time to initialize. A disk's first program is attempted;
this cannot guarantee that every disk uses that boot convention. Media is selected
once, with normal browsing still available afterward.

The argument is copied at entry. An explicit `sd:/` or `fat:/` selects that mounted
card immediately after filesystem initialization, before normal resource lookups.
Relative paths are resolved against the mount-time directory, before the browser
moves to `/roms/c64`. The chosen media's directory becomes the working directory,
as in the manual mount workflow. Parent paths and basenames must each fit within
256 bytes, including the terminator. Missing, empty, unsupported or overlong input
leaves normal startup in place. Only `argv[1]` is used.

Upstream's C64 BIOS files are still required in their documented locations; no BIOS
has been added to this fork. This startup path does not add new media formats.

## Source changes

`arm9/source/argv_launch.cpp` and its two headers hold the adapter. Existing code
has small hooks in `main.cpp` and `Display.cpp`. The old `ArgvReceived` placeholder
is replaced without changing the manual media menus or their loaders. The original
Makefiles are unchanged. This branch started from upstream commit
`e99a7e8b2ad65b7dcb93dd5d9feb217eaccc5740`.

## Building

### Optional current BlocksDS build

The original devkitARM/libnds Makefiles remain available with their original SDK
requirements. To build using current BlocksDS instead, run the separate adapter:

```sh
python3 tools/build_blocksds.py --clean
```

Requirements are Python 3.9 or later, BlocksDS 1.25 (including libnds, Maxmod,
DSWiFi and its asset tools), and the Wonderful ARM GCC toolchain. Local builds used
Wonderful GCC 16.2. Defaults are `/opt/blocksds/core` and
`/opt/wonderful/toolchain/gcc-arm-none-eabi/bin`; override them using `--blocksds`
and `--toolchain`. `BLOCKSDS` also supplies the SDK default. Use `--jobs` to control
parallel compilation.

The adapter reads upstream's source/include/asset/library lists and writes all
objects, generated graphics, linker maps and the NDS file under `build/blocksds/`.
It rebuilds all selected sources each run. A forced compatibility header isolates
legacy formatted-output and heap-helper names from the new SDK. The application's
normal Makefiles and SDK headers are not patched. This is a build option, not an
upstream SDK migration or proof of hardware timing/audio compatibility.

A complete BlocksDS build and the sanitized host tests passed locally. Actual DS/DSi
argv launches and title compatibility still require console/emulator validation.

## Keeping up with upstream

The `argv-launch` branch contains the fork additions; `main` is kept as an upstream
baseline. Startup adapters and host tests are separate files, and the original
loading routines remain responsible for file decoding and emulation. The small
hooks in existing source are the expected merge points.

```sh
git remote add upstream https://github.com/wavemotion-dave/GimliDS.git  # Once, if not already configured.
git fetch upstream
git switch argv-launch
git merge upstream/main
sh tests/run-argv-tests.sh
```

Rebuild after merging. If upstream adds native argument support, retire the adapter
rather than layering a second startup loader over it.

## Validation and licensing

Run `sh tests/run-argv-tests.sh` with a C/C++ compiler supporting AddressSanitizer and
UndefinedBehaviorSanitizer. These tests exercise the actual adapter against small
host API doubles and synthetic files: path bounds, spaces, extension matching,
missing files and one-shot selection. They do not run a DS CPU or prove that a
game/book works on a console.

The source and assets inherited from upstream retain their original authors,
credits and terms. New integration files are MIT-licensed as described in
[ARGUMENT_LAUNCH_LICENSE.md](ARGUMENT_LAUNCH_LICENSE.md); this does not relicense the
application or its dependencies. No game, book or separately supplied BIOS is
added by this fork. No binary release has been published for this change.
