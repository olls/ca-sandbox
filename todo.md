- debug print system
- clean up/formalise String functions
- temporary frame memory (for string building, etc...)

# Performance
= save trees to save building large trees every time
= thread simulation
- reduce tree building time
- profiling tools

# Functionality
= fix rule pattern parsing to allow cell state groups in all places handled by UI, and ensure the tree building also handles these cases.
- figure out explanation of "or" cell patterns
- rule pattern groups/sets/blocks
- better cell editing tools:
  - selections: move, copy, paste
  - sub-modules
- better parsing error messages (i.e: keep a line number counter?)
- colour cells state buttons in rule patterns UI where only one state is used