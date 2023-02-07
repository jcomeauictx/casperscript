# casperscript - ghostscript/postscript made friendlier for Python and C programmers

First by changing the comment character from '%' to '#', which will enable
shebang-line execution, avoiding having to use `binfmt_misc` and a wrapper
script for the interpreter (because to process args, gs must be run as
`gs -- myfile.ps arg1 arg2`).

Next goal will be to switch the `name` character from forward slash to
backslash. That may be more difficult, since gs already uses backslash as an
escape in strings, but to my knowledge it doesn't use '#' for anything yet.
