GMIPS is a MIPS assembler and emulator.


INSTALLATION

Run `make` in the root directory of the repository to build the software. The
binaries will be compiled and placed in `bin/`. `bin/assemble` takes a mips
assembly file as an argument, and writes a MIPS binary to `stdout`. `bin/emulate`
takes a MIPS binary file as an argument, and runs it in a virtual machine,
printing the output of the VM to `stdout`, and other VM information (errors,
ending state) to `stderr`. `bin/run` takes a mips assembly file as an argument,
assembles it, and runs it in a VM using the previous two binaries.
