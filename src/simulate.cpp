#include "simulate.h"

#include "print.h"
#include "types.h"
#include "cell-storage.h"


Cell *
test_get_cell(Universe *universe, CellBlock *cell_block, s32 cell_x, s32 cell_y)
{
  // Gets a cell from within 1 CELL_BLOCK_DIM of the cell_block

  // TODO: Cache CellBlocks

  Cell *result = 0;

  s32vec2 relative_cell_block = {0, 0};

  if (cell_x < 0)
  {
    relative_cell_block.x = -1;
    cell_x += CELL_BLOCK_DIM;
  }
  if (cell_x >= CELL_BLOCK_DIM)
  {
    relative_cell_block.x = 1;
    cell_x -= CELL_BLOCK_DIM;
  }

  if (cell_y < 0)
  {
    relative_cell_block.y = -1;
    cell_y += CELL_BLOCK_DIM;
  }
  if (cell_y >= CELL_BLOCK_DIM)
  {
    relative_cell_block.y = 1;
    cell_y -= CELL_BLOCK_DIM;
  }

  if (!vec2_eq(relative_cell_block, (s32vec2){0, 0}))
  {
    s32vec2 new_cell_block_position = vec2_add(relative_cell_block, cell_block->block_position);

    CellBlock *new_cell_block = get_cell_block(universe, new_cell_block_position);
    cell_block = new_cell_block;
  }

  if (cell_block != 0)
  {
    result = cell_block->cells + (cell_y * CELL_BLOCK_DIM) + cell_x;
  }

  return result;
}


void
test_transition_rule(Universe *universe, CellBlock *cell_block, s32 cell_x, s32 cell_y)
{
  if (!(cell_block->block_position.x >= -2 && cell_block->block_position.x <= 2 &&
        cell_block->block_position.y >= -2 && cell_block->block_position.y <= 2))
  {
    print("Hit boundary at %d %d\n", cell_block->block_position.x, cell_block->block_position.y);
  }
  else
  {
    Cell *cell = test_get_cell(universe, cell_block, cell_x, cell_y);

    // Test Von Neumann neighbourhood
    Cell *cell_north = test_get_cell(universe, cell_block, cell_x, cell_y - 1);
    Cell *cell_east  = test_get_cell(universe, cell_block, cell_x + 1, cell_y);
    Cell *cell_south = test_get_cell(universe, cell_block, cell_x, cell_y + 1);
    Cell *cell_west  = test_get_cell(universe, cell_block, cell_x - 1, cell_y);

    Cell *cell_north_east = test_get_cell(universe, cell_block, cell_x + 1, cell_y - 1);
    Cell *cell_south_east = test_get_cell(universe, cell_block, cell_x + 1, cell_y + 1);
    Cell *cell_south_west = test_get_cell(universe, cell_block, cell_x - 1, cell_y + 1);
    Cell *cell_north_west = test_get_cell(universe, cell_block, cell_x - 1, cell_y - 1);

    u32 n_enabled_neighbours = (cell_north->previous_state + cell_east->previous_state + cell_south->previous_state + cell_west->previous_state +
                                cell_north_east->previous_state + cell_south_east->previous_state + cell_south_west->previous_state + cell_north_west->previous_state +
                                cell->previous_state);

    // if (n_enabled_neighbours == 3 ||
    //     n_enabled_neighbours == 7 ||
    //     n_enabled_neighbours == 8)
    // {
    //   cell->state = 1;
    // }
    if (n_enabled_neighbours <= 4)
    {
      cell->state = 0;
    }
    else
    {
      cell->state = 1;
    }
  }
}


void
test_simulate_cells(Universe *universe, u64 current_frame)
{
  // First copy all Cell states into previous_state

  for (u32 hash_slot = 0;
       hash_slot < universe->hashmap_size;
       ++hash_slot)
  {
    CellBlock *cell_block = universe->hashmap[hash_slot];

    if (cell_block != 0 && cell_block->initialised)
    {
      for (u32 cell_index = 0;
           cell_index < CELL_BLOCK_DIM * CELL_BLOCK_DIM;
           ++cell_index)
      {
        Cell *cell = cell_block->cells + cell_index;

        cell->previous_state = cell->state;
      }
    }
  }

  // Loop through all CellBlocks, simulating them until they are all flagged as being
  //  simulated on this frame. This is so that new CellBlocks - which have been created
  //  when the simulation reaches the edge of an existing CellBlock - are simulated in
  //  the same frame.

  b32 simulated_any_blocks = true;
  while (simulated_any_blocks)
  {
    simulated_any_blocks = false;

    for (u32 hash_slot = 0;
         hash_slot < universe->hashmap_size;
         ++hash_slot)
    {
      CellBlock *cell_block = universe->hashmap[hash_slot];

      if (cell_block != 0 &&
          cell_block->initialised &&
          cell_block->last_simulated_on_frame != current_frame)
      {
        cell_block->last_simulated_on_frame = current_frame;
        simulated_any_blocks = true;

        for (s32 cell_y = 0;
             cell_y < CELL_BLOCK_DIM;
             ++cell_y)
        {
          for (s32 cell_x = 0;
               cell_x < CELL_BLOCK_DIM;
               ++cell_x)
          {
            test_transition_rule(universe, cell_block, cell_x, cell_y);
          }
        }
      }
    }
  }
}