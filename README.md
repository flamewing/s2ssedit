# Badges

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

[![CodeFactor Grade](https://img.shields.io/codefactor/grade/github/flamewing/s2ssedit?label=codefactor&logo=codefactor&logoColor=white)](https://www.codefactor.io/repository/github/flamewing/s2ssedit)
[![LGTM Alerts](https://img.shields.io/lgtm/alerts/github/flamewing/s2ssedit?logo=LGTM)](https://lgtm.com/projects/g/flamewing/s2ssedit/alerts/)
[![LGTM Grade](https://img.shields.io/lgtm/grade/cpp/github/flamewing/s2ssedit?logo=LGTM)](https://lgtm.com/projects/g/flamewing/s2ssedit/context:cpp)

[![CI Mac OS Catalina 10.15](https://img.shields.io/github/workflow/status/flamewing/s2ssedit/ci-macos?label=CI%20Mac%20OS%20X&logo=Apple&logoColor=white)](https://github.com/flamewing/s2ssedit/actions?query=workflow%3Aci-macos)
[![CI Ubuntu 20.04](https://img.shields.io/github/workflow/status/flamewing/s2ssedit/ci-linux?label=CI%20Ubuntu&logo=Ubuntu&logoColor=white)](https://github.com/flamewing/s2ssedit/actions?query=workflow%3Aci-linux)
[![CI Windows Server 2019](https://img.shields.io/github/workflow/status/flamewing/s2ssedit/ci-windows?label=CI%20Windows&logo=Windows&logoColor=white)](https://github.com/flamewing/s2ssedit/actions?query=workflow%3Aci-windows)

[![Coverity Scan Analysis](https://img.shields.io/github/workflow/status/flamewing/s2ssedit/coverity-scan?label=Coverity%20Scan%20Analysis&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=)](https://github.com/flamewing/s2ssedit/actions?query=workflow%3Acoverity-scan)
[![Coverity Scan](https://img.shields.io/coverity/scan/13714?label=Coverity%20Scan%20Results&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=)](https://scan.coverity.com/projects/flamewing-s2ssedit)

[![Windows snapshot build](https://img.shields.io/github/workflow/status/flamewing/s2ssedit/snapshots-windows?label=Windows%20Snapshot%20build&logo=windows&logoColor=white)](https://github.com/flamewing/s2ssedit/actions?query=workflow%3Asnapshots-windows)
[![Latest Windows snapshot](https://img.shields.io/github/release-date/flamewing/s2ssedit?label=Latest%20Windows%20snapshot&logo=windows&logoColor=white)](https://github.com/flamewing/s2ssedit/releases/latest)

## S2SSEDIT

Special Stage Editor for Sonic 2

## Create and install the package

You need a C++ development toolchain, including `cmake` at least 3.19, [Boost](https://www.boost.org/) at least 1.54, and GTKMM-3. You also need Git. With the dependencies installed, you run the following commands:

```bash
   cmake -S . -B build -G <generator>
   cmake --build build -j2
   cmake --install build
```

Here, `<generator>` is one appropriate for your platform/IDE. It can be any of the following:

- `MSYS Makefiles`
- `Ninja`
- `Unix Makefiles`
- `Visual Studio 16 2019`
- `Xcode`

Some IDEs support cmake by default, and you can just ask for the IDE to configure/build/install without needing to use the terminal.

## Commands

Page Up/Page Down/Mouse Wheel up or down: scrolls through the special stage.
Ctrl+Page Up/Page Down/Mouse Wheel up or down: as above, but faster scrolling.
Mouse wheel left or right: cycle through the editing modes.
Home: Go to start of special stage.
End: Go to end of special stage.

In Select mode:
    Left click: select object and/or drag it around.
    Right click: Cycle highlighted object (nothing->ring->bomb->nothing).
    Arrow keys: Move selected object inside its segment (cyclical motion).
    Ctrl+Left/Right arrows: As above, but faster motion.
    Delete: Deletes selected object.

In Ring insertion mode:
    Left click: Add new ring.
    Ctrl+Left click: Add new ring constrained in position.
    Right click: Delete highlighted object.

In Bomb insertion mode:
    Left click: Add new bomb.
    Ctrl+Left click: Add new bomb constrained in position.
    Right click: Delete highlighted object.

In Delete mode:
    Left click: Delete highlighted object.

## Step by step tutorial

In order to use this program, you must be using one of the split disassemblies
for Sonic 2. For best results, I recommend the SVN disassembly.

The first thing you need to do is open the special stage files; or rather, the
directory in which they reside. In the SVN disassembly, browse to the 'misc'
directory. You should see the folowing files on the left pane:

> Special stage level layouts (Nemesis compression).bin
> Special stage object location lists (Kosinski compression).bin

When they are there, you can click 'OK' or press enter.

You can now begin editing the special stages.

The top toolbar allows you to save your changes or revert to the last save. It
also has the edit mode palette (see above for commands).

The 'Special Stage browser' allows you to select which special stage you are
viewing/editing, as well as to add new special stages or move a special stage
around. You can also delete a special stage, but use this with care!

Each special stage is divided into segments. Segments are loaded one at a time
as you progress through the special stage. All objects in a given segment are
loaded at the same time, so avoid putting too many objects in a segment.
The 'Segment browser' toolbar allows you to move around the segments, add or
delete segments or move segments around.

Each segment has a list of objects, which terminates in one of four ways, and a
value describing how it twists and turns. The terminator of a segment is its
'type', and can be one of the following:

- Regular segment: nothing special.
- Rings message: Display message telling you how many rings you need.
- Checkpoint: Checks number of rings collected and may end special stage.
- Chaos emerald: Also checks number of rings collected.

Any given special stage will have at least three rings messages, two checkpoints
and one chaos emerald. They will also have at least 3 empty regular segments
at the start and at least 3 empty regular segments after the chaos emerald.

The twisting and turning of the special stage segments are controlled by the
segment's geometry and orientation. The geometry affects the entire segment, and
can be one of the following:

- Turn then rise
- Turn then drop
- Turn then straight
- Straight
- Straight then turn

The geometry determines the animation (sequence of "frames") of the segment.

The orientation, on the other hand, will usually affect only the next segment;
it depends on the geometry. The two exceptions are:

- 'Turn then straight' geometry: ignores its own orientation setting, and will *not* affect subsequent segments.
- 'Straight then turn' geometry: it is affected by its own orientation setting and will also affect subsequent segments.

The object properties allow moving or changing the type of the selected object.
There are other ways to to this, though.

At the top of the special stage display area, there is a ruler. This ruler marks
the angle (-128 to 127). Angle 0 is the normal position, -128 is upside down,
and + or - 64 are horizontal.
