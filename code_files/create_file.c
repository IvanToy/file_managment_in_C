#include <stdio.h>
#include <stdlib.h>
#include "../header_files/create_file.h"


void create_file(struct Options *chosen_options)
{

    FILE *fp;

    for (int i = 0; i < chosen_options->names_count; i++)
    {
        fp = fopen(chosen_options->names[i], "w");

        if (fp == NULL)
        {
            fprintf(stderr, "Error creating file %s\n", chosen_options->names[i]);
            return;
        }
        else
        {
            fprintf(stdout, "%s has been created\n", chosen_options->names[i]);
        }

        fclose(fp);
    }

    free(chosen_options->names);
    free(chosen_options);

    return;
}