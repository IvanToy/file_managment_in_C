#include <stdio.h>
#include <stdlib.h>
#include "../header_files/move_dir.h"

void move_dir(struct Options *chosen_options)
{

    int is_moved;

    for (int i = 0; i < chosen_options->names_count; i++)
    {

        is_moved = rename(chosen_options->names[i], chosen_options->dir_path);

        if (is_moved == 0)
        {
            fprintf(stdout, "%s directory has been moved to %s\n", chosen_options->names[i], chosen_options->dir_path);
        }
        else
        {
            fprintf(stderr, "Error: unable to move %s directory to %s\n", chosen_options->names[i], chosen_options->dir_path);
        }
    }

    free(chosen_options->names);
    free(chosen_options);
}