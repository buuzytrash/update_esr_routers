#include "ui.h"

#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

static WINDOW *win_list, *win_detail, *win_log;
static int max_rows, max_cols, list_w, detail_h;

#define ICON_OK '+'
#define ICON_WARN '!'
#define ICON_ERR 'X'
#define ICON_PEND '?'

void ui_init(void) {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_RED, -1);
    init_pair(4, COLOR_CYAN, -1);
    getmaxyx(stdscr, max_rows, max_cols);
    list_w = max_cols / 3;
    detail_h = max_rows / 3;
    ui_draw_layout();
}

void ui_draw_layout(void) {
    win_list = newwin(max_rows, list_w, 0, 0);
    box(win_list, 0, 0);
    mvwprintw(win_list, 0, 2, " Routers ");
    wrefresh(win_list);

    win_detail = newwin(detail_h, max_cols - list_w, 0, list_w);
    box(win_detail, 0, 0);
    mvwprintw(win_detail, 0, 2, " Details ");
    wrefresh(win_detail);

    win_log = newwin(max_rows - detail_h, max_cols - list_w, detail_h, list_w);
    box(win_log, 0, 0);
    mvwprintw(win_log, 0, 2, " Log ");
    // Добавляем строку выхода
    mvwprintw(win_log, getmaxy(win_log) - 1, 2, "Press 'q' to quit");
    scrollok(win_log, TRUE);
    wrefresh(win_log);
}

int ui_select_method(const char **methods, size_t count) {
    int highlight = 0, ch;
    while (1) {
        werase(win_list);
        box(win_list, 0, 0);
        mvwprintw(win_list, 0, 2, " Select Method ");
        for (size_t i = 0; i < count; ++i) {
            if ((int)i == highlight) wattron(win_list, A_REVERSE);
            mvwprintw(win_list, (int)i + 2, 2, "%s", methods[i]);
            if ((int)i == highlight) wattroff(win_list, A_REVERSE);
        }
        wrefresh(win_list);
        ch = getch();
        if (ch == 'q' || ch == 'Q') exit(0);
        if (ch == KEY_UP)
            highlight = (highlight - 1 + count) % count;
        else if (ch == KEY_DOWN)
            highlight = (highlight + 1) % count;
        else if (ch == '\n')
            break;
    }
    return highlight;
}

int *ui_select_routers(const char **routers, size_t count, size_t *sel_count) {
    int *sel = calloc(count, sizeof(int));
    int highlight = 0, ch;
    while (1) {
        werase(win_list);
        box(win_list, 0, 0);
        mvwprintw(win_list, 0, 2, " Select Routers ");
        for (size_t i = 0; i < count; ++i) {
            if ((int)i == highlight) wattron(win_list, A_REVERSE);
            mvwprintw(win_list, (int)i + 2, 2, "[%c] %s", sel[i] ? 'x' : ' ',
                      routers[i]);
            if ((int)i == highlight) wattroff(win_list, A_REVERSE);
        }
        wrefresh(win_list);
        ch = getch();
        if (ch == 'q' || ch == 'Q') exit(0);
        if (ch == KEY_UP)
            highlight = (highlight - 1 + count) % count;
        else if (ch == KEY_DOWN)
            highlight = (highlight + 1) % count;
        else if (ch == ' ')
            sel[highlight] = !sel[highlight];
        else if (ch == '\n')
            break;
    }
    *sel_count = 0;
    for (size_t i = 0; i < count; ++i)
        if (sel[i]) (*sel_count)++;
    int *idx = malloc((*sel_count) * sizeof(int));
    size_t j = 0;
    for (size_t i = 0; i < count; ++i) {
        if (sel[i]) idx[j++] = (int)i;
    }
    free(sel);
    return idx;
}

void ui_update_router_list(const char **routers, size_t count,
                           const char **status) {
    werase(win_list);
    box(win_list, 0, 0);
    mvwprintw(win_list, 0, 2, " Routers ");
    for (size_t i = 0; i < count; ++i) {
        char icon = ICON_PEND;
        int color = 4;
        if (strcmp(status[i], "OK") == 0) {
            icon = ICON_OK;
            color = 1;
        } else if (strcmp(status[i], "WARN") == 0) {
            icon = ICON_WARN;
            color = 2;
        } else if (strcmp(status[i], "ERROR") == 0) {
            icon = ICON_ERR;
            color = 3;
        }
        wattron(win_list, COLOR_PAIR(color));
        mvwprintw(win_list, (int)i + 1, 2, "%c %s", icon, routers[i]);
        wattroff(win_list, COLOR_PAIR(color));
    }
    wrefresh(win_list);
}

void ui_update_detail(size_t index, int percent, int seconds_left) {
    werase(win_detail);
    box(win_detail, 0, 0);
    mvwprintw(win_detail, 0, 2, " Detail %zu ", index + 1);
    int bar_w = max_cols - list_w - 4;
    int fill = (percent * bar_w) / 100;
    mvwprintw(win_detail, 2, 2, "Progress:");
    wattron(win_detail, A_REVERSE);
    mvwhline(win_detail, 2, 12, ' ', fill);
    wattroff(win_detail, A_REVERSE);
    mvwprintw(win_detail, 2, 12 + bar_w + 1, "%3d%%", percent);
    mvwprintw(win_detail, 4, 2, "Time left: %d sec", seconds_left);
    wrefresh(win_detail);
}

void ui_log_append(const char *line) {
    waddstr(win_log, line);
    waddch(win_log, '\n');
    wrefresh(win_log);
}

void ui_deinit(void) {
    delwin(win_list);
    delwin(win_detail);
    delwin(win_log);
    endwin();
}
