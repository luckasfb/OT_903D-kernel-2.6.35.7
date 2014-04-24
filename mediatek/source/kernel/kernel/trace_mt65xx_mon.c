
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>
#include <linux/ftrace.h>
#include <trace/events/mt65xx_mon_trace.h>

#include <../../kernel/kernel/trace/trace.h>
#include "mach/mt65xx_mon.h"

static struct trace_array *mt65xx_mon_trace;
static int __read_mostly mt65xx_mon_enabled;
static int mt65xx_mon_ref;
static DEFINE_MUTEX(mt65xx_mon_mutex);
static int mt65xx_mon_stopped = 0;

void
tracing_mt65xx_mon_function(struct trace_array *tr,
                           struct task_struct *prev,
                           struct task_struct *next,
                           unsigned long flags, int pc)
{
#if 0
    struct ftrace_event_call *call = &event_mt65xx_mon;
#endif
    struct ring_buffer *buffer = tr->buffer;
    struct ring_buffer_event *event;
    struct mt65xx_mon_entry *entry;

    event = trace_buffer_lock_reserve(buffer, TRACE_MT65XX_MON_TYPE,
                      sizeof(*entry), flags, pc);
    if (!event)
        return;

    entry = ring_buffer_event_data(event);

    mt65xx_mon_disable();
    entry->log = mt65xx_mon_log();
    mt65xx_mon_enable();

#if 0
    if (!filter_check_discard(call, entry, buffer, event))
#endif
        trace_buffer_unlock_commit(buffer, event, flags, pc);
}

static void
probe_mt65xx_mon_tracepoint(void *ignore, struct task_struct *prev,
                            struct task_struct *next)
{
    struct trace_array_cpu *data;
    unsigned long flags;
    int cpu;
    int pc;

    if (unlikely(!mt65xx_mon_ref))
        return;

    if (!mt65xx_mon_enabled || mt65xx_mon_stopped)
        return;

    tracing_record_cmdline(prev);

    pc = preempt_count();
    local_irq_save(flags);
    cpu = raw_smp_processor_id();
    data = mt65xx_mon_trace->data[cpu];

    if (likely(!atomic_read(&data->disabled)))
        tracing_mt65xx_mon_function(mt65xx_mon_trace, prev, next, flags, pc);

    local_irq_restore(flags);
}

static int tracing_mt65xx_mon_register(void)
{
    int ret;

    ret = register_trace_mt65xx_mon_sched_switch(probe_mt65xx_mon_tracepoint, NULL);
    if (ret) {
        pr_info("sched trace: Couldn't activate tracepoint"
            " probe to mt65xx monitor\n");
    }
    return ret;
}

static void tracing_mt65xx_mon_unregister(void)
{
    unregister_trace_mt65xx_mon_sched_switch(probe_mt65xx_mon_tracepoint, NULL);
}

static void tracing_start_mt65xx_mon(void)
{
    mutex_lock(&mt65xx_mon_mutex);
    if (!(mt65xx_mon_ref++))
        tracing_mt65xx_mon_register();
    mutex_unlock(&mt65xx_mon_mutex);
}
static void tracing_stop_mt65xx_mon(void)
{
    mutex_lock(&mt65xx_mon_mutex);
    if (!(--mt65xx_mon_ref))
        tracing_mt65xx_mon_unregister();
    mutex_unlock(&mt65xx_mon_mutex);
}

void tracing_start_mt65xx_mon_record(void)
{
    if (unlikely(!mt65xx_mon_trace)) {
        WARN_ON(1);
        return;
    }

    tracing_start_mt65xx_mon();

    mutex_lock(&mt65xx_mon_mutex);
    mt65xx_mon_enabled++;
    mutex_unlock(&mt65xx_mon_mutex);
}

static int mt65xx_mon_trace_init(struct trace_array *tr)
{
    mt65xx_mon_init();

    mt65xx_mon_trace = tr;
    tracing_reset_online_cpus(tr);
    tracing_start_mt65xx_mon_record();

    return 0;
}

static void mt65xx_mon_trace_reset(struct trace_array *tr)
{
    if (mt65xx_mon_ref) {
        mutex_lock(&mt65xx_mon_mutex);
        mt65xx_mon_enabled--;
        WARN_ON(mt65xx_mon_enabled < 0);
        mutex_unlock(&mt65xx_mon_mutex);

        tracing_stop_mt65xx_mon();
    }
}

static void mt65xx_mon_trace_start(struct trace_array *tr)
{
    mt65xx_mon_stopped = 0;
}

static void mt65xx_mon_trace_stop(struct trace_array *tr)
{
    mt65xx_mon_stopped = 1;

    mt65xx_mon_deinit();
}

static struct tracer mt65xx_mon_tracer __read_mostly =
{
    .name        = "mt65xx monitor",
    .init        = mt65xx_mon_trace_init,
    .reset        = mt65xx_mon_trace_reset,
    .start        = mt65xx_mon_trace_start,
    .stop        = mt65xx_mon_trace_stop,
    .wait_pipe    = poll_wait_pipe,
#ifdef CONFIG_FTRACE_SELFTEST
    .selftest    = trace_selftest_startup_mtk,
#endif
};

__init static int init_mt65xx_mon_trace(void)
{
    return register_tracer(&mt65xx_mon_tracer);
}

device_initcall(init_mt65xx_mon_trace);

