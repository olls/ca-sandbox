- debug print system
- clean up/formalise String functions
- temporary frame memory (for string building, etc...)

# Performance
= save trees to save building large trees every time
= thread simulation
- reduce simulation time
- reduce tree building time
- profiling tools

# Functionality
= better cell editing tools:
  - selections: move, copy, paste
  - sub-modules
- cell initialisation options ui
- ability to specify associated .rule file in .cells file
- composite count matching comparison: (3 <= x <= 4) or (>= 3, <= 4)
- figure out explanation of "or" cell patterns ("Match if one OR pattern cell matches")
- rule pattern groups/sets/blocks
- better parsing error messages (i.e: keep a line number counter?)
- colour cells state buttons in rule patterns UI where only one state is used
- file pickers should be modals
- reduce risk of corrupting cells files (i.e: it's too easy to overwrite, after opening with rule using different states)
- resize universe hashmap, if saturated
- procedure for resizing a cells file's cell_block_dim
