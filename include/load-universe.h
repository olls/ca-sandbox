#ifndef LOAD_UNIVERSE_H_DEF
#define LOAD_UNIVERSE_H_DEF

#include "types.h"
#include "text.h"

#include "universe.h"
#include "simulate.h"
#include "rule.h"


/// Universe files are text files, containing a directory of CellBlock%s.
///
///   cell_block_dim: %d
///   n_cell_blocks: %d
///
///   (CellBlocks follow, each starting with the CellBlock label to identify the start)
///
///   CellBlock: %d, %d
///   (Cell data follows; list of state values separated by ` ` or `\n`s)


b32
load_universe_from_file(String file_string, Universe *universe, NamedStates *named_states);


b32
load_simulate_options(String file_string, SimulateOptions *simulate_options);


b32
load_cell_initialisation_options(String file_string, CellInitialisationOptions *cell_intialisation_options, NamedStates *named_states);


#endif