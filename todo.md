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
- resize universe hashmap, if saturated

# composite states?
- eg: Modifier: Transmission
      Modifier: Ordinary
      Modifier: Special
      Modifier: Excited
      Modifier: Quiescent

      State: Ordinary & Transmission & Quiescent
      State: Ordinary & Transmission & Excited
      State: Special & Transmission & Quiescent
      State: Special & Transmission & Excited

- and templated patterns?
  - eg: Pattern:
        Var: a = Transmission & Excited

    Var must have the same value at every use in the pattern for the pattern to match.

# Functionality
= better cell editing tools:
  - selections: move, rotate, mirror
  - sub-modules
  - cut regions
- fix re-blockifying -ve cell blocks
- keyboard shortcuts for selecting states (0-9)
- auto centre/scale universe
- render border
- draw grid (or origin axes)
- cell initialisation options ui
- ability to specify associated .rule file in .cells file
- composite count matching comparison: (3 <= x <= 4) or (>= 3, <= 4)
- figure out explanation of "or" cell patterns ("Match if one OR pattern cell matches")
- rule pattern groups/sets/blocks
- better parsing error messages (i.e: keep a line number counter?)
- colour cells state buttons in rule patterns UI where only one state is used
- reduce risk of corrupting cells files (i.e: it's too easy to overwrite, after opening with rule using different states)
- separate model matrix and projection matrix uniform (ie: aspect ratio goes in projection, view_panning goes in model)
- tools for creation of new rule files
- renaming files
- rotationally invariant patterns
- delete states
- unfiddlify named states given value by position in .rule file (to enable state deletion)
    (would require reloading the cells file to update the state values stored in memory)
- undo history
- loading a different cells file after creating a new cells file is broken