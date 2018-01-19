- debug print system
- clean up/formalise String functions
- temporary frame memory (for string building, etc...)

# Performance
= save trees to save building large trees every time
= thread simulation
- reduce tree building time
- profiling tools
- tom fosyth's initial ExtendableArray space

# Functionality
= better cell editing tools:
  - selections: move, copy, paste
  - sub-modules
- >= and <= in count matching
- composite count matching comparison: (3 <= x <= 4) or (>= 3, <= 4)
- figure out explanation of "or" cell patterns ("Match if one OR pattern cell matches")
- rule pattern groups/sets/blocks
- better parsing error messages (i.e: keep a line number counter?)
- colour cells state buttons in rule patterns UI where only one state is used
- file pickers should be modals