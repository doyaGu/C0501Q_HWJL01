include_directories(
    .
)

add_definitions(
    -DHAVE_CONFIG_H
)

add_library(tre STATIC
    xmalloc.c
    tre-mem.c
    tre-stack.c
    tre-ast.c
    tre-parse.c
    tre-compile.c
    tre-match-parallel.c
    tre-match-backtrack.c
    tre-match-approx.c
    regcomp.c
    regexec.c
    regerror.c
)