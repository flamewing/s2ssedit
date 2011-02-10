Commands
========
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


Step by step tutorial
=====================

In order to use this program, you must be using one of the split disassemblies
for Sonic 2. For best results, I recommend the SVN disassembly.

The first thing you need to do is open the special stage files; or rather, the
directory in which they reside. In the SVN disassembly, browse to the 'misc'
directory. You should see the folowing files on the left pane:

	Special stage level layouts (Nemesis compression).bin
	Special stage object location lists (Kosinski compression).bin

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

	Regular segment: nothing special.
	Rings message: Display message telling you how many rings you need.
	Checkpoint: Checks number of rings collected and may end special stage.
	Chaos emerald: Also checks number of rings collected.
	
Any given special stage will have at least three rings messages, two checkpoints
and one chaos emerald. They will also have at least 3 empty regular segments
at the start and at least 3 empty regular segments after the chaos emerald.

The twisting and turning of the special stage segments are controlled by the
segment's geometry and orientation. The geometry affects the entire segment, and
can be one of the following:

	Turn then rise
	Turn then drop
	Turn then straight
	Straight
	Straight then turn

The geometry determines the animation (sequence of "frames") of the segment.

The orientation, on the other hand, will usually affect only the next segment;
it depends on the geometry. The two exceptions are:

	'Turn then straight' geometry: ignores its own orientation setting, and will
	*not* affect subsequent segments.

	'Straight then turn' geometry: it is affected by its own orientation setting
	and will also affect subsequent segments.

The object properties allow moving or changing the type of the selected object.
There are other ways to to this, though.

At the top of the special stage display area, there is a ruler. This ruler marks
the angle (-128 to 127). Angle 0 is the normal position, -128 is upside down,
and + or - 64 are horizontal.
