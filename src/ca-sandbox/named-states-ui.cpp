#include "named-states-ui.h"

#include "text.h"
#include "util.h"
#include "allocate.h"
#include "print.h"
#include "extendable-array.h"
#include "colour.h"

#include "rule.h"
#include "named-states.h"

#include "imgui.h"
#include "imgui_user.h"
#include <stdio.h>


void
do_named_states_ui(RuleConfiguration *rule_config, CellState *currently_active_state)
{
  NamedStates *named_states = &rule_config->named_states;

  if (named_states->states.elements)
  {
    if (ImGui::Begin("Named States"))
    {
      // Need to use a separate buffer for the text edit, because NamedStates stores a String, not a
      //   buffer.

      char state_buffer[512];

      for (u32 state_n = 0;
           state_n < named_states->states.n_elements;
           ++state_n)
      {
        NamedState *named_state = named_states->states.get(state_n);
        String *state_name = &named_state->name;

        u32 old_state_length = string_length(*state_name);

        copy_string(state_buffer, state_name->start, old_state_length);
        state_buffer[old_state_length] = '\0';

        ImGui::PushID(state_n);
        vec4 state_colour = get_state_colour(named_state->value);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, state_colour);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, lighten_colour(state_colour));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, darken_colour(state_colour));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, {0, 0, 0, 0.5});
        ImGui::RadioButton("##make state active", currently_active_state, named_state->value);
        ImGui::PopStyleColor(4);
        ImGui::SameLine();

        ImGui::PushItemWidth(-70);
        if (ImGui::InputText("##state editing", state_buffer, array_count(state_buffer)))
        {
          u32 new_state_length = strlen(state_buffer)+1;
          char *new_state_buffer = allocate(char, new_state_length);
          copy_string(new_state_buffer, state_buffer, new_state_length);

          un_allocate((void*)state_name->start);
          state_name->start = new_state_buffer;
          state_name->end = state_name->start + new_state_length-1;
        }

        b32 was_null_state = is_null_state(rule_config, named_state->value);
        b32 is_now_null_state = was_null_state;
        ImGui::SameLine();
        ImGui::Text("Null:");
        ImGui::SameLine();
        if (ImGui::Checkbox("##null state", (bool *)&is_now_null_state))
        {
          if (is_now_null_state && !was_null_state)
          {
            // Add to null states
            rule_config->null_states.add(named_state->value);
          }
          else if (was_null_state && !is_now_null_state)
          {
            // Remove from null states
            rule_config->null_states.remove(named_state->value);
          }
        }

        ImGui::PopID();
      }

      if (ImGui::Button("Add State"))
      {
        // Build new state string
        char new_state_name[512];
        snprintf(new_state_name, array_count(new_state_name), "New State #%d", named_states->states.n_elements);

        // Copy string into heap allocated string
        u32 new_state_length = strlen(new_state_name)+1;
        char *new_state_buffer = allocate(char, new_state_length);
        copy_string(new_state_buffer, new_state_name, new_state_length);

        NamedState new_state = {
          .name = {
            .start = new_state_buffer,
            .end = new_state_buffer + new_state_length-1
          },
          .value = get_next_unused_state_value(named_states)
        };

        named_states->states.add(new_state);
      }
    }

    ImGui::End();
  }
}