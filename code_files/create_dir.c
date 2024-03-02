#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../header_files/create_dir.h"

void create_dir(struct Options *chosen_options)
{

    int is_created;

    for (int i = 0; i < chosen_options->names_count; i++)
    {
        is_created = mkdir(chosen_options->names[i], 0777);

        if (is_created == 0)
        {
            fprintf(stdout, "%s directory has been created\n", chosen_options->names[i]);
        }
        else
        {
            fprintf(stderr, "Error: %s directory hasn't been created\n", chosen_options->names[i]);
        }
    }

    free(chosen_options->names);
    free(chosen_options);

    return;
}