#include <string.h>
#include <stdlib.h>

#include "parsing.h"

#include "types.h"
#include "allocate.h"
#include "text.h"


u32
read_u32_list(String string, u32 **list_result)
{
  u32 n_items = 0;
  u32 list_size = 0;

  while (string.current_position != string.end)
  {
    consume_until(&string, is_num);

    if (string.current_position != string.end)
    {
      u32 item = get_u32(&string);

      u32 new_n_items = n_items + 1;

      if (new_n_items > list_size)
      {
        if (list_size == 0)
        {
          list_size = 4;
        }
        else
        {
          list_size *= 2;
        }

        u32 *new_list = allocate(u32, list_size);

        if (*list_result != 0)
        {
          memcpy(*list_result, new_list, n_items);
          free(*list_result);
        }

        *list_result = new_list;
      }

      (*list_result)[n_items] = item;
      n_items = new_n_items;
    }
  }

  return n_items;
}