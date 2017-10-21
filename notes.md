Cell Storage
============

- CellBlocks store NxN block of cells
- Universe is made up of a grid of CellBlocks
- All cells in a CellBlock are initialised
- CellBlocks are stored on the heap, and accessed via a hashmap on the block position.
  - i.e.: block_pos = cell.pos / block_size
- On simulation, each cell block is simulated as a whole
- Possible optimisation: CellBlocks store pointers to neighbours for quick access to border cell states.