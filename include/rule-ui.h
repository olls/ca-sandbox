#ifndef RULE_UI_H_DEF
#define RULE_UI_H_DEF

#include "file-picker.h"

#include "rule.h"

/// @file
///


/// Holds the state for the rule GUI
struct RuleUI
{
  FilePicker file_picker;
  b32 reload_rule_file;
};


void
do_rule_ui(RuleUI *rule_ui, Rule *rule);


#endif