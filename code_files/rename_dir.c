#include <stdio.h>
#include <stdlib.h>

#include "../header_files/rename_dir.h"

void rename_dir(struct Options *chosen_options)
{

    int is_renamed;

    for (int i = 0; i < chosen_options->names_count; i++)
    {

        is_renamed = rename(chosen_options->names[i], chosen_options->renames[i]);

        if (is_renamed == 0)
        {
            fprintf(stdout, "%s directory has been rename to %s\n ", chosen_options->names[i], chosen_options->renames[i]);
        }
        else
        {
            fprintf(stderr, "Error: unable to rename %s directory to %s", chosen_options->names[i], chosen_options->renames[i]);
        }
    }

    free(chosen_options->names);
    free(chosen_options->renames);
    free(chosen_options);

    return;
}