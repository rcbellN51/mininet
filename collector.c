#include "collector.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>

int
net_list_interfaces(struct net_interface *list, int max)
{
    struct ifaddrs *ifap, *ifa;
    int count = 0;
    int i;

    if (list == NULL || max <= 0)
        return -1;

    if (getifaddrs(&ifap) != 0)
        return -1;

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_name == NULL ||
            ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family != AF_LINK)
            continue;

        /* Avoid duplicate interface names. */
        for (i = 0; i < count; i++) {
            if (strcmp(list[i].name, ifa->ifa_name) == 0)
                break;
        }

        if (i < count)
            continue;

        if (count >= max)
            break;

        snprintf(list[count].name,
                 sizeof(list[count].name),
                 "%s", ifa->ifa_name);

        count++;
    }

    freeifaddrs(ifap);
    return count;
}

int
net_read(const char *name, struct net_sample *sample)
{
    struct ifaddrs *ifap, *ifa;
    struct if_data *data;
    int found = 0;

    if (name == NULL || sample == NULL)
        return -1;

    if (getifaddrs(&ifap) != 0)
        return -1;

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_name == NULL ||
            ifa->ifa_addr == NULL ||
            ifa->ifa_data == NULL)
            continue;

        if (ifa->ifa_addr->sa_family != AF_LINK)
            continue;

        if (strcmp(ifa->ifa_name, name) != 0)
            continue;

        data = (struct if_data *)ifa->ifa_data;

        sample->rx_bytes   = data->ifi_ibytes;
        sample->tx_bytes   = data->ifi_obytes;
        sample->rx_packets = data->ifi_ipackets;
        sample->tx_packets = data->ifi_opackets;
        sample->rx_errors  = data->ifi_ierrors;
        sample->tx_errors  = data->ifi_oerrors;

        found = 1;
        break;
    }

    freeifaddrs(ifap);

    return found ? 0 : -1;
}

int
net_calculate_rate(
    const struct net_sample *previous,
    const struct net_sample *current,
    const struct timespec *previous_time,
    const struct timespec *current_time,
    struct net_rate *rate)
{
    double elapsed;

    if (previous == NULL || current == NULL ||
        previous_time == NULL || current_time == NULL ||
        rate == NULL)
        return -1;

    elapsed =
        (double)(current_time->tv_sec - previous_time->tv_sec) +
        (double)(current_time->tv_nsec - previous_time->tv_nsec)
        / 1000000000.0;

    if (elapsed <= 0.0)
        return -1;

    /* Detect counter resets or interface restarts. */
    if (current->rx_bytes < previous->rx_bytes ||
        current->tx_bytes < previous->tx_bytes ||
        current->rx_packets < previous->rx_packets ||
        current->tx_packets < previous->tx_packets)
        return -1;

    rate->rx_kib =
        (double)(current->rx_bytes - previous->rx_bytes)
        / elapsed / 1024.0;

    rate->tx_kib =
        (double)(current->tx_bytes - previous->tx_bytes)
        / elapsed / 1024.0;

    rate->rx_pps =
        (double)(current->rx_packets - previous->rx_packets)
        / elapsed;

    rate->tx_pps =
        (double)(current->tx_packets - previous->tx_packets)
        / elapsed;

    return 0;
}
