#ifndef MININET_UI_H
#define MININET_UI_H

#include "collector.h"
#include "history.h"

int ui_init(void);
void ui_shutdown(void);

void ui_draw(const char *interface,
             const struct net_sample *sample,
             const struct net_rate *rate,
             const struct net_history *history,
             double interval);

#endif
