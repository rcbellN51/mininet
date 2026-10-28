#include "ui.h"

#include <curses.h>
#include <inttypes.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#define COLOR_MINIX_GREEN   22
#define COLOR_MINIX_BLUE    24
#define COLOR_MINIX_YELLOW  221
#define COLOR_MINIX_RED     196

#define PAIR_RX       1
#define PAIR_TX       2
#define PAIR_HEADING  3
#define PAIR_ERROR    4
#define PAIR_RX_GRAPH 5
#define PAIR_TX_GRAPH 6

static void
draw_meter(int row, int col, int width, double value,
           double maximum, int pair)
{
    int i, filled;

    if (width <= 0 || maximum <= 0.0)
        return;

    filled = (int)((value / maximum) * width + 0.5);

    if (filled < 0)
        filled = 0;
    if (filled > width)
        filled = width;

    for (i = 0; i < filled; i++) {
        attron(COLOR_PAIR(pair));
        mvaddch(row, col + i, ' ');
        attroff(COLOR_PAIR(pair));
    }
}

static double
history_visible_scale(const struct net_history *history,
                      int width, int is_rx)
{
    int available, i, index;
    double peak = 1.0;
    double value;

    available = history->count;

    if (available > width)
        available = width;

    for (i = 0; i < available; i++) {
        index = (history->next - available + i +
                 HISTORY_SIZE) % HISTORY_SIZE;

        value = is_rx
              ? history->samples[index].rx_kib
              : history->samples[index].tx_kib;

        if (value > peak)
            peak = value;
    }

    return peak;
}

static void
draw_history_sparkline(int row, int col, int width,
                       const struct net_history *history,
                       int is_rx, int pair, double scale)
{
    static const wchar_t levels[] =
        L"\u2581\u2582\u2583\u2584\u2585\u2586\u2587\u2588";

    int available, x, index, level;
    double value;

    if (width <= 0 || scale <= 0.0)
        return;

    available = history->count;

    if (available > width)
        available = width;

    for (x = 0; x < available; x++) {
        index = (history->next - available + x +
                 HISTORY_SIZE) % HISTORY_SIZE;

        value = is_rx
              ? history->samples[index].rx_kib
              : history->samples[index].tx_kib;

        if (value <= 0.0) {
            mvaddch(row, col + x, ' ');
            continue;
        }

        level = (int)((value / scale) * 7.0 + 0.5);

        if (level < 0)
            level = 0;
        if (level > 7)
            level = 7;

        attron(COLOR_PAIR(pair));
        mvaddnwstr(row, col + x, &levels[level], 1);
        attroff(COLOR_PAIR(pair));
    }
}

int
ui_init(void)
{
    if (setlocale(LC_CTYPE, "en_US.UTF-8") == NULL)
        return -1;

    if (initscr() == NULL)
        return -1;

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);

    if (has_colors()) {
        start_color();

        if (COLORS >= 256) {
            use_default_colors();

            init_pair(PAIR_RX, 15, COLOR_MINIX_BLUE);
            init_pair(PAIR_TX, 15, COLOR_MINIX_GREEN);
            init_pair(PAIR_HEADING, COLOR_MINIX_YELLOW, -1);
            init_pair(PAIR_ERROR, COLOR_MINIX_RED, -1);
            init_pair(PAIR_RX_GRAPH, COLOR_MINIX_BLUE, -1);
            init_pair(PAIR_TX_GRAPH, COLOR_MINIX_GREEN, -1);
        }
    }

    return 0;
}

void
ui_shutdown(void)
{
    endwin();
}

void
ui_draw(const char *interface,
        const struct net_sample *sample,
        const struct net_rate *rate,
        const struct net_history *history,
        double interval)
{
    int rows, cols, i, meter_width;
    double scale;

    getmaxyx(stdscr, rows, cols);

    erase();

    if (rows < 24 || cols < 50) {
        mvprintw(0, 0, "Terminal too small (minimum 50x24)");
        refresh();
        return;
    }

    attron(COLOR_PAIR(PAIR_HEADING) | A_BOLD);
    mvprintw(0, 2, "mininet 0.1");
    attroff(COLOR_PAIR(PAIR_HEADING) | A_BOLD);

    char interval_text[32];

    snprintf(interval_text, sizeof(interval_text),
             "Interval: %.1fs", interval);

    mvprintw(0, cols - 2 - (int)strlen(interval_text),
             "%s", interval_text);

    for (i = 0; i < cols; i++)
        mvaddch(1, i, ACS_HLINE);

    mvprintw(3, 2, "Interface: %s", interface);

    attron(COLOR_PAIR(PAIR_RX) | A_BOLD);
    mvprintw(5, 2, " RX ");
    attroff(COLOR_PAIR(PAIR_RX) | A_BOLD);

    attron(COLOR_PAIR(PAIR_TX) | A_BOLD);
    mvprintw(6, 2, " TX ");
    attroff(COLOR_PAIR(PAIR_TX) | A_BOLD);

    mvprintw(5, 8, "%9.2f KiB/s", rate->rx_kib);
    mvprintw(6, 8, "%9.2f KiB/s", rate->tx_kib);

    mvprintw(5, 30, "%8.1f packets/s", rate->rx_pps);
    mvprintw(6, 30, "%8.1f packets/s", rate->tx_pps);

    scale = history_get_scale(history);

    meter_width = cols - 8;

    if (meter_width > 70)
        meter_width = 70;

    mvprintw(8, 2, "RX");
    draw_meter(8, 6, meter_width,
               rate->rx_kib, scale, PAIR_RX);

    mvprintw(9, 2, "TX");
    draw_meter(9, 6, meter_width,
               rate->tx_kib, scale, PAIR_TX);

    mvprintw(11, 2, "Scale: %.2f KiB/s", scale);

    /*
     * Compact throughput history.
     * RX and TX use independent visible-history scales.
     */
    {
        int graph_width = cols - 18;
        int scale_col;
        double rx_scale, tx_scale;

        if (graph_width > 70)
            graph_width = 70;

        rx_scale = history_visible_scale(history, graph_width, 1);
        tx_scale = history_visible_scale(history, graph_width, 0);

        scale_col = 6 + graph_width + 2;

        attron(COLOR_PAIR(PAIR_HEADING) | A_BOLD);
        mvprintw(13, 2, "Throughput history");
        attroff(COLOR_PAIR(PAIR_HEADING) | A_BOLD);

        mvprintw(15, 2, "RX");
        draw_history_sparkline(15, 6, graph_width,
                               history, 1, PAIR_RX_GRAPH, rx_scale);

        mvprintw(16, 2, "TX");
        draw_history_sparkline(16, 6, graph_width,
                               history, 0, PAIR_TX_GRAPH, tx_scale);

        mvprintw(15, scale_col, "%.0fK", rx_scale);
        mvprintw(16, scale_col, "%.0fK", tx_scale);
    }

    mvprintw(18, 2, "Received: %10.2f MiB",
             (double)sample->rx_bytes / 1048576.0);

    mvprintw(19, 2, "Sent:     %10.2f MiB",
             (double)sample->tx_bytes / 1048576.0);

    mvprintw(18, 30, "RX packets: %10" PRIu64,
             sample->rx_packets);

    mvprintw(19, 30, "TX packets: %10" PRIu64,
             sample->tx_packets);

    mvprintw(18, 56, "RX errors: %" PRIu64,
             sample->rx_errors);

    mvprintw(19, 56, "TX errors: %" PRIu64,
             sample->tx_errors);

    for (i = 0; i < cols; i++)
        mvaddch(rows - 2, i, ACS_HLINE);

    mvprintw(rows - 1, 2,
             "q Quit   Tab Interface   +/- Interval");

    refresh();
}
