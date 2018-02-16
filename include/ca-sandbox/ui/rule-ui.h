#ifndef RULE_UI_H_DEF
#define RULE_UI_H_DEF

#include "interface/file-picker.h"

#include "ca-sandbox/rule.h"
#include "ca-sandbox/files-loaded-state.h"

/// @file
///


/// Holds the state for the rule GUI
struct RuleUI
{
  FilePicker file_picker;
  b32 save_rule_file;
};


void
do_rule_ui(RuleUI *rule_ui, Rule *rule, RuleCreationThread *rule_creation_thread, FilesLoadedState *files_loaded_state);


#endif