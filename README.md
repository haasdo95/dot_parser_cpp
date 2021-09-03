## Intro

A C++ parser for [the DOT language](https://graphviz.org/doc/info/lang.html), leveraging [the lexy parser](https://github.com/foonathan/lexy).

Requires C++ 17. 

## Language Nonconformities

- statements listed in `graph`/`subgraph` must be separated by semicolons, newlines, or both
- each statement in `graph`/`subgraph` can take up **at most** one line
- subgraph in edge statements not fully supported; currently we only support node lists in edge statements. For example, the following can be successfully parsed
```
A -> {B, C} -> {D E} -> {F; G} -> {H}
```
- comments not supported (might do sth about this in the future)
- node compass not supported
- only one `attr_list` is allowed after each statement; for example, `vertex [color=b; size=5]` is okay, but `vertex [color=b][size=5]` is not (why would u want it anyway?)