#ifndef SAVE_RULE_CONFIG_H_DEF
#define SAVE_RULE_CONFIG_H_DEF

#include "engine/types.h"

#include "ca-sandbox/load-rule.h"


b32
save_rule_config_to_file(const char *filename, RuleConfiguration *rule_config);


#endif