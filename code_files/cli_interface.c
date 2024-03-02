#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header_files/cli_interface.h"

struct Options *cli_interface()
{
    _Bool true = 1;

    int choice = 0;

    struct Options *chosen_options = (struct Options *)malloc(sizeof(struct Options));

    if (chosen_options == NULL)
    {
        free(chosen_options);
        exit(1);
    }

    printf("------------------------------------------------------------------\n");
    printf("               Welcome to file management system!\n");
    printf("------------------------------------------------------------------");

    while (true)
    {
        if (choice == 0)
        {
            printf("\n\n           Choose what you want to work with:\n");
            printf("-Enter number 1 to work with file(s)\n");
            printf("-Enter number 2 to work with directory(ies)\n");
            printf("Enter:");
            scanf("%d", &choice);
            system("clear");
        }

        if (choice == 1)
        {
            printf("               File(s) Operations:\n");
            printf("-Create file(s): type \'create_f\'\n");
            printf("-Delete file(s): type \'delete_f\'\n");
            printf("-Rename file(s): type \'rename_f\'\n");
            printf("-Edit file: type \'edit_f\'\n");
            printf("-Move file(s): type \'move_f\'\n");
            printf("-Go back to main menu: type \'back\'\n");
            printf("Enter a command:");
            scanf("%8s", chosen_options->command);

            if (strcmp(chosen_options->command, "back") == 0)
            {
                free(chosen_options);
                choice = 0;
                system("clear");
            }
            else if (strcmp(chosen_options->command, "create_f") == 0)
            {
                printf("Enter the number of files to create:\n");
                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->names == NULL)
                {
                    free(chosen_options->names);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter file name (max file name is 50 chars) %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                true = !true;
            }
            else if (strcmp(chosen_options->command, "delete_f") == 0)
            {

                printf("Enter the number of files to delete:\n");
                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->names == NULL)
                {
                    free(chosen_options->names);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter file name %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                true = !true;
            }
            else if (strcmp(chosen_options->command, "rename_f") == 0)
            {

                printf("Enter the number of files to rename:\n");

                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->names == NULL)
                {
                    free(chosen_options->names);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter file name to rename %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                chosen_options->renames = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->renames == NULL)
                {
                    free(chosen_options->renames);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter new file name %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->renames[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->renames[i], buffer, strlen(buffer));
                    chosen_options->renames[i][strlen(buffer)] = '\0';
                }

                true = !true;
            }
            else if (strcmp(chosen_options->command, "edit_f") == 0)
            {

                chosen_options->names_count = 1;

                printf("Enter the file name to edit:");
                char buffer[51];
                scanf("%50s", buffer);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->names == NULL)
                {
                    free(chosen_options->names);
                    exit(1);
                }

                chosen_options->names[0] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                strncpy(chosen_options->names[0], buffer, strlen(buffer));
                chosen_options->names[0][strlen(buffer)] = '\0';

                true = !true;
            }
            else if (strcmp(chosen_options->command, "move_f") == 0)
            {

                printf("Enter the number of files to move:\n");
                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter file name %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                printf("Enter the path to a new location(max location name is 100 char):\n");
                scanf("%100s", chosen_options->dir_path);

                true = !true;
            }
            else
            {
                printf("Invalid command. Try again\n\n");
            }
        }
        else if (choice == 2)
        {
            printf("               Directory(ies) Operations:\n");
            printf("-Create directory(ies): type \'create_d\'\n");
            printf("-Delete directory(ies): type \'delete_d\'\n");
            printf("-Rename directory(ies): type \'rename_d\'\n");
            printf("-Move directory(ies): type \'move_d\'\n");
            printf("-Go back to main menu: type \'back\'\n");
            printf("Enter a command:");
            scanf("%8s", chosen_options->command);

            if (strcmp(chosen_options->command, "back") == 0)
            {
                free(chosen_options);
                choice = 0;
                system("clear");
            }
            else if (strcmp(chosen_options->command, "create_d") == 0)
            {
                printf("Enter the number of directories to create:\n");
                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->names == NULL)
                {
                    free(chosen_options->names);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter directory name (max file name is 50 chars) %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                true = !true;
            }
            else if (strcmp(chosen_options->command, "delete_d") == 0)
            {

                printf("Enter the number of directories to delete:\n");
                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->names == NULL)
                {
                    free(chosen_options->names);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter directory name %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                true = !true;
            }
            else if (strcmp(chosen_options->command, "rename_d") == 0)
            {

                printf("Enter the number of directories to rename:\n");

                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->names == NULL)
                {
                    free(chosen_options->names);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter directory name to rename %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                chosen_options->renames = (char **)malloc(chosen_options->names_count * sizeof(char *));

                if (chosen_options->renames == NULL)
                {
                    free(chosen_options->renames);
                    exit(1);
                }

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter new directory name %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->renames[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->renames[i], buffer, strlen(buffer));
                    chosen_options->renames[i][strlen(buffer)] = '\0';
                }

                true = !true;
            }
            else if (strcmp(chosen_options->command, "move_d") == 0)
            {

                printf("Enter the number of directories to move:\n");
                scanf("%d", &chosen_options->names_count);

                chosen_options->names = (char **)malloc(chosen_options->names_count * sizeof(char *));

                for (int i = 0; i < chosen_options->names_count; i++)
                {
                    printf("Enter directory name %d:\n", i + 1);
                    char buffer[51];
                    scanf("%50s", buffer);

                    chosen_options->names[i] = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
                    strncpy(chosen_options->names[i], buffer, strlen(buffer));
                    chosen_options->names[i][strlen(buffer)] = '\0';
                }

                printf("Enter the path to a new location(max location name is 100 char):\n");
                scanf("%100s", chosen_options->dir_path);

                true = !true;
            }
            else
            {
                printf("Invalid command. Try again\n\n");
            }
        }
        else
        {
            printf("Wrong option, please try again");
            free(chosen_options);
            system("clear");
            exit(1);
        }
    }
    return chosen_options;
}