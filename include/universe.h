#ifndef UNIVERSE_H_DEF
#define UNIVERSE_H_DEF

#include "cell.h"
#include "vectors.h"

/// @file
/// @brief Defines the storage data structure for the Cell%s.
///
/// Implementation:
/// - CellBlocks store NxN block of cells
/// - Universe is made up of a grid of CellBlocks
/// - All cells in a CellBlock are initialised
/// - CellBlocks are stored on the heap, and accessed via a hashmap on the block position.
///   - ie.: block_pos = cell.pos / block_size
/// - On simulation, each cell block is simulated as a whole
/// - To iterate over all CellBlock%s / Cell%s just loop through the hashmap.
/// - Possible optimisation: CellBlocks store pointers to neighbours for quick access to border cell
///     states.
///
/// Resetting Universes:
/// - The currently simulated state of the universe is stored in one main Universe hash map
/// - The canonical initial state of the universe is stored in the .cells file.
/// - Modifications to the universe can be saved to a .cells file, and then loaded as an initial
///     state.


const u32 DEFAULT_CELL_BLOCK_DIM = 16;

/// Storage for a square block of Cell%s
struct CellBlock
{
  /// Used to determine whether a slot in the Universe hashmap is in use.
  b32 slot_in_use;

  /// Used by simulation.cpp to ensure the block is only simulated once per frame.
  u64 last_simulated_on_frame;

  /// The position of the block relative to the origin of the CA, in block space.
  s32vec2 block_position;

  /// The next CellBlock in this hash slot in the hashmap of Universe.  0 if this is the last
  ///   CellBlock in this slot.
  CellBlock *next_block;

  /// Array of cells within the block. length of universe->cell_block_dim ^2
  Cell cells[];
};


/// The initial length of the Universe hashmap.

/// This is the number of CellBlock%s which can fit in the Universe with zero conflicts.
const u32 INITIAL_CELL_HASHMAP_SIZE = 512;


/// The main storage struct for a CA

/// Stores a hashmap of CellBlock pointers.
struct Universe
{
  /// The dimension of a CellBlock, ie. the CellBlock%s will contain a square of Cell%s with size
  ///   cell_block_dim x cell_block_dim.
  u32 cell_block_dim;

  /// Pointer to an array of CellBlock pointers.
  CellBlock **hashmap;

  /// The length of the hashmap array.
  u32 hashmap_size;
};


void
init_cell_hashmap(Universe *universe);


void
destroy_cell_hashmap(Universe *universe);


CellBlock *
create_cell_block(Universe *universe, CellInitialisationOptions *cell_initialisation_options, s32vec2 search_cell_block_position);


CellBlock *
create_uninitialised_cell_block(Universe *universe, s32vec2 search_cell_block_position);


CellBlock *
get_or_create_cell_block(Universe *universe, CellInitialisationOptions *cell_initialisation_options, s32vec2 search_cell_block_position);


CellBlock *
get_existing_cell_block(Universe *universe, s32vec2 search_cell_block_position);


Cell *
get_cell_from_block(Universe *universe, CellBlock *cell_block, s32vec2 cell_coord);


#endif