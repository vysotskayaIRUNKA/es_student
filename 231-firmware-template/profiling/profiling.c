#include "profiling.h"
#include "pico/stdlib.h"

typedef struct
{
    const char *name;
    uint64_t start_us;
    uint32_t count;
    uint32_t sum_us;
    uint32_t max_us;
} profiling_stopwatch_t;

static profiling_stopwatch_t stopwatches[PROFILING_MAX_STOPWATCHES];
static uint32_t stopwatch_count = 0;

void profiling_init(void)
{
    stopwatch_count = 0;
    profiling_reset();
}

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    stopwatches[id].name = name;
    if (id >= stopwatch_count) {
        stopwatch_count = id + 1;
    }
}

void profiling_start(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    stopwatches[id].start_us = time_us_64();
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES) return;
    uint64_t now = time_us_64();
    uint32_t dt = (uint32_t)(now - stopwatches[id].start_us);

    stopwatches[id].sum_us += dt;
    stopwatches[id].count++;
    if (dt > stopwatches[id].max_us) {
        stopwatches[id].max_us = dt;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= stopwatch_count || stopwatches[id].name == NULL) {
        return false;
    }

    result->name = stopwatches[id].name;
    result->count = stopwatches[id].count;
    result->sum_us = stopwatches[id].sum_us;
    result->max_us = stopwatches[id].max_us;

    return true;
}

void profiling_reset(void)
{
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++) {
        stopwatches[i].count = 0;
        stopwatches[i].sum_us = 0;
        stopwatches[i].max_us = 0;
    }
}
