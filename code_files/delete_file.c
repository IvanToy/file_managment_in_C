#include <stdio.h>
#include <stdlib.h>
#include "../header_files/delete_file.h"


void delete_file(struct Options *chosen_options)
{

    int is_deleted;

    for (int i = 0; i < chosen_options->names_count; i++)
    {
        is_deleted = remove(chosen_options->names[i]);

        if (is_deleted == 0)
        {
            fprintf(stdout, "%s has been deleted\n", chosen_options->names[i]);
        }
        else
        {
            fprintf(stderr, "Error: unable to delete %s\n", chosen_options->names[i]);
            return;
        }
    }

    free(chosen_options->names);
    free(chosen_options);

    return;
}