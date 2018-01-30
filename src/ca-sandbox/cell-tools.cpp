#include "ca-sandbox/cell-tools.h"

#include "ca-sandbox/rule.h"


void
set_cells_to_state(CellSelectionsUI *cell_selections_ui, Universe *universe, CellState new_state)
{
  s32vec2 start_block = cell_selections_ui->selection_start.cell_block_position;
  s32vec2 end_block = cell_selections_ui->selection_end.cell_block_position;

  s32vec2 start_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_start.cell_position, universe->cell_block_dim));
  s32vec2 end_cell = vec2_to_s32vec2(vec2_multiply(cell_selections_ui->selection_end.cell_position, universe->cell_block_dim));

  for (u32 cell_block_slot = 0;
       cell_block_slot < universe->hashmap_size;
       ++cell_block_slot)
  {
    CellBlock *cell_block = universe->hashmap[cell_block_slot];

    while (cell_block != 0)
    {
      if (cell_block->block_position.x >= start_block.x &&
          cell_block->block_position.y >= start_block.y &&
          cell_block->block_position.x <= end_block.x &&
          cell_block->block_position.y <= end_block.y)
      {
        s32vec2 this_block_start_cell = {0, 0};
        s32vec2 this_block_end_cell = {(s32)universe->cell_block_dim, (s32)universe->cell_block_dim};

        if (cell_block->block_position.x == start_block.x)
        {
          this_block_start_cell.x = start_cell.x;
        }
        if (cell_block->block_position.y == start_block.y)
        {
          this_block_start_cell.y = start_cell.y;
        }
        if (cell_block->block_position.x == end_block.x)
        {
          this_block_end_cell.x = end_cell.x;
        }
        if (cell_block->block_position.y == end_block.y)
        {
          this_block_end_cell.y = end_cell.y;
        }

        s32vec2 cell_position;
        for (cell_position.y = this_block_start_cell.y;
             cell_position.y < this_block_end_cell.y;
             ++cell_position.y)
        {
          for (cell_position.x = this_block_start_cell.x;
               cell_position.x < this_block_end_cell.x;
               ++cell_position.x)
          {
            u32 cell_index = get_cell_index_in_block(universe, cell_position);
            cell_block->cell_states[cell_index] = new_state;
          }
        }
      }

      cell_block = cell_block->next_block;
    }
  }
}


void
delete_null_cell_blocks(Universe *universe, RuleConfiguration *rule_config)
{
  for (u32 cell_block_slot = 0;
       cell_block_slot < universe->hashmap_size;
       ++cell_block_slot)
  {
    CellBlock *cell_block = universe->hashmap[cell_block_slot];

    while (cell_block != 0)
    {
      b32 block_null = true;

      s32vec2 cell_position;
      for (cell_position.y = 0;
           cell_position.y < universe->cell_block_dim;
           ++cell_position.y)
      {
        for (cell_position.x = 0;
             cell_position.x < universe->cell_block_dim;
             ++cell_position.x)
        {
          u32 cell_index = get_cell_index_in_block(universe, cell_position);
          CellState cell_state = cell_block->cell_states[cell_index];

          if (!is_null_state(rule_config, cell_state))
          {
            block_null = false;
            break;
          }
        }

        if (!block_null)
        {
          break;
        }
      }

      if (block_null)
      {
        delete_cell_block(universe, cell_block->block_position);
      }

      cell_block = cell_block->next_block;
    }
  }
}


void
perform_cell_tools(CellTools *cell_tools, CellSelectionsUI *cell_selections_ui, Universe *universe, RuleConfiguration *rule_configuration)
{
  if (cell_tools->flags & CellToolFlags__SetSelectionNull)
  {
    if (cell_selections_ui->selection_made)
    {
      set_cells_to_state(cell_selections_ui, universe, cell_tools->state);
    }
  }

  if (cell_tools->flags & CellToolFlags__DeleteNullBlocks)
  {
    delete_null_cell_blocks(universe, rule_configuration);
  }

  cell_tools->flags = CellToolFlags__zero;
}
