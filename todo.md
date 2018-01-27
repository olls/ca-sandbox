- debug print system
- clean up/formalise String functions
- temporary frame memory (for string building, etc...)

# Performance
= save trees to save building large trees every time
= thread simulation
- reduce simulation time
  - rewrite create-new loop to only loop through the cells it actually needs to check
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
- auto centre/scale universe
- colour cells state buttons in rule patterns UI where only one state is used
- reduce risk of corrupting cells files (i.e: it's too easy to overwrite, after opening with rule using different states)
- resize universe hashmap, if saturated
- remove null cell blocks button
- modifier key for making selection instead of button
- store cell boundaries of original selection in regions
- separate model matrix and projection matrix uniform (ie: aspect ratio goes in projection, view_panning goes in model)
- render border