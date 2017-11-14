#ifndef LOAD_UNIVERSE_H_DEF
#define LOAD_UNIVERSE_H_DEF

#include "types.h"
#include "cell-storage.h"
#include "simulate.h"


/// Universe files are text files, containing a directory of CellBlock.
///
///   cell_block_dim: %d
///   n_cell_blocks: %d
///
///   (CellBlocks follow, each starting with the CellBlock label to identify the start)
///
///   CellBlock: %d, %d
///   (Cell data follows; list of state values separated by ` ` or `\n`s)



b32
load_universe_from_file(const char *filename, Universe *universe, SimulateOptions *simulate_options);


#endif