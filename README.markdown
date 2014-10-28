RUBE
====

Distribution version 1.6.

©1997-2014 Chris Pressey, Cat's Eye Technologies.  All rights reserved.
Distributed under a BSD-style license; see the file LICENSE.

What is RUBE?
-------------

_RUBE_ is a twisted programming language in tribute to Rube Goldberg,
creator of all those unique cartoon contrivances, who showed the world that
anything can be done in a more complicated fashion than necessary.  The RUBE
programming language has some similarities to said contrivances.

Anyone who's played The Incredible Machine (and to some extent, Donkey Kong)
will understand RUBE almost immediately.  "Instructions" in the program
interact to execute the algorithm required.  I made the choice to use a
"warehouse" paradigm for these interacting instructions (because this is what
happens to your brain when you work at a lumberyard.  Which, I submit, is
far better than what happens to your brain when you go to grad school.)

The RUBE language can be described as a two-dimensional "bully automaton":
like a cellular automaton, but certain state transitions force other,
non-adjacent cells to assume certain states.  Although this may sound like an
interesting notion, the number of interactions climbs quickly as more objects
do things, and things get much messier than they do in a regular ol'
cellular automaton like John Conway's Game of Life.  Also unlike a regular
cellular automaton, RUBE supports rudimentary input and output functionality.

Examples
--------

Because implementing an algorithm in RUBE is a clumsy job
of co-ordinating concurrently operating sections of the playfield, few
non-trivial RUBE programs exist.  In particular, it has never to the
author's knowledge been shown that RUBE is Turing-complete.
On the other hand, it has never been shown that it's not.

Sadly, the ubiquitous "Hello, world!" in RUBE is a very unfriendly

    4666622767662
    85ccfc07f2c41
    OOOOOOOOOOOOO
    ccccccccccccc
    =============

Or an equally ugly

    (21646c726f77202c6f6c6c6548
    ===========================

                               O F
                               c
                               =

Language Description
--------------------

A RUBE program is a two-dimensional grid of ASCII characters, or at least
represented by characters drawn from the ASCII set.

Integral are the crates, `0` to `f`.  These represent data.  They also
represent movable boxes which are subject to gravity, being pushed
around, and conveyor belts, among other things.

Girders `=` are stationary parts of the program on which crates can
be piled.  When there's no girder directly below a crate, that crate
falls.

Dozers `(` and `)` push crates around.  Dozers are subject to gravity,
and must travel on girders.  Dozers turn around when there is a `,` in
front and above them.  As an interesting side effect they also
replicate themselves at `,`'s.

Ramps `/` and `\\` are used for dozers to move from girder to girder.

Conveyor belts `>` and `<` carry crates along with no help needed from
dozers.  Note that conveyor belots do not affect dozer motion.

Winches up `W` and down `M` move crates from girder to girder.

"Swinches" move crates up `V` or down `A` and then switch states.

Gates `K` compare crates to a reference crate and move the crate
depending on if it's less than, or more than/equal to the
reference crate.

     t   | t = target crate     l = lesser crates
     K   | r = reference crate  g = greater than or equal to
    lrg  |

Packers `+` and Unpackers `-` perform 4-bit arithmetic on crates:

     +      +     -      -
    12  ->   3   ef  ->   1
    ===    ===   ===    ===

Furnaces `F` destroy everything one space left, right, below, and
above them.

Replicators `:` make a copy below them of whatever is above them,
usually a crate or a dozer.  Special replicators `;` only make
copies of crates.  Upside-down special replicators `.` work like `;`
but make a copy above them of what's below them.

Dozers turn around when they hit certain instructions.
Crumble walls `*` disappear after being hit horizontally by a dozer.

Program output provides a print mechanism.  If there is a `c` crate
below the `O`, the value specified by the two crates above the `O`
(if they exist) is output as an ASCII character.  If there is a `b`
crate below the `O`, they are output as the decimal representation,
in ASCII, of the byte (numerical) value of the crates.

The following outputs a space:

    2
    0
    O
    c
    =

Table of Instructions
---------------------

    Symbol  Name                    Bump R  Bump L  Surface Gravity

    Implemented:
            space                   pass    pass    no      no
    0..f    crate                   move r  move l  yes     yes
    (       dozer right             move r  move l  yes     yes
    )       dozer left              move r  move l  yes     yes
    =       girder                  rev     rev     yes     no
    F       furnace                 die     die     die     no
    \       ramp                    +1 elev +1 elev yes     no
    /       ramp                    +1 elev +1 elev yes     no
    >       conveyor belt right     rev     rev     yes     no
    <       conveyor belt left      rev     rev     yes     no
    :       replicator              rev     rev     yes     no
    ;       special replicator      rev     rev     yes     no
    .       upside down replicator  rev     rev     yes     no
    ,       turn signal                             no      no
    *       crumble-wall            break   break   yes     no
    O       output                  rev     rev     yes     no
            b       byte<-crate crate
            c       char<-crate crate
    K       gate                    rev     rev     yes     no
    W       winch up                rev     rev     yes     no
    M       winch down              rev     rev     yes     no
    V       swinch up               rev     rev     yes     no
    A       swinch down             rev     rev     yes     no
    +       packer                  rev     rev     yes     no
    -       unpacker                rev     rev     yes     no
    C       crate killer            die cr  die cr  die cr  no
    D       dozer killer            die doz die doz die doz no


    Planned:
    I       input                   input   input   yes     no
            b       byte->crate crate
            c       char->crate crate
    T       scale                   rev     rev     yes     no
    |       door                    rev     rev     yes     no


    Kinda Silly (extraneous to real programming):
    km      monkey                  rev     rev     yes     yes
    wl      weasel                  rev     rev     yes     yes
    H       mulberry bush           rev     rev     yes     yes

    Q       supergoo                rev     rev     yes     kinda

    ~       water                   wet     wet     wet     kinda
    u       pontoon                 rev     rev     yes     yes (floats)
    sz      electric eel            die     die     die     yes

Implementation
--------------

`rube.c` is the reference implementation of the RUBE programming language.
In fact, it is far more "reference" than I care for -- if, in this
implementation, a dozer (say) falls on another dozer while it is turning
around and pushing a crate, and that combination results in only one dozer
remaining there (or perhaps creates a new dozer out of nowhere for a total
of three dozers), well, hey, that's what the language defines for that
configuration.

The reference implementation loads the grid of instructions from an
ASCII text file.  Coincidentally (because I ripped off the Befunge-93
routines to write it) it's currently restricted to an 80x25 source file.
However, that should not be taken as an defining limitation; the language
itself imposes no bounds on the size of a RUBE playfield.
