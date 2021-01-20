mkg3a 0.5.0
Copyright 2011-2020 Peter Marheine <peter@taricorp.net>

mkg3a is a tool to pack raw binaries into Casio Prizm .g3a (add-in) files.

## Usage

Run mkg3a -h for help.

To build useful programs, you might do something like the following (requires
a GNU toolchain configured for sh4nofpu as well as crt0 and the g3a.lkr linker
script):

   sh4nofpu-elf-gcc -c -o crt0.o crt0.s
   sh4nofpu-elf-gcc -c -o myprogram.o myprogram.c
   sh4nofpu-elf-ld -T g3a.lkr -o myprogram.bin myprogram.o crt0.o
   mkg3a myprogram.bin

When setting names in the output file, any unspecified entries will be derived
from 'basic', which itself is derived from the output filename if not provided.
Thus, to set a friendly name for your program in every language with ease, you
may do something like the following.

   mkg3a -n "basic:My Awesome Program" myprogram.bin

Note that the English text may not render correctly in some languages
(particularly Chinese).  The 'internal' name entry will also be derived
from 'basic', but truncated and modified as necessary.  In the above example,
it would become something like '@MY AWESOME '.

### g3a-updateicon

The provided g3a-updateicon tool can be used to change the icons embedded in an
existing g3a file. This can be particularly useful if you have a newer
calculator (with different UI style) and an add-on with icon style designed for
the older calculators.

Simply pass the path to the g3a file and both icons to the tool:

    g3a-updateicon Utilities.g3a selected.png unselected.png

The g3a file will be modified in place, so make a backup first if needed.

## Compiling

Configure mkg3a with cmake.  Use cmake -i or your favorite cmake UI (such as
ccmake) to customize the configuration.

Build and install to the default location:

    mkdir build
    cd build
    cmake ..
    make
    make install

