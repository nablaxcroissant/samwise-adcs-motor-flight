#include "watchdog.h"

watchdog_t watchdog_mk()
{
    return (watchdog_t){
        .pin = SAMWISE_WATCHDOG_FEED_PIN,
        .last_transition = nil_time,
        .set = false,
        .us_low = 1000 * 1000, // 5 second
        .us_high = 200 * 1000, // 200ms
    };
}

void watchdog_init(watchdog_t *wd)
{
    gpio_init(wd->pin);
    gpio_set_dir(wd->pin, GPIO_OUT);

    gpio_put(wd->pin, 0);
    wd->last_transition = get_absolute_time();
    wd->set = false;
}

void watchdog_feed(watchdog_t *wd)
{
    uint64_t delta =
        absolute_time_diff_us(get_absolute_time(), wd->last_transition);
    if (wd->set && delta > wd->us_high)
    {
        gpio_put(wd->pin, 0);
        wd->set = false;
        wd->last_transition = get_absolute_time();
    }
    else if (!wd->set && delta > wd->us_low)
    {
        gpio_put(wd->pin, 1);
        wd->set = true;
        wd->last_transition = get_absolute_time();
    }
}
