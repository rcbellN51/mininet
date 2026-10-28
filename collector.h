#ifndef MININET_COLLECTOR_H
#define MININET_COLLECTOR_H

#include <stdint.h>
#include <time.h>

#define NET_NAME_MAX 32
#define NET_MAX_INTERFACES 32

struct net_sample {
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
};

struct net_rate {
    double rx_kib;
    double tx_kib;
    double rx_pps;
    double tx_pps;
};

struct net_interface {
    char name[NET_NAME_MAX];
};

/* Enumerate unique network interfaces. */
int net_list_interfaces(struct net_interface *list, int max);

/* Read cumulative interface statistics. */
int net_read(const char *name, struct net_sample *sample);

/* Calculate throughput between two samples. */
int net_calculate_rate(
    const struct net_sample *previous,
    const struct net_sample *current,
    const struct timespec *previous_time,
    const struct timespec *current_time,
    struct net_rate *rate
);

#endif
