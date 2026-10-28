#include "history.h"

#include <string.h>

void
history_init(struct net_history *history)
{
    memset(history, 0, sizeof(*history));
    history->scale = 64.0;
}

void
history_reset(struct net_history *history)
{
    history_init(history);
}

void
history_add(struct net_history *history,
            const struct net_rate *rate,
            const struct timespec *now)
{
    double elapsed;
    double decay;
    double peak;

    history->samples[history->next] = *rate;

    history->next =
        (history->next + 1) % HISTORY_SIZE;

    if (history->count < HISTORY_SIZE)
        history->count++;

    elapsed = 0.0;

    if (history->previous_time.tv_sec != 0 ||
        history->previous_time.tv_nsec != 0) {

        elapsed =
            (double)(now->tv_sec -
                     history->previous_time.tv_sec) +
            (double)(now->tv_nsec -
                     history->previous_time.tv_nsec) /
            1000000000.0;
    }

    history->previous_time = *now;

    if (elapsed < 0.0)
        elapsed = 0.0;

    /* Approximately 5% decay per second. */
    decay = 1.0 - 0.05 * elapsed;

    if (decay < 0.0)
        decay = 0.0;

    history->scale *= decay;

    if (history->scale < 1.0)
        history->scale = 1.0;

    peak = rate->rx_kib > rate->tx_kib
         ? rate->rx_kib
         : rate->tx_kib;

    if (peak > history->scale)
        history->scale = peak;
}

double
history_get_scale(const struct net_history *history)
{
    return history->scale;
}
