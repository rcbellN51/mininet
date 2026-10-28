#ifndef MININET_HISTORY_H
#define MININET_HISTORY_H

#include "collector.h"

#define HISTORY_SIZE 300

struct net_history {
    struct net_rate samples[HISTORY_SIZE];
    int count;
    int next;
    double scale;
    struct timespec previous_time;
};

void history_init(struct net_history *history);

void history_reset(struct net_history *history);

void history_add(struct net_history *history,
                 const struct net_rate *rate,
                 const struct timespec *now);

double history_get_scale(const struct net_history *history);

#endif
