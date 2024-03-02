#ifndef CLI_INTERFACE_H
#define CLI_INTERFACE_H

struct Options
{
    char command[9];
    char **names;
    char **renames;
    char dir_path[101];
    int names_count;
};

struct Options *cli_interface();

#endif