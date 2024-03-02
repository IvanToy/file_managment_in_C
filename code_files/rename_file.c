#include <stdlib.h>
#include <stdio.h>
#include "../header_files/rename_file.h"

void rename_file(struct Options *chosen_option)
{
    int is_renamed;

    for (int i = 0; i < chosen_option->names_count; i++)
    {
        is_renamed = rename(chosen_option->names[i], chosen_option->renames[i]);

        if (is_renamed == 0)
        {
            fprintf(stdout, "%s has been rename to %s\n", chosen_option->names[i], chosen_option->renames[i]);
        }
        else
        {
            fprintf(stderr, "Error: unable to rename %s to %s", chosen_option->names[i], chosen_option->renames[i]);
        }
    }

    free(chosen_option->names);
    free(chosen_option->renames);
    free(chosen_option);

    return;
}