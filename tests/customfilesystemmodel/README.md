Unit test for CustomFileSystemModel
-----------------------------------

These tests only concern the actual contents of the model, not the
`emit dataChanged()` functionality.  Test scenarios are performed against the
`dirs` subdirectory, and are written in files named `scenario-??.txt` where
each file contains:

- `#` comments and blank lines: ignored by the test program.
- `##` "double comments" are used for the "title" of each test, and extracted
  to form the `list-scenarios.txt` file with `extract-scenarios.sh`.
- `actions:` either `+` to select a path, or `-` to unselect a path.  Actions
  are processed in order.
- `results:` after all the actions are completed, should each path be `0`
  unchecked, `1` partially checked, or `2` checked?

For now, the number of tests must be defined in `scenario-num.h`.

