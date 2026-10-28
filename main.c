#include <stdio.h>
#include <string.h>
#include <time.h>
#include <curses.h>

#include "collector.h"
#include "history.h"
#include "ui.h"

#define DEFAULT_INTERVAL 2.0
#define MIN_INTERVAL 0.5
#define MAX_INTERVAL 10.0

static double
elapsed_seconds(const struct timespec *start,
                const struct timespec *end)
{
    return (double)(end->tv_sec - start->tv_sec) +
           (double)(end->tv_nsec - start->tv_nsec)
           / 1000000000.0;
}

int
main(void)
{
    struct net_interface interfaces[NET_MAX_INTERFACES];
    struct net_sample previous, current;
    struct net_rate rate = { 0.0, 0.0, 0.0, 0.0 };
    struct net_history history;
    struct timespec previous_time, current_time;
    double interval = DEFAULT_INTERVAL;
    int count, selected = 0;
    int ch;
    int running = 1;

    count = net_list_interfaces(
        interfaces, NET_MAX_INTERFACES);

    if (count <= 0) {
        fprintf(stderr, "No network interfaces found\n");
        return 1;
    }

    /* Prefer a non-loopback interface. */
    for (int i = 0; i < count; i++) {
        if (strncmp(interfaces[i].name, "lo", 2) != 0) {
            selected = i;
            break;
        }
    }

    if (net_read(interfaces[selected].name, &previous) != 0 ||
        clock_gettime(CLOCK_MONOTONIC, &previous_time) != 0) {
        fprintf(stderr, "Initial sample failed\n");
        return 1;
    }

    current = previous;

    history_init(&history);

    if (ui_init() != 0) {
        fprintf(stderr, "Cannot initialize curses\n");
        return 1;
    }

    while (running) {
        if (clock_gettime(CLOCK_MONOTONIC, &current_time) != 0)
            break;

        if (elapsed_seconds(&previous_time, &current_time)
            >= interval) {

            if (net_read(interfaces[selected].name, &current) == 0) {

                if (net_calculate_rate(
                        &previous, &current,
                        &previous_time, &current_time,
                        &rate) != 0) {
                    memset(&rate, 0, sizeof(rate));
                }

                history_add(&history, &rate, &current_time);

                previous = current;
                previous_time = current_time;
            }
        }

        ui_draw(interfaces[selected].name,
                &current, &rate, &history, interval);

        ch = getch();

        switch (ch) {
        case 'q':
        case 'Q':
            running = 0;
            break;

        case '\t':
            selected = (selected + 1) % count;

            if (net_read(interfaces[selected].name, &previous) == 0 &&
                clock_gettime(CLOCK_MONOTONIC, &previous_time) == 0) {
                current = previous;
                memset(&rate, 0, sizeof(rate));
                history_reset(&history);
            }
            break;

        case '+':
        case '=':
            if (interval < MAX_INTERVAL)
                interval += 0.5;
            break;

        case '-':
            if (interval > MIN_INTERVAL)
                interval -= 0.5;
            break;

        default:
            break;
        }
    }

    ui_shutdown();
    return 0;
}
