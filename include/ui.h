#ifndef UI_H
#define UI_H

#include <stddef.h>

/** Инициализация ncurses UI и создание окон */
void ui_init(void);
/** Отрисовка базового макета (список, детали, лог) */
void ui_draw_layout(void);

/** Меню выбора метода обновления (tftp/scp) */
int ui_select_method(const char **methods, size_t count);
/** Меню выбора образа прошивки */
int ui_select_firmware(const char **images, size_t count);
/** Меню чек-листа роутеров */
int *ui_select_routers(const char **routers, size_t count, size_t *sel_count);

/** Обновление списка роутеров с иконками и цветом */
void ui_update_router_list(const char **routers, size_t count,
                           const char **status);
/** Обновление панели «детали» — прогресс и таймер */
void ui_update_detail(size_t index, int percent, int seconds_left);
/** Добавить строку в лог-панель */
void ui_log_append(const char *line);

/** Завершение работы UI */
void ui_deinit(void);

#endif  // UI_H
