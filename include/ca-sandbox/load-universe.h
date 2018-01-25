#ifndef LOAD_UNIVERSE_H_DEF
#define LOAD_UNIVERSE_H_DEF

#include "engine/types.h"
#include "engine/text.h"

#include "ca-sandbox/cell-blocks.h"
#include "ca-sandbox/simulate.h"
#include "ca-sandbox/rule.h"

/// @file
/// @brief  Functions for parsing and loading `.cells` files
///


/// Universe files are text files, containing a directory of CellBlock%s.
///
///   cell_block_dim: %d
///   n_cell_blocks: %d
///
///   (CellBlocks follow, each starting with the CellBlock label to identify the start)
///
///   CellBlock: %d, %d
///   (Cell data follows; list of state values separated by ` ` or `\n`s)


Universe *
load_universe(const char *filename, SimulateOptions *simulate_options, CellInitialisationOptions *cell_initialisation_options, NamedStates *named_states, Array::Array<char>& error_message);


void
debug_blank_universe(Universe *universe, CellInitialisationOptions *cell_initialisation_options);


#endif