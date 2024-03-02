#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./header_files/cli_interface.h"
#include "./header_files/create_file.h"
#include "./header_files/delete_file.h"
#include "./header_files/rename_file.h"
#include "./header_files/edit_file.h"
#include "./header_files/move_file.h"
#include "./header_files/create_dir.h"
#include "./header_files/delete_dir.h"
#include "./header_files/rename_dir.h"
#include "./header_files/move_dir.h"

int main(void)
{

  int back_to_menu = 0;

  struct Options *chosen_options = cli_interface();

  if (chosen_options != NULL)
  {

    int length = strlen(chosen_options->command);

    char option = chosen_options->command[length - 1];

    if (option == 'f')
    {
      if (strcmp(chosen_options->command, "create_f") == 0)
      {
        create_file(chosen_options);
      }
      else if (strcmp(chosen_options->command, "delete_f") == 0)
      {
        delete_file(chosen_options);
      }
      else if (strcmp(chosen_options->command, "rename_f") == 0)
      {
        rename_file(chosen_options);
      }
      else if (strcmp(chosen_options->command, "edit_f") == 0)
      {
        edit_file(chosen_options);
      }
      else if (strcmp(chosen_options->command, "move_f") == 0)
      {
        move_file(chosen_options);
      }
    }
    else if (option == 'd')
    {
      if (strcmp(chosen_options->command, "create_d") == 0)
      {
        create_dir(chosen_options);
      }
      else if (strcmp(chosen_options->command, "delete_d") == 0)
      {
        delete_dir(chosen_options);
      }
      else if (strcmp(chosen_options->command, "rename_d") == 0)
      {
        rename_dir(chosen_options);
      }
      else if (strcmp(chosen_options->command, "move_d") == 0)
      {
        move_dir(chosen_options);
      }
    }
  }

  return 0;
}