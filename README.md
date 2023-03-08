# casperscript - ghostscript/postscript made friendlier for Python and C programmers

First by making '#!' equivalent to a comment ('%'), thus enabling
shebang-line execution, avoiding having to use `binfmt_misc` and a wrapper
script for the interpreter (because to process args, gs must be run as
`gs -- myfile.ps arg1 arg2`). (done).

This will also require splitting the args, since linux returns multiple
shebang-line args as a single string. (done).

I wanted to use '#' as a general alias to '%', but experimentations with this
have so far led to silent failures. (now works for standalone '# ' but not for
'#comment')

Next goal will be to switch the `name` character from forward slash to
backslash, but that is likely to be as problematic as '#' due to its
ubiquitous use through the interpreter. Also noting that '\' is already used as
an escape in strings, and '#' as a base delimiter in integers.
