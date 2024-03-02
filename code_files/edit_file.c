/*** includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

// included them to make sure that the compiler can compile with the getline function that is part of the stdio.h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "../header_files/edit_file.h"

/*** defines ***/

#define EDITOR_VERSION "0.0.1"
#define TAB_STOP 8
#define QUIT_TIMES 3
#define CTRL_KEY(key) ((key) & 0x1f)
#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)

enum editor_key
{
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT, // 1001
    ARROW_UP,    // 1002
    ARROW_DOWN,  // 1003
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN

};

enum editor_highlight
{
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

/************************************************* data ******************************************************/

/* This structure represents a single line of the file we are editing. */
typedef struct e_row
{
    int index;
    int size;          /* Size of the row, excluding the null term. */
    int r_size;        /* Size of the rendered row on the terminal screen. */
    char *chars;       /* Row content. */
    char *render;      /* Row content "rendered" for screen (for TABs). */
    unsigned char *hl; /* array of highlighting  */
    int hl_open_comment;
} e_row;
;

struct editor_syntax
{
    char *file_type;
    char **file_match;
    char **keywords;
    char *singleline_comment_start;
    char *multiline_comment_start;
    char *multiline_comment_end;
    int flags;
};

/************************************************* filetypes  ************************************************/

char *C_HL_extensions[] = {".c", ".h", ".cpp", ".txt", ".js", ".py", NULL};
char *C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", NULL};

struct editor_syntax HLDB[] = {
    "c",
    C_HL_extensions,
    C_HL_keywords,
    "//",
    "/*",
    "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

// this struct is used to stor x and y coordinates of terminal windows and etc
struct editor_config
{
    int cx, cy; /* Cursor x and y position in characters */
    int rx;
    int row_off;     /* Offset of row displayed. */
    int col_off;     /* Offset of column displayed. */
    int screen_rows; /* Number of rows that we can show */
    int screen_cols; /* Number of cols that we can show */
    int num_rows;    /* Number of rows */
    e_row *row;      /* Rows. Essentially it is a array of struct of single rows */
    int dirty;       /* File modified but not saved. */
    char *filename;  /* Currently open filename */
    char status_message[80];
    time_t status_message_time;
    struct editor_syntax *syntax;
    struct termios initial_termios;
};

struct editor_config E;

struct append_buffer
{
    char *buffer;
    int length;
};

#define APPEND_BUFFER \
    {                 \
        NULL, 0       \
    }

/******************************************************** prototypes *****************************************************/

void enable_raw_mode();
void disable_raw_mode();
int editor_read_key();
void editor_process_keypress();
void editor_move_cursor(int key);
void editor_refresh_screen();
void editor_draw_rows();
void editor_scroll();
void editor_save();
void editor_del_char();
void editor_insert_new_line();
void editor_find();
void editor_select_syntax_highlight();
int is_separator(int c);
int editor_syntax_to_color(int hl);
void editor_update_syntax(e_row *row);
void editor_find_callback(char *query, int key);
int editor_row_rx_to_cx(e_row *row, int rx);
char *editor_prompt(char *prompt, void (*callback)(char *, int));
void editor_row_append_string(e_row *row, char *s, size_t length);
void editor_free_row(e_row *row);
void editor_del_row(int at);
void editor_row_del_char(e_row *row, int at);
char *editor_rows_to_string(int *buf_len);
void editor_insert_char(int c);
void editor_row_insert_char(e_row *row, int at, int c);
void editor_set_status_message(const char *fmt, ...);
void editor_draw_message_bar(struct append_buffer *ab);
void editor_draw_status_bar(struct append_buffer *ab);
void editor_update_row(e_row *row);
void editor_open(struct Options *chosen_options);
void editor_insert_row(int at, char *str, size_t length);
void init_editor();
void buffer_append(struct append_buffer *ab, const char *s, int length);
void append_buffer_free(struct append_buffer *ab);
int editor_row_cx_to_rx(e_row *row, int cx);
int get_windows_size(int *rows, int *cols);
int get_cursor_position(int *rows, int *cols);
void error_handler(const char *str);

/************************************************************** main function ************************************************/

void edit_file(struct Options *chosen_options)
{

    enable_raw_mode();
    init_editor();
    editor_open(chosen_options);

    editor_set_status_message("HELP: Ctrl + S = save | Ctrl + Q = quit | Ctrl + F = find");

    while (1)
    {
        editor_refresh_screen();
        editor_process_keypress();
    }

    free(chosen_options->names);
    free(chosen_options);

    return;
}

/************************************************** terminal ****************************************************************************/

void error_handler(const char *str)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // writing to the terminal ANSI/VT100 control escape sequence \x1b[2J - it means to erase the screen with background color and move the cursor to home(upper left corner);
    write(STDOUT_FILENO, "\x1b[H", 3);
    // sets the cursor to the home;

    perror(str); // prints a string indicating which function caused an error;
    exit(1);     // exit the program which code 1 - miscellaneous error;
}

void disable_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.initial_termios) == -1)
        // tcsetattr comes from termios.h and it sets the params associated with the terminal
        // STDIN_FILENO - comes from unistd.h and it is symbolic constant for stdin(which is 0);
        // TCSAFLUSH - change attributes when output has drained(stoped); also flush pending input;
        //&E.initial_termios - unedited initial terminal attributes;

        error_handler("tcsetattr"); // prints out error if tcsetattr == -1;
}

void enable_raw_mode()
{ /*
  NOTE:The reason for enabling raw mode is to handle keyboard input in a more direct and unprocessed manner.
  1)input processing: in canonical mode, input is processed line by line, but for text editor you want to process each keypress immediately
  2)control chars: Ctrl+C or Ctrl+Z have a specific actions in canonical mode. In raw mode you can handle them yourself and decide how to interpret and respond to them
  3)Arrow keys, function keys, and other special keys often send escape sequences in raw mode, allowing you to distinguish between different keypresses.
    In canonical mode, these keys might not be directly accessible, and you would need to wait for a complete line to be entered.
  4)Immediate feedback: In raw mode, you can receive and respond to each keypress immediately, providing more immediate feedback to the user during text editing.
  5)Disabling Output Processing: Raw mode also often involves disabling certain output processing, ensuring that the terminal doesn't interpret or modify the characters you send to it. This is crucial for rendering user interfaces or displaying edited text.

  */

    if (tcgetattr(STDIN_FILENO, &E.initial_termios) == -1)
        // tcgetattr comes from termios.h and it gets the parameters associated with terminal. It is saved in E struct
        error_handler("tcgetattr");
    // if there is an error getting params then evoke error handler function;

    atexit(disable_raw_mode);
    // it registers a specified function to be called when program terminates normally. It can be registered anywhere but it gets called only upon termination

    struct termios raw_mode = E.initial_termios;
    // Here the original terminal params are saved(copied) in another struct so that we can modify them in order to enter raw mode
    // We turn off flags under to enter raw mode:
    raw_mode.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // c_iflag is a part of terminal param. it is for input modes
    // BRKINT = Signal interrupt on break;
    // ICRNL = Map CR to NL on input.CR is carriage return and NL is a new line.
    // if a program sends a Carriage Return to the terminal, the terminal will treat it as if a Newline was received.
    // INPCK = enable input parity check. It is done because of a tradition.
    // ISTRIP = strip character. It is used to strip eight bit from every byte. It is done because in old times eight bit was used for parity checking.
    // IXON = enable start/stop output control.
    raw_mode.c_oflag &= ~(OPOST);
    // c_oflag is for output mode.
    // OPOST - post-process output.
    raw_mode.c_cflag |= (CS8);
    // c_cflag is for control modes.
    // CS8 - is part of CSIZE(char size). CS8 means each char should be 8 bits.
    raw_mode.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // c_lflag is for local modes.
    // ECHO - enable echo
    // ICANON - canonical input
    // IEXTEN - enable extended input char processing.
    // Extended input processing includes the recognition and interpretation of special input characters, often used for terminal control and line editing.
    // ISIG - enable signals
    raw_mode.c_cc[VMIN] = 0;
    // c_cc is control chars(array);
    // VMIN - min number of chars for non-canonical reads. When set to 0 it means that the read operation will return as soon as any input is available.
    raw_mode.c_cc[VTIME] = 1;
    // VTIME - time out value for non-canonical reads. It means that read will block until at least one byte is available, or timeout period of 0.1 second has passed.

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode) == -1)
        // setting edited params for the terminal to enter raw mode
        error_handler("tcgetattr");
}

int editor_read_key()
{
    int n_read;
    char c;

    while ((n_read = read(STDIN_FILENO, &c, 1)) != 1)
    {
        // reads comes from unistd.h it reads from a file.
        //  reads from the terminal input
        // saves char into the address for c
        // 1 is the number of bytes to read. 1 = a single char.
        if (n_read == -1 && errno != EAGAIN)
            // if read is interrupted by a signal before it reads data it returns -1
            // errno comes from errno.h. It is a global integer that is used to indicate error in functions.
            // EAGAIN - is an error code that is used to indicate that an operation cannot be completed immediately, and calling process should try again later.
            // If read function returns -1 with errno set to EAGAIN, it means that there is no data available at the moment, and the application may want to retry the read operation later.
            error_handler("read");
    }

    if (c == '\x1b')
    {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';

        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';

        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';

                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {
                    case '1':
                        return HOME_KEY;
                    case '3':
                        return DEL_KEY;
                    case '4':
                        return END_KEY;
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    case '7':
                        return HOME_KEY;
                    case '8':
                        return END_KEY;
                    }
                }
            }
            else
            {
                switch (seq[1])
                {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch (seq[1])
            {
            case 'H':
                return HOME_KEY;
            case 'F':
                return END_KEY;
            }
        }
        return '\x1b';
    }
    else
    {
        return c;
    }
}

int get_cursor_position(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        //\x1b[6n - request a current cursor position in the terminal
        return -1;

    while (i < sizeof(buf) - 1)
    { // here we are going over the response given in the above request(typically it looks lie that:\x1b[y;xR, where x = num of rows, y = num of cols)
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }

    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return -1;

    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
        // we save into rows and cols num of rows(x) and num of cols(y). &buf[2] =  y;x
        return -1;

    return 0;
}

int get_windows_size(int *rows, int *cols)
{
    // pointer to rows and cols is passed in init_editor function

    struct winsize ws;
    // winsize struct comes from sys/ioctl.h and termios.h.

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        // isoctl is used for input/output operations. Comes from sys/ioctl.h
        // STDOUT_FILENO is standard output(terminal)
        // TIOCGWINSZ is a request to get the windows size. Comes from termios.h
        //&ws where the windows size is saved
        if (write(STDOUT_FILENO, "\x1b[99C\x1b[999B", 12) != 12)
            //\x1b[99C\x1b[999B - move the cursor by 99 columns and 999 rows in the terminal
            return -1;
        return get_cursor_position(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/************************************************************ syntax highlight *********************************************************/

int is_separator(int c)
{
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
    // strchr comes from string.h
    // it looks for the first occurrence of a char in a string
    // returns a pointer to the matching char in the string.
    // otherwise returns null
}

void editor_update_syntax(e_row *row)
{
    row->hl = realloc(row->hl, row->r_size);
    memset(row->hl, HL_NORMAL, row->r_size);
    // memset comes from sting.h

    if (E.syntax == NULL)
        return;

    char **keywords = E.syntax->keywords;

    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    int scs_length = scs ? strlen(scs) : 0;
    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int prev_sep = 1;
    int in_string = 0;
    int in_comment = (row->index > 0 && E.row[row->index - 1].hl_open_comment);

    int i;
    while (i < row->r_size)
    {
        char c = row->render[i];
        unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

        if (scs_length && !in_string && !in_comment)
        {
            if (!strncmp(&row->render[i], scs, scs_length))
            {
                memset(&row->hl[i], HL_COMMENT, row->r_size - i);
                break;
            }
        }

        if (mcs_len && mce_len && !in_string)
        {
            if (in_comment)
            {
                row->hl[i] = HL_MLCOMMENT;
                if (!strncmp(&row->render[i], mce, mce_len))
                {
                    memset(&row->hl[i], HL_MLCOMMENT, mce_len);
                    i += mce_len;
                    in_comment = 0;
                    prev_sep = 1;
                    continue;
                }
                else
                {
                    i++;
                    continue;
                }
            }
            else if (!strncmp(&row->render[i], mcs, mcs_len))
            {
                memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
                i += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_STRINGS)
        {
            if (in_string)
            {
                row->hl[i] = HL_STRING;
                if (c == '\\' && i + 1 < row->r_size)
                {
                    row->hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (c == in_string)
                    in_string = 0;
                i++;
                prev_sep = 1;
                continue;
            }
            else
            {
                if (c == '"' || c == '\'')
                {
                    in_string = c;
                    row->hl[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS)
        {
            if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) || (c == '.' && prev_hl == HL_NUMBER))
            {
                row->hl[i] = HL_NUMBER;
                i++;
                prev_sep = 0;
                continue;
            }
        }

        if (prev_sep)
        {
            int j;
            for (j = 0; keywords[j]; j++)
            {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen - 1] == '|';
                if (kw2)
                    klen--;
                if (!strncmp(&row->render[i], keywords[j], klen) &&
                    is_separator(row->render[i + klen]))
                {
                    memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != NULL)
            {
                prev_sep = 0;
                continue;
            }
        }

        prev_sep = is_separator(c);
        i++;
    }
}

int editor_syntax_to_color(int hl)
{
    switch (hl)
    {
    case HL_COMMENT:
    case HL_MLCOMMENT:
        return 36;
    case HL_KEYWORD1:
        return 33;
    case HL_KEYWORD2:
        return 32;
    case HL_STRING:
        return 35;
    case HL_NUMBER:
        return 31;
    case HL_MATCH:
        return 34;
    default:
        return 37;
    }
}

void editor_select_syntax_highlight()
{
    E.syntax = NULL;
    if (E.filename == NULL)
        return;

    char *ext = strrchr(E.filename, '.');
    // strchr comes from string.h
    // returns a pointer to the last occurrence of a char in a string.

    for (unsigned int j = 0; j < HLDB_ENTRIES; j++)
    {
        struct editor_syntax *s = &HLDB[j];
        unsigned int i = 0;
        while (s->file_match[i])
        {
            int is_ext = (s->file_match[i][0] == '.');
            if ((is_ext && ext && !strcmp(ext, s->file_match[i])) || (!is_ext && strstr(E.filename, s->file_match[i])))
            {
                s->file_type = &s->file_match[i][1];
                E.syntax = s;

                int file_row;
                for (file_row = 0; file_row < E.num_rows; file_row++)
                {
                    editor_update_syntax(&E.row[file_row]);
                }
                return;
            }
            i++;
        }
    }
}

/************************************************************ row operations ***********************************************************/

int editor_row_cx_to_rx(e_row *row, int cx)
{
    int rx = 0;
    int j;
    for (j = 0; j < cx; j++)
    {
        if (row->chars[j] == '\t')
        {
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        }
        rx++;
    }

    return rx;
}

int editor_row_rx_to_cx(e_row *row, int rx)
{
    int cur_rx = 0;
    int cx;
    for (cx = 0; cx < row->size; cx++)
    {
        if (row->chars[cx] == '\t')
            cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
        cur_rx++;
        if (cur_rx > rx)
            return cx;
    }
    return cx;
}

void editor_update_row(e_row *row)
{
    int tabs = 0;
    int j;
    for (j = 0; j < row->size; j++)
    {
        if (row->chars[j] == '\t')
            tabs++;
    }
    // here we count number of tabs in each line

    free(row->render);
    row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

    int index = 0;

    for (j = 0; j < row->size; j++)
    {
        if (row->chars[j] == '\t')
        {
            row->render[index++] = ' ';
            while (index % TAB_STOP != 0)
                row->render[index++] = ' ';
        }
        else
        {
            row->render[index++] = row->chars[j];
        }
    }

    row->render[index] = '\0';
    row->r_size = index;

    editor_update_syntax(row);
    // the whole point of this function is to copy chars from row->chars to row->render as well as tabs.
}

void editor_free_row(e_row *row)
{
    free(row->render);
    free(row->chars);
    free(row->hl);
}

void editor_del_row(int at)
{
    if (at < 0 || at >= E.num_rows)
        return;
    editor_free_row(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(e_row) * (E.num_rows - at - 1));
    E.num_rows--;
    E.dirty++;
}

void editor_row_insert_char(e_row *row, int at, int c)
{
    if (at < 0 || at > row->size)
        at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    // memove comes from string.h
    // first arg - is a pointer to the destination
    // second arg - is a pointer to the source
    // third - num of bytes to copy
    row->size++;
    row->chars[at] = c;
    editor_update_row(row);
    E.dirty++;
}

void editor_insert_row(int at, char *str, size_t length)
{
    if (at < 0 || at > E.num_rows)
        return;

    E.row = realloc(E.row, sizeof(e_row) * (E.num_rows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(e_row) * (E.num_rows - at));
    for (int j = at + 1; j <= E.num_rows; j++)
        E.row[j].index++;

    E.row[at].index = at;
    // int at = E.num_rows;

    E.row[at].size = length;
    E.row[at].chars = malloc(length + 1);
    memcpy(E.row[at].chars, str, length);
    E.row[at].chars[length] = '\0';

    E.row[at].r_size = 0;
    E.row[at].render = NULL;
    E.row[at].hl = NULL;
    E.row[at].hl_open_comment = 0;
    editor_update_row(&E.row[at]);

    E.num_rows++;
    E.dirty++;
}

void editor_row_append_string(e_row *row, char *s, size_t length)
{
    row->chars = realloc(row->chars, row->size + length + 1);
    memcpy(&row->chars[row->size], s, length);
    row->size += length;
    row->chars[row->size] = '\0';
    editor_update_row(row);
    E.dirty++;
}

void editor_row_del_char(e_row *row, int at)
{
    if (at < 0 || at >= row->size)
        return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editor_update_row(row);
    E.dirty++;
}

/********************************************************** editor operations *************************************************************************/

/* Insert a character at the specified position in a row, moving the remaining
 * chars on the right if needed. */
void editor_insert_char(int c)
{
    if (E.cy == E.num_rows)
    {
        editor_insert_row(E.num_rows, "", 0);
    }

    editor_row_insert_char(&E.row[E.cy], E.cx, c);
    E.cx++;
}

void editor_insert_new_line()
{
    if (E.cx == 0)
    {
        editor_insert_row(E.cy, "", 0);
    }
    else
    {
        e_row *row = &E.row[E.cy];
        editor_insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editor_update_row(row);
    }
    E.cy++;
    E.cx = 0;
}

void editor_del_char()
{
    if (E.cy == E.num_rows)
        return;
    if (E.cx == 0 && E.cy == 0)
        return;

    e_row *row = &E.row[E.cy];
    if (E.cx > 0)
    {
        editor_row_del_char(row, E.cx - 1);
        E.cx--;
    }
    else
    {
        E.cx = E.row[E.cy - 1].size;
        editor_row_append_string(&E.row[E.cy - 1], row->chars, row->size);
        editor_del_row(E.cy);
        E.cy--;
    }
}

/************************************************************ File I/O ***********************************************************/

/* Turn the editor rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, escluding
 * the final nulterm. */
char *editor_rows_to_string(int *buf_len)
{

    int total_length = 0;
    int j;
    for (j = 0; j < E.num_rows; j++)
    {
        total_length += E.row[j].size + 1;
        *buf_len = total_length;
    }

    char *buffer = malloc(total_length);
    char *p = buffer;

    for (j = 0; j < E.num_rows; j++)
    {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }

    return buffer;
}

void editor_open(struct Options *chosen_options)
{
    free(E.filename);
    E.filename = strdup(chosen_options->names[0]);
    // strdup comes from string.h. It makes a copy of the given string, allocating the required memory and assuming you will free that memory
    // filename is saved separately so that it could be displayed in the status bar.

    editor_select_syntax_highlight();

    FILE *fp = fopen(chosen_options->names[0], "r");

    if (!fp)
        error_handler("fopen");

    char *line = NULL;
    size_t line_capacity = 0;
    size_t line_length;

    while ((line_length = getline(&line, &line_capacity, fp)) != -1)
    {
        while (line_length > 0 && (line[line_length - 1] == '\n' ||
                                   line[line_length - 1] == '\r'))
            line_length--;
        editor_insert_row(E.num_rows, line, line_length);
    }
    // getline gets a line from a file and saves it in line. It also gives us the size of the line which is saved into line_capacity
    // then line length is decremented until it reaches newline char or return carrige character
    // line and line length which is now equal to the length of the line without \n and \r passed to editor_insert_row
    // in that function array of struct row is created to hold each line as well as its size.

    free(line);
    fclose(fp);
    free(chosen_options->names);
    free(chosen_options);
    E.dirty = 0;
}

void editor_save()
{
    if (E.filename == NULL)
    {
        E.filename = editor_prompt("Save as: %s (ESC to cancel)", NULL);
        if (E.filename == NULL)
        {
            editor_set_status_message("Save aborted");
            return;
        }
        editor_select_syntax_highlight();
    }

    int length;

    char *buffer = editor_rows_to_string(&length);

    int file_disk = open(E.filename, O_RDWR | O_CREAT, 0664);
    // O_RDWR and O_CREAT come from fcntl.h
    // ftruncate and close from unistd.h
    // O_CREATE tells open to create a new file if it doesn't exist
    // O_RDWR open file for reading and writing
    // 0664 sre linux permission for a file
    // ftruncate sets file's size to the specific range. and if length is larger than specified length it will it cut it off if shorter it will append zeros.

    if (file_disk != -1)
    {
        if (ftruncate(file_disk, length) != -1)
        {
            if (write(file_disk, buffer, length) == length)
            {
                close(file_disk);
                free(buffer);
                editor_set_status_message("%d bytes written to disk", length);
                return;
            }
        }
        close(file_disk);
    }
    free(buffer);
    editor_set_status_message("Can't save! I/O error: %s", strerror(errno));
    // strerror comes from string.h
    // it returns a human readable string for thr error code
}

/************************************************************* Find ***********************************************************/

void editor_find_callback(char *query, int key)
{
    static int last_match = -1;
    static int direction = 1;

    static int saved_hl_line;
    static char *saved_hl = NULL;

    if (saved_hl)
    {
        memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].r_size);
        free(saved_hl);
        saved_hl = NULL;
    }

    if (key == '\r' || key == '\x1b')
    {
        last_match = -1;
        direction = 1;
        return;
    }
    else if (key == ARROW_RIGHT || key == ARROW_DOWN)
    {
        direction = 1;
    }
    else if (key == ARROW_LEFT || key == ARROW_UP)
    {
        direction = -1;
    }
    else
    {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1)
        direction = 1;
    int current = last_match;
    int i;
    for (i = 0; i < E.num_rows; i++)
    {
        current += direction;
        if (current == -1)
            current = E.num_rows - 1;
        else if (current == E.num_rows)
            current = 0;

        e_row *row = &E.row[current];
        char *match = strstr(row->render, query);
        if (match)
        {
            last_match = current;
            E.cy = current;
            E.cx = editor_row_rx_to_cx(row, match - row->render);
            E.row_off = E.num_rows;

            saved_hl_line = current;
            saved_hl = malloc(row->r_size);
            memcpy(saved_hl, row->hl, row->r_size);
            memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
            break;
        }
    }
}

void editor_find()
{
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.col_off;
    int saved_rowoff = E.row_off;

    char *query = editor_prompt("Search: %s (Use ESC/Arrows/Enter)",
                                editor_find_callback);

    if (query)
    {
        free(query);
    }
    else
    {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.col_off = saved_coloff;
        E.row_off = saved_rowoff;
    }
}

/********************************************************* append buffer *******************************************************/

// this struct is used to collect everything that is needed to be displayed in terminal window

void buffer_append(struct append_buffer *ab, const char *s, int length)
{
    char *new_c = realloc(ab->buffer, ab->length + length + 1);
    // making a new string with according to the length;

    if (new_c == NULL)
        return;
    memcpy(&new_c[ab->length], s, length);
    ab->buffer = new_c;
    ab->length += length;
}

void append_buffer_free(struct append_buffer *ab)
{
    free(ab->buffer);
}

/******************************************************** output ***************************************************************/

void editor_scroll()
{
    E.rx = 0;

    if (E.cy < E.num_rows)
    {
        E.rx = editor_row_cx_to_rx(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.row_off)
    {
        E.row_off = E.cy;
    }
    if (E.cy >= E.row_off + E.screen_rows)
    {
        E.row_off = E.cy - E.screen_rows + 1;
    }
    if (E.rx < E.col_off)
    {
        E.col_off = E.rx;
    }
    if (E.rx >= E.col_off + E.screen_cols)
    {
        E.col_off = E.rx - E.screen_cols + 1;
    }
}

void editor_draw_rows(struct append_buffer *ab)
{
    int y_cor;
    // E.screen_rows is set in get_windows_size function
    for (y_cor = 0; y_cor < E.screen_rows; y_cor++)
    {
        int file_row = y_cor + E.row_off;

        if (file_row >= E.num_rows)
        {

            if (E.num_rows == 0 && y_cor == E.screen_rows / 3)
            { // displaying welcome message 1/3 way down the terminal
                char welcome[80];
                int welcome_len = snprintf(welcome, sizeof(welcome), "File Editor -- version %s", EDITOR_VERSION);
                if (welcome_len > E.screen_cols)
                    welcome_len = E.screen_cols;

                // centering welcome message
                int padding = (E.screen_cols - welcome_len) / 2;

                if (padding)
                {
                    buffer_append(ab, "~", 1);
                    padding--;
                }

                while (padding--)
                    buffer_append(ab, " ", 1);

                buffer_append(ab, welcome, welcome_len);
            }
            else
            {
                buffer_append(ab, "~", 1);
            }
        }
        else
        {
            int length = E.row[file_row].r_size - E.col_off;
            if (length < 0)
                length = 0;
            if (length > E.screen_cols)
                length = E.screen_cols;
            char *c = &E.row[file_row].render[E.col_off];
            unsigned char *hl = &E.row[file_row].hl[E.col_off];
            int current_color = -1;
            int j;
            for (j = 0; j < length; j++)
            {
                if (hl[j] == HL_NORMAL)
                {
                    if (iscntrl(c[j]))
                    {
                        char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                        buffer_append(ab, "\x1b[7m", 4);
                        buffer_append(ab, &sym, 1);
                        buffer_append(ab, "\x1b[m", 3);
                        if (current_color != -1)
                        {
                            char buf[16];
                            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
                            buffer_append(ab, buf, clen);
                        }
                    }
                    else if (current_color != -1)
                    {
                        buffer_append(ab, "\x1b[39m", 5); // reset to default color
                        current_color = -1;
                    }
                    // buffer_append(ab, "\x1b[31m", 5); // setting foreground to red

                    buffer_append(ab, &c[j], 1);
                }
                else
                {
                    int color = editor_syntax_to_color(hl[j]);
                    if (color != current_color)
                    {
                        char buffer[16];
                        int c_length = snprintf(buffer, sizeof(buffer), "\x1b[%dm", color);
                        buffer_append(ab, buffer, c_length);
                    }
                    buffer_append(ab, &c[j], 1);
                }
            }
            buffer_append(ab, "\x1b[39m", 5);
        }

        buffer_append(ab, "\x1b[K", 3); // erase end of line
        buffer_append(ab, "\r\n", 2);
    }
}

void editor_draw_status_bar(struct append_buffer *ab)
{

    buffer_append(ab, "\x1b[7m", 4);
    //\x1b[7m - reverses terminal colors
    char status[80], r_status[80];

    int length = snprintf(status, sizeof(status), "%.20s - %d lines %s", E.filename ? E.filename : "[No Name]", E.num_rows, E.dirty ? "(modified)" : "");
    int r_length = snprintf(r_status, sizeof(r_status), "%s | %d%d-", E.syntax ? E.syntax->file_type : "no ft", E.cy + 1, E.num_rows);

    if (length > E.screen_cols)
        length = E.screen_cols;

    buffer_append(ab, status, length);
    // basically, here to buffer is saved name  of the file if it exists and number of rows in the file
    // that information comes form snprintf above

    while (length < E.screen_cols)
    {
        if (E.screen_cols - length == r_length)
        {
            buffer_append(ab, r_status, r_length); // here right status is saved to buffer(r_status);
            break;
        }
        else
        {

            buffer_append(ab, " ", 1); // saves to buffer spaces until second status string(r_string) against right edge of the screen
            length++;
        }
    }

    buffer_append(ab, "\x1b[m", 3); // revers color
    buffer_append(ab, "\r\n", 2);   // adds new line
}

void editor_draw_message_bar(struct append_buffer *ab)
{
    buffer_append(ab, "\x1b[K", 3);
    //\x1b[K erases from the current cursor postion to the end of the current line.

    int msg_length = strlen(E.status_message);

    if (msg_length > E.screen_cols)
        msg_length = E.screen_cols;
    if (msg_length && time(NULL) - E.status_message_time < 5)
        buffer_append(ab, E.status_message, msg_length); // saved status message to buffer
}

void editor_refresh_screen() // this function runs in a loop
{
    editor_scroll();

    struct append_buffer ab = APPEND_BUFFER;
    // initialize append_buffer struct that holds buffer string  and length;

    buffer_append(&ab, "\x1b[?25l", 6); // hide the cursor
                                        // buffer_append(&ab, "\x1b[2J", 4);   // erases screen and moves the cursor to home
    buffer_append(&ab, "\x1b[H", 3);    // moves cursor to home

    editor_draw_rows(&ab);       // saves tildes in a buffer.
    editor_draw_status_bar(&ab); // saves info about file name and number of lines in file as well as where the current cursor is
    editor_draw_message_bar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.row_off) + 1, (E.rx - E.col_off) + 1);
    //\x1b[%d;%dH -  moves cursor to home(top left corner)

    buffer_append(&ab, buf, strlen(buf));
    buffer_append(&ab, "\x1b[?25h", 6); // show the cursor

    write(STDOUT_FILENO, ab.buffer, ab.length); // write tildes to the terminal on the left side like in VIM or text from the file

    append_buffer_free(&ab); // free allocated memory for buffer
}

void editor_set_status_message(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.status_message, sizeof(E.status_message), fmt, ap);
    va_end(ap);
    E.status_message_time = time(NULL);

    // here we save variadic arguments to status message and message time
}

/************************************************************** input ********************************************************************/

char *editor_prompt(char *prompt, void (*callback)(char *, int))
{
    size_t buffer_size = 128;
    char *buffer = malloc(buffer_size);

    size_t buffer_length = 0;
    buffer[0] = '\0';

    while (1)
    {
        editor_set_status_message(prompt, buffer);
        editor_refresh_screen();

        int c = editor_read_key();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE)
        {
            if (buffer_length != 0)
                buffer[--buffer_length] = '\0';
        }
        else if (c == '\x1b')
        {
            editor_set_status_message("");
            if (callback)
                callback(buffer, c);
            free(buffer);
            return NULL;
        }
        else if (c == '\r')
        {
            if (buffer_length != 0)
            {
                editor_set_status_message("");
                if (callback)
                    callback(buffer, c);
                return buffer;
            }
        }
        else if (!iscntrl(c) && c < 128)
        {
            if (buffer_length == buffer_size - 1)
            {
                buffer_size *= 2;
                buffer = realloc(buffer, buffer_size);
            }
            buffer[buffer_length++] = c;
            buffer[buffer_length] = '\0';
        }
        if (callback)
            callback(buffer, c);
    }
}

void editor_move_cursor(int key)
{
    e_row *row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];

    switch (key)
    {
    case ARROW_LEFT:
        if (E.cx != 0)
        {
            E.cx--;
        }
        else if (E.cy > 0)
        {
            E.cy--;
            E.cx = E.row[E.cy].size;
        }
        break;
    case ARROW_RIGHT:
        if (row && E.cx < row->size)
        {
            E.cx++;
        }
        else if (row && E.cx == row->size)
        {
            E.cy++;
            E.cx = 0;
        }
        break;
    case ARROW_UP:
        if (E.cy != 0)
        {
            E.cy--;
        }
        break;
    case ARROW_DOWN:
        if (E.cy < E.num_rows)
        {
            E.cy++;
        }
        break;
    }

    row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];
    int row_length = row ? row->size : 0;
    if (E.cx > row_length)
    {
        E.cx = row_length;
    }

    return;
}

void editor_process_keypress() // this function runs in a loop
{
    static int quit_times = QUIT_TIMES;

    int c = editor_read_key(); // reading the keys from the terminal input

    switch (c)
    {

    case '\r':
        editor_insert_new_line();
        break;

    case CTRL_KEY('q'):
        if (E.dirty && quit_times > 0)
        {
            editor_set_status_message("WARNING!!! File has unsaved changes. Press Ctr + Q %d more times to quit", quit_times);
            quit_times--;
            return;
        }
        // if Ctrl+q then clear the screen, return cursor to home and exit.
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;

    case CTRL_KEY('s'):
        editor_save();
        exit(0);
        break;

    case HOME_KEY:
        E.cx = 0;
        break;
    case END_KEY:
        if (E.cy < E.num_rows)
        {
            E.cx = E.row[E.cy].size;
        }
        break;

    case CTRL_KEY('f'):
        editor_find();
        break;

    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL_KEY:
        if (c == DEL_KEY)
            editor_move_cursor(ARROW_RIGHT);
        editor_del_char();
        break;

    // handle pressing page up or page down
    case PAGE_UP:
    case PAGE_DOWN:
    {
        if (c == PAGE_UP)
        {
            E.cy = E.row_off;
        }
        else if (c == PAGE_DOWN)
        {
            E.cy = E.row_off + E.screen_rows - 1;
            if (E.cy > E.num_rows)
                E.cy = E.num_rows;
        }

        int times = E.screen_rows;
        while (times--)
            editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
    }
    break;
    // moving the cursor with arrow keys keys;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        editor_move_cursor(c);
        break;

    case CTRL_KEY('l'):
    case '\x1b':
        break;

    default:
        editor_insert_char(c);
        break;
    }

    quit_times = QUIT_TIMES;
}

/****************************************************** initialization function ********************************************/

void init_editor()
{
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.row_off = 0;
    E.col_off = 0;
    E.num_rows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.status_message[0] = '\0';
    E.status_message_time = 0;
    E.syntax = NULL;

    if (get_windows_size(&E.screen_rows, &E.screen_cols) == -1)
        error_handler("get_windows_size");

    E.screen_rows -= 2;
}
