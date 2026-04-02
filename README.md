early stages, experimental compiler for ARM-aarch64

I'm mqking the language up as I go along; I am also deliberately working *very slowly*, making sure each stage works well and has tests implemented &c before moving on to the next.

I'll be adding new syntax incrementally.

Current state: lexing & parsing of programs consisting of a single unsigned integer works; compiling is very hacky and just produces a program that exits with an exit code set to the provided number

also, main function still needs a ton of work.

I'm releasing all the code into the public domain; this was done for educational and (more realistically) entertainment purposes (ie to ward off boredom when I didn't have easy access to my laptop). so I really couldn't carw what other people do with the code.
