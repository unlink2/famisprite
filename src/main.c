/**
 * CLI interface for famisprite.
 * Sample implementation of library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "include/famisprite.h"
#include "include/utility.h"

#define my_malloc(x) malloc(x)
#define my_free(x) free(x)

#define MAX_BUFFER_SIZE 128

#define PIXEL_W 2
#define PIXEL_H 1

enum FAMI_COLOR_PAIRS {
    DEFAULT,
    C0PAIR,
    C1PAIR,
    C2PAIR,
    C3PAIR,
    CURSORPAIR,
};

/**
 * Application state
 */

typedef struct settings {
    uint8_t running; // boolean

    char *input_path;
    char *output_path;

    char current[MAX_BUFFER_SIZE]; // current buffer on screen
    char *buffer; // loaded file
    size_t buffer_len;

    // cursor x and y location for editing
    short cursor_x;
    short cursor_y;

    fami_color_index color; // current color
    uint32_t offset; // current buffer offset
    char long_sprite;
    uint32_t current_buffer;
    char color_on;
    char show_cursor;
} settings_t;

void init_settings(settings_t *settings) {
    settings->input_path = NULL;
    settings->output_path = "./out.bin";

    memset(settings->current, 0, MAX_BUFFER_SIZE);
    settings->buffer = NULL;

    settings->color = 0;
    settings->cursor_x = 0;
    settings->cursor_y = 0;

    settings->running = 1;
    settings->offset = 0;
    settings->long_sprite = 0;
    settings->current_buffer = MAX_BUFFER_SIZE/2;
    settings->color_on = 1;
    settings->show_cursor = 1;
}


void parse_arg_inputs(int argc, char **argv, settings_t *ps) {
    for (size_t i = 1; i < argc; i++) {
        if (is_arg(argv[i], "-h")) {
            printf("Usage: famisprite <infile> <outfile>\n\n");
            printf("Optional arguments:\n\n");
            printf("-o<number>\tStarting offset.");
            printf("-no-color\tDisables colors");
            exit(0);
        } else if (is_arg(argv[i], "-o")) {
            arg a = parse_arg(argv[i], "-o");
            ps->offset = strtol(a.value, NULL, 0);
        } else if (is_arg(argv[i], "-no-color")) {
            ps->color_on = 0;
        } else {
            // first set input then output then error
            if (!ps->input_path) {
                ps->input_path = argv[i];
            } else if (strcmp(ps->output_path, "./out.bin") == 0){
                ps->output_path = argv[i];
            } else {
                printf("Unknown argument: %s\n", argv[i]);
                exit(1);
            }
        }
    }
}

void end_curses() {
    endwin();
}

void read_input_file(settings_t *ps) {
    FILE *f = fopen(ps->input_path, "r");

    if (f == NULL) {
        fprintf(stderr, "Unable to open input file: %s\n", ps->input_path);
        exit(1);
    }
    fseek(f, 0L, SEEK_END);
    size_t len = ftell(f);
    rewind(f);

    // get enough memory
    ps->buffer = my_malloc(sizeof(char) * len);

    // now read
    ps->buffer_len = fread(ps->buffer, 1, len, f);
    fclose(f);
    if (ps->buffer_len != len) {
        fprintf(stderr, "Input error while reading file: %s\n", ps->input_path);
        exit(1);
    }
}

void write_output_file(settings_t *ps) {
    FILE *f = fopen(ps->output_path, "w");
    if (f == NULL) {
        end_curses();
        fprintf(stderr, "Unable to open output file: %s\n", ps->output_path);
        exit(1);
    }
    fwrite(ps->buffer, 1, ps->buffer_len, f);
    fclose(f);
}

void init_curses(settings_t *ps) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (ps->color_on && has_colors()) {
        start_color();

        init_pair(C0PAIR, COLOR_BLACK, COLOR_BLACK);
        init_pair(C1PAIR, COLOR_RED, COLOR_RED);
        init_pair(C2PAIR, COLOR_GREEN, COLOR_GREEN);
        init_pair(C3PAIR, COLOR_BLUE, COLOR_BLUE);
        init_pair(CURSORPAIR, COLOR_MAGENTA, COLOR_MAGENTA);
    }
}

void init_windows(WINDOW **main_win, WINDOW **status_win, settings_t *ps) {
    if (*main_win) {
        delwin(*main_win);
    }
    if (*status_win) {
        delwin(*status_win);
    }

    int height = FAMI_TILE_SIZE * PIXEL_H + 2;
    int width = FAMI_TILE_SIZE * PIXEL_W + 2;

    if (ps->long_sprite) {
        height = height * 2 - 2;
        ps->current_buffer = MAX_BUFFER_SIZE;
    } else {
        ps->current_buffer = MAX_BUFFER_SIZE/2;
    }

    *main_win = newwin(height, width, 0, 0);

    *status_win = newwin(7, width, height, 0);
}

int coordinate_to_render_w(int x) {
    return x*PIXEL_W*2+1;
}

int coordinate_to_render_h(int y) {
    return y*PIXEL_H*2+1;
}

int color_to_char(char c) {
    switch(c) {
        case 0:
            return ACS_BULLET;
        case 1:
            return ACS_DIAMOND;
        case 2:
            return ACS_BLOCK;
        case 3:
            return ACS_CKBOARD;
    }
    return c;
}

void draw_pixel(WINDOW *win, int x, int y, int c) {
    x = coordinate_to_render_w(x);
    y = coordinate_to_render_h(y);
    for (int i = 0; i < PIXEL_W*2; i++) {
        for (int j = 0; j < PIXEL_H*2; j++) {
            mvwaddch(win, y+j, x+i, c);
        }
    }
}

void render_main(WINDOW *main_win, settings_t *ps) {
    werase(main_win);

    int x = 0;
    int y = 0;
    for (int i = 0; i < ps->current_buffer; i++) {
        wattron(main_win, COLOR_PAIR(ps->current[i]+1));
        draw_pixel(main_win, x, y, color_to_char(ps->current[i]));
        wattroff(main_win, COLOR_PAIR(ps->current[i]+1));
        x++;
        if (x >= FAMI_TILE_LEN) {
            y++;
            x = 0;
        }
    }

    if (ps->show_cursor) {
        wattron(main_win, COLOR_PAIR(CURSORPAIR));
        draw_pixel(main_win, ps->cursor_x, ps->cursor_y, '$');
        wattroff(main_win, COLOR_PAIR(CURSORPAIR));
    }

    box(main_win, 0 , 0);
}

void render_status(WINDOW *status_win, settings_t *ps) {
    werase(status_win);
    box(status_win, 0, 0);

    mvwprintw(status_win, 1, 1, "(Q)Quit ");
    wprintw(status_win, "(W)Write ");
    wprintw(status_win, "(1-4)Color");

    mvwprintw(status_win, 2, 1, "(Space)Paint ");
    wprintw(status_win, "(HJKL)Move ");
    wprintw(status_win, "(I)Mode");

    mvwprintw(status_win, 3, 1, "(<>)+/- ");
    wprintw(status_win, "(R)Reload ");
    wprintw(status_win, "(F)Fill ");

    mvwprintw(status_win, 4, 1, "(C)Hide Cursor");

    mvwprintw(status_win, 5, 1, "Color: %d ", ps->color);
    wprintw(status_win, "Offset: %X", ps->offset);
}

void gui(settings_t *ps) {
    WINDOW *main_win = NULL;
    WINDOW *status_win = NULL;
    init_windows(&main_win, &status_win, ps);

    // sanity check on offset
    if (ps->offset > ps->buffer_len-FAMI_TILE_SIZE) {
        ps->offset = 0;
    }

    // get first item
    unsigned int len = ps->current_buffer/4;
    fami_decode(ps->buffer+ps->offset, &len, (char*)ps->current);

    while (ps->running) {
        erase();
        render_main(main_win, ps);
        render_status(status_win, ps);

        refresh();
        wrefresh(main_win);
        wrefresh(status_win);
        int ch = getch();
        switch (ch) {
            case 'q':
                ps->running = 0;
                break;
            case '1':
                ps->color = 0;
                break;
            case '2':
                ps->color = 1;
                break;
            case '3':
                ps->color = 2;
                break;
            case '4':
                ps->color = 3;
                break;
            case ',':
            case '<':
                // put current offset back into file
                len = ps->current_buffer;
                fami_encode((char*)ps->current, &len, ps->buffer+ps->offset);
                ps->offset -= FAMI_TILE_SIZE * (ps->long_sprite+1);
                if (ps->offset > ps->buffer_len) {
                    ps->offset = ps->buffer_len-FAMI_TILE_SIZE;
                }
                len = ps->current_buffer/4;
                fami_decode(ps->buffer+ps->offset, &len, (char*)ps->current);
                break;
            case '.':
            case '>':
                // put current offset back into file
                len = ps->current_buffer;
                fami_encode((char*)ps->current, &len, ps->buffer+ps->offset);
                ps->offset += FAMI_TILE_SIZE * (ps->long_sprite+1);
                if (ps->offset > ps->buffer_len-FAMI_TILE_SIZE) {
                    ps->offset = 0;
                }
                len = ps->current_buffer/4;
                fami_decode(ps->buffer+ps->offset, &len, (char*)ps->current);
                break;
            case KEY_DOWN:
            case 'j':
                ps->cursor_y += 1;
                break;
            case KEY_UP:
            case 'k':
                ps->cursor_y -= 1;
                break;
            case KEY_LEFT:
            case 'h':
                ps->cursor_x -= 1;
                break;
            case KEY_RIGHT:
            case 'l':
                ps->cursor_x += 1;
                break;
            case ' ':
                fami_set_pixel(ps->current, ps->cursor_x, ps->cursor_y, ps->color);
                break;
            case 'f':
                // fill
                memset(ps->current, ps->color, MAX_BUFFER_SIZE);
                break;
            case 'r':
                // reload from memory
                len = ps->current_buffer/4;
                fami_decode(ps->buffer+ps->offset, &len, (char*)ps->current);
                break;
            case 'w':
                // write
                // put current offset back into file
                len = ps->current_buffer;
                fami_encode((char*)ps->current, &len, ps->buffer+ps->offset);
                write_output_file(ps);
                break;
            case 'i':
                ps->long_sprite = !ps->long_sprite;
                init_windows(&main_win, &status_win, ps);
                // reload from memory
                len = ps->current_buffer/4;
                fami_decode(ps->buffer+ps->offset, &len, (char*)ps->current);
                break;
            case 'c':
                ps->show_cursor = !ps->show_cursor;
                break;
        }

        // check cursor oob
        if (ps->cursor_x >= FAMI_TILE_LEN) {
            ps->cursor_x = 0;

        } else if (ps->cursor_x < 0) {
            ps->cursor_x = FAMI_TILE_LEN-1;
        }
        if (ps->cursor_y >= FAMI_TILE_LEN * (ps->long_sprite+1)) {
            ps->cursor_y = 0;

        } else if (ps->cursor_y < 0) {
            ps->cursor_y = FAMI_TILE_LEN-1;
        }
    }
    delwin(main_win);
    delwin(status_win);
}

int main(int argc, char **argv) {
    settings_t settings;
    init_settings(&settings);
    parse_arg_inputs(argc, argv, &settings);

    read_input_file(&settings);

    init_curses(&settings);
    gui(&settings);
    end_curses();

    // if file was opened free it now
    if (settings.buffer) {
        my_free(settings.buffer);
    }

    return 0;
}
