# Building a release

The argv branch retains upstream's devkitARM Makefiles. The optional
`tools/build_blocksds.py` adapter builds with BlocksDS without changing those
Makefiles. The first published argv build uses BlocksDS 1.25.0 and Wonderful
GCC 16.2.0. These are development builds; physical-console validation remains
outstanding.

```sh
sh tests/run-argv-tests.sh
python3 tools/build_blocksds.py --clean --jobs 4
# Commit source changes before preparing an identified release.
python3 tools/build_release.py --tag v1.7a-argv.1 --skip-build
```

The release builder checks a clean source tree and SDK version, then downloads
checksum-pinned SDK, libnds and Maxmod source archives from
`tools/release-dependencies.json`. Cached downloads live under ignored
`build/release-cache/`; the source package includes them under `dependencies/`.
Copy them back into that cache before rebuilding offline. Install the matching
SDK/libraries and Wonderful compiler; the normal BlocksDS build adapter uses the
installed SDK. Pins identify the SDK-selected library revisions, rather than
current upstream branches.

Each version under ignored `dist/` contains a standalone `GimliDS.nds`, runtime
ZIP, corresponding source tarball, build identity and checksums. The runtime ZIP
and source tarball retain original upstream credits, GPL-2.0 text, adapter MIT
terms and runtime notices. Source contains the application, build adapters and
pinned dependency archives; prebuilt upstream binaries and blank disks are omitted.
No Commodore firmware, games or personal configuration is included.

`tools/notices/gcc-COPYING.RUNTIME` is the GCC Runtime Library Exception 3.1.
`tools/notices/picolibc-COPYING` retains the Wonderful Picolibc notice collection
from revision `770d868896622d726129fa99168d5d20160d0f55`:
https://codeberg.org/WonderfulToolchain/wf-picolibc/src/commit/770d868896622d726129fa99168d5d20160d0f55/COPYING.picolibc.
Other runtime notices are collected from the exact pinned source archives.
Retain these notices and distribute matching source alongside executable downloads.
See README.md for required user-supplied BIOS names and checksums, and
ARGUMENT_LAUNCH.md for argv behavior and compatibility limits.
