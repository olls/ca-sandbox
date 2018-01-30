#ifndef CELL_TOOLS_H_DEF
#define CELL_TOOLS_H_DEF

#include "ca-sandbox/cell-selections-ui.h"
#include "ca-sandbox/cell.h"
#include "ca-sandbox/load-rule.h"

#include "engine/my-array.h"

#include "header-libs/flags-enum.h"


// Cannot use scoped enums with bitwise logic; therefore `CellToolFlags__` prefix.
enum CellToolFlags
{
  CellToolFlags__zero = 0x00,

  CellToolFlags__SetSelectionNull = 0x01,
  CellToolFlags__DeleteNullBlocks = 0x02
};

MAKE_FLAGS_ENUM_OPS(CellToolFlags)


struct CellTools {
  CellToolFlags flags;

  CellState state;
};


void
perform_cell_tools(CellTools *cell_tools, CellSelectionsUI *cell_selections_ui, Universe *universe, RuleConfiguration *rule_configuration);


#endif