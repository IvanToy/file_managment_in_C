#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "../header_files/delete_dir.h"

int is_dir_empty(char *dir_name);
void delete_not_empty_dir(char *dir_name);

void delete_dir(struct Options *chosen_options)
{
    int is_empty;
    int is_removed;

    for (int i = 0; i < chosen_options->names_count; i++)
    {
        is_empty = is_dir_empty(chosen_options->names[i]);
        if (is_empty == -1)
        {
            fprintf(stderr, "Error checking %s directory", chosen_options->names[i]);
        }
        else if (is_empty == 1)
        {
            is_removed = rmdir(chosen_options->names[i]);
            is_removed ? fprintf(stdout, "The %s directory had been removed\n", chosen_options->names[i]) : fprintf(stderr, "Error: %s could not be removed", chosen_options->names[i]);
        }
        else if (is_empty == 0)
        {
            delete_not_empty_dir(chosen_options->names[i]);
            fprintf(stdout, "The %s directory has been removed\n", chosen_options->names[i]);
        }
    }
    free(chosen_options->names);
    free(chosen_options);
}

int is_dir_empty(char *dir_name)
{
    DIR *dir = opendir(dir_name);

    if (dir == NULL)
    {
        perror("Error opening directory");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG || entry->d_type == DT_DIR)
        {
            closedir(dir);
            return 0;
        }
    }
    closedir(dir);
    return 1;
}

void delete_not_empty_dir(char *dir_name)
{

    DIR *dir;
    struct dirent *entry;
    char path[1024];

    dir = opendir(dir_name);
    if (dir == NULL)
    {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            delete_not_empty_dir(path);
        }
        else
        {
            remove(path);
        }
    }

    closedir(dir);

    rmdir(dir_name);
}