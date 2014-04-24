

#include <linux/time.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/rcupdate.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/bitops.h>
#include <linux/smp_lock.h>
#include <linux/stat.h>
#include <linux/cpu.h>
#include <linux/idle.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <asm/atomic.h>
#include <asm/local.h>

#include "ltt-tracer.h"
#include "ltt-relay.h"
#include "ltt-relay-lockless.h"

#if 0
#define printk_dbg(fmt, args...) printk(fmt, args)
#else
#define printk_dbg(fmt, args...)
#endif

struct ltt_reserve_switch_offsets {
	long begin, end, old;
	long begin_switch, end_switch_current, end_switch_old;
	size_t before_hdr_pad, size;
};

static
void ltt_force_switch(struct ltt_chanbuf *buf, enum force_switch_mode mode);

static
void ltt_relay_print_buffer_errors(struct ltt_chan *chan, unsigned int cpu);

static const struct file_operations ltt_file_operations;

static
void ltt_buffer_begin(struct ltt_chanbuf *buf, u64 tsc, unsigned int subbuf_idx)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);
	struct ltt_subbuffer_header *header =
		(struct ltt_subbuffer_header *)
			ltt_relay_offset_address(&buf->a,
				subbuf_idx * chan->a.sb_size);

	header->cycle_count_begin = tsc;
	header->data_size = 0xFFFFFFFF; /* for debugging */
	ltt_write_trace_header(chan->a.trace, header);
}

static
void ltt_buffer_end(struct ltt_chanbuf *buf, u64 tsc, unsigned int offset,
		    unsigned int subbuf_idx)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);
	struct ltt_subbuffer_header *header =
		(struct ltt_subbuffer_header *)
			ltt_relay_offset_address(&buf->a,
				subbuf_idx * chan->a.sb_size);
	u32 data_size = SUBBUF_OFFSET(offset - 1, chan) + 1;

	header->data_size = data_size;
	header->sb_size = PAGE_ALIGN(data_size);
	header->cycle_count_end = tsc;
	header->events_lost = local_read(&buf->events_lost);
	header->subbuf_corrupt = local_read(&buf->corrupted_subbuffers);
}

void ltt_chanbuf_free(struct ltt_chanbuf *buf)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);

	ltt_relay_print_buffer_errors(chan, buf->a.cpu);
#ifdef LTT_VMCORE
	kfree(buf->commit_seq);
#endif
	kfree(buf->commit_count);

	ltt_chanbuf_alloc_free(&buf->a);
}

int ltt_chanbuf_create(struct ltt_chanbuf *buf, struct ltt_chan_alloc *chana,
		       int cpu)
{
	struct ltt_chan *chan = container_of(chana, struct ltt_chan, a);
	struct ltt_trace *trace = chana->trace;
	unsigned int j, n_sb;
	int ret;

	/* Test for cpu hotplug */
	if (buf->a.allocated)
		return 0;

	ret = ltt_chanbuf_alloc_create(&buf->a, &chan->a, cpu);
	if (ret)
		return ret;

	buf->commit_count =
		kzalloc_node(ALIGN(sizeof(*buf->commit_count) * chan->a.n_sb,
				   1 << INTERNODE_CACHE_SHIFT),
			GFP_KERNEL, cpu_to_node(cpu));
	if (!buf->commit_count) {
		ret = -ENOMEM;
		goto free_chanbuf;
	}

#ifdef LTT_VMCORE
	buf->commit_seq =
		kzalloc_node(ALIGN(sizeof(*buf->commit_seq) * chan->a.n_sb,
				   1 << INTERNODE_CACHE_SHIFT),
			GFP_KERNEL, cpu_to_node(cpu));
	if (!buf->commit_seq) {
		kfree(buf->commit_count);
		ret = -ENOMEM;
		goto free_commit;
	}
#endif

	local_set(&buf->offset, ltt_sb_header_size());
	atomic_long_set(&buf->consumed, 0);
	atomic_long_set(&buf->active_readers, 0);
	n_sb = chan->a.n_sb;
	for (j = 0; j < n_sb; j++) {
		local_set(&buf->commit_count[j].cc, 0);
		local_set(&buf->commit_count[j].cc_sb, 0);
		local_set(&buf->commit_count[j].events, 0);
	}
	init_waitqueue_head(&buf->write_wait);
	init_waitqueue_head(&buf->read_wait);
	spin_lock_init(&buf->full_lock);

	RCHAN_SB_CLEAR_NOREF(buf->a.buf_wsb[0].pages);
	ltt_buffer_begin(buf, trace->start_tsc, 0);
	/* atomic_add made on local variable on data that belongs to
	 * various CPUs : ok because tracing not started (for this cpu). */
	local_add(ltt_sb_header_size(), &buf->commit_count[0].cc);

	local_set(&buf->events_lost, 0);
	local_set(&buf->corrupted_subbuffers, 0);
	buf->finalized = 0;

	ret = ltt_chanbuf_create_file(chan->a.filename, chan->a.parent,
				      S_IRUSR, buf);
	if (ret)
		goto free_init;

	/*
	 * Ensure the buffer is ready before setting it to allocated.
	 * Used for cpu hotplug vs async wakeup.
	 */
	smp_wmb();
	buf->a.allocated = 1;

	return 0;

	/* Error handling */
free_init:
#ifdef LTT_VMCORE
	kfree(buf->commit_seq);
free_commit:
#endif
	kfree(buf->commit_count);
free_chanbuf:
	ltt_chanbuf_alloc_free(&buf->a);
	return ret;
}

void ltt_chan_remove_files(struct ltt_chan *chan)
{
	ltt_ascii_remove(chan);
	ltt_chan_alloc_remove_files(&chan->a);
}
EXPORT_SYMBOL_GPL(ltt_chan_remove_files);


void ltt_chan_free(struct kref *kref)
{
	struct ltt_chan *chan = container_of(kref, struct ltt_chan, a.kref);

	ltt_chan_alloc_free(&chan->a);
}
EXPORT_SYMBOL_GPL(ltt_chan_free);

int ltt_chan_create(const char *base_filename,
		    struct ltt_chan *chan, struct dentry *parent,
		    size_t sb_size, size_t n_sb,
		    int overwrite, struct ltt_trace *trace)
{
	int ret;

	chan->overwrite = overwrite;

	ret = ltt_chan_alloc_init(&chan->a, trace, base_filename, parent,
				  sb_size, n_sb, overwrite, overwrite);
	if (ret)
		goto error;

	chan->commit_count_mask = (~0UL >> chan->a.n_sb_order);

	ret = ltt_ascii_create(chan);
	if (ret)
		goto error_chan_alloc_free;

	return ret;

error_chan_alloc_free:
	ltt_chan_alloc_free(&chan->a);
error:
	return ret;
}
EXPORT_SYMBOL_GPL(ltt_chan_create);

int ltt_chanbuf_open_read(struct ltt_chanbuf *buf)
{
	kref_get(&buf->a.chan->kref);
	if (!atomic_long_add_unless(&buf->active_readers, 1, 1)) {
		kref_put(&buf->a.chan->kref, ltt_chan_free);
		return -EBUSY;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(ltt_chanbuf_open_read);

void ltt_chanbuf_release_read(struct ltt_chanbuf *buf)
{
	//ltt_relay_destroy_buffer(&buf->a.chan->a, buf->a.cpu);
	WARN_ON(atomic_long_read(&buf->active_readers) != 1);
	atomic_long_dec(&buf->active_readers);
	kref_put(&buf->a.chan->kref, ltt_chan_free);
}
EXPORT_SYMBOL_GPL(ltt_chanbuf_release_read);

static void ltt_relay_wake_writers(struct ltt_chanbuf *buf)
{

	if (waitqueue_active(&buf->write_wait))
		wake_up_interruptible(&buf->write_wait);
}

static void ltt_buf_unfull(struct ltt_chanbuf *buf)
{
	ltt_relay_wake_writers(buf);
}

static void remote_mb(void *info)
{
	smp_mb();
}

int ltt_chanbuf_get_subbuf(struct ltt_chanbuf *buf, unsigned long *consumed)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);
	long consumed_old, consumed_idx, commit_count, write_offset;
	int ret;

	consumed_old = atomic_long_read(&buf->consumed);
	consumed_idx = SUBBUF_INDEX(consumed_old, chan);
	commit_count = local_read(&buf->commit_count[consumed_idx].cc_sb);
	/*
	 * Make sure we read the commit count before reading the buffer
	 * data and the write offset. Correct consumed offset ordering
	 * wrt commit count is insured by the use of cmpxchg to update
	 * the consumed offset.
	 * smp_call_function_single can fail if the remote CPU is offline,
	 * this is OK because then there is no wmb to execute there.
	 * If our thread is executing on the same CPU as the on the buffers
	 * belongs to, we don't have to synchronize it at all. If we are
	 * migrated, the scheduler will take care of the memory barriers.
	 * Normally, smp_call_function_single() should ensure program order when
	 * executing the remote function, which implies that it surrounds the
	 * function execution with :
	 * smp_mb()
	 * send IPI
	 * csd_lock_wait
	 *                recv IPI
	 *                smp_mb()
	 *                exec. function
	 *                smp_mb()
	 *                csd unlock
	 * smp_mb()
	 *
	 * However, smp_call_function_single() does not seem to clearly execute
	 * such barriers. It depends on spinlock semantic to provide the barrier
	 * before executing the IPI and, when busy-looping, csd_lock_wait only
	 * executes smp_mb() when it has to wait for the other CPU.
	 *
	 * I don't trust this code. Therefore, let's add the smp_mb() sequence
	 * required ourself, even if duplicated. It has no performance impact
	 * anyway.
	 *
	 * smp_mb() is needed because smp_rmb() and smp_wmb() only order read vs
	 * read and write vs write. They do not ensure core synchronization. We
	 * really have to ensure total order between the 3 barriers running on
	 * the 2 CPUs.
	 */
#ifdef LTT_NO_IPI_BARRIER
	/*
	 * Local rmb to match the remote wmb to read the commit count before the
	 * buffer data and the write offset.
	 */
	smp_rmb();
#else
	if (raw_smp_processor_id() != buf->a.cpu) {
		smp_mb();	/* Total order with IPI handler smp_mb() */
		smp_call_function_single(buf->a.cpu, remote_mb, NULL, 1);
		smp_mb();	/* Total order with IPI handler smp_mb() */
	}
#endif
	write_offset = local_read(&buf->offset);
	/*
	 * Check that the subbuffer we are trying to consume has been
	 * already fully committed.
	 */
	if (((commit_count - chan->a.sb_size)
	     & chan->commit_count_mask)
	    - (BUFFER_TRUNC(consumed_old, chan)
	       >> chan->a.n_sb_order)
	    != 0) {
		return -EAGAIN;
	}
	/*
	 * Check that we are not about to read the same subbuffer in
	 * which the writer head is.
	 */
	if ((SUBBUF_TRUNC(write_offset, chan)
	   - SUBBUF_TRUNC(consumed_old, chan))
	   == 0) {
		return -EAGAIN;
	}

	ret = update_read_sb_index(&buf->a, &chan->a, consumed_idx);
	if (ret)
		return ret;

	*consumed = consumed_old;
	return 0;
}
EXPORT_SYMBOL_GPL(ltt_chanbuf_get_subbuf);

int ltt_chanbuf_put_subbuf(struct ltt_chanbuf *buf, unsigned long consumed)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);
	long consumed_new, consumed_old;

	WARN_ON(atomic_long_read(&buf->active_readers) != 1);

	consumed_old = consumed;
	consumed_new = SUBBUF_ALIGN(consumed_old, chan);
	WARN_ON_ONCE(RCHAN_SB_IS_NOREF(buf->a.buf_rsb.pages));
	RCHAN_SB_SET_NOREF(buf->a.buf_rsb.pages);

	spin_lock(&buf->full_lock);
	if (atomic_long_cmpxchg(&buf->consumed, consumed_old, consumed_new)
	    != consumed_old) {
		/* We have been pushed by the writer. */
		spin_unlock(&buf->full_lock);
		/*
		 * We exchanged the subbuffer pages. No corruption possible
		 * even if the writer did push us. No more -EIO possible.
		 */
		return 0;
	} else {
		/* tell the client that buffer is now unfull */
		int index;
		long data;
		index = SUBBUF_INDEX(consumed_old, chan);
		data = BUFFER_OFFSET(consumed_old, chan);
		ltt_buf_unfull(buf);
		spin_unlock(&buf->full_lock);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(ltt_chanbuf_put_subbuf);

static void switch_buffer(unsigned long data)
{
	struct ltt_chanbuf *buf = (struct ltt_chanbuf *)data;
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);

	/*
	 * Only flush buffers periodically if readers are active.
	 */
	if (atomic_long_read(&buf->active_readers))
		ltt_force_switch(buf, FORCE_ACTIVE);

	mod_timer_pinned(&buf->switch_timer,
			 jiffies + chan->switch_timer_interval);
}

static void ltt_chanbuf_start_switch_timer(struct ltt_chanbuf *buf)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);

	if (!chan->switch_timer_interval)
		return;

	init_timer_deferrable(&buf->switch_timer);
	buf->switch_timer.function = switch_buffer;
	buf->switch_timer.expires = jiffies + chan->switch_timer_interval;
	buf->switch_timer.data = (unsigned long)buf;
	add_timer_on(&buf->switch_timer, buf->a.cpu);
}

void ltt_chan_start_switch_timer(struct ltt_chan *chan)
{
	int cpu;

	if (!chan->switch_timer_interval)
		return;

	for_each_online_cpu(cpu) {
		struct ltt_chanbuf *buf;

		buf = per_cpu_ptr(chan->a.buf, cpu);
		ltt_chanbuf_start_switch_timer(buf);
	}
}

static void ltt_chanbuf_stop_switch_timer(struct ltt_chanbuf *buf)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);

	if (!chan->switch_timer_interval)
		return;

	del_timer_sync(&buf->switch_timer);
}

void ltt_chan_stop_switch_timer(struct ltt_chan *chan)
{
	int cpu;

	if (!chan->switch_timer_interval)
		return;

	for_each_online_cpu(cpu) {
		struct ltt_chanbuf *buf;

		buf = per_cpu_ptr(chan->a.buf, cpu);
		ltt_chanbuf_stop_switch_timer(buf);
	}
}

static void ltt_chanbuf_idle_switch(struct ltt_chanbuf *buf)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);

	if (chan->switch_timer_interval)
		ltt_force_switch(buf, FORCE_ACTIVE);
}

static void ltt_chanbuf_switch(struct ltt_chanbuf *buf)
{
	if (buf->a.allocated)
		ltt_force_switch(buf, FORCE_ACTIVE);
}

static
int ltt_chanbuf_hotcpu_callback(struct notifier_block *nb,
					  unsigned long action,
					  void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;

	switch (action) {
	case CPU_DOWN_FAILED:
	case CPU_DOWN_FAILED_FROZEN:
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		/*
		 * CPU hotplug lock protects trace lock from this callback.
		 */
		ltt_chan_for_each_channel(ltt_chanbuf_start_switch_timer, cpu);
		return NOTIFY_OK;

	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		/*
		 * Performs an IPI to delete the timer locally on the target
		 * CPU.	CPU hotplug lock protects trace lock from this
		 * callback.
		 */
		ltt_chan_for_each_channel(ltt_chanbuf_stop_switch_timer, cpu);
		return NOTIFY_OK;

	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
		/*
		 * Performing a buffer switch on a remote CPU. Performed by
		 * the CPU responsible for doing the hotunplug after the target
		 * CPU stopped running completely. Ensures that all data
		 * from that remote CPU is flushed. CPU hotplug lock protects
		 * trace lock from this callback.
		 */
		ltt_chan_for_each_channel(ltt_chanbuf_switch, cpu);
		return NOTIFY_OK;

	default:
		return NOTIFY_DONE;
	}
}

static int pm_idle_entry_callback(struct notifier_block *self,
				  unsigned long val, void *data)
{
	if (val == IDLE_START) {
		rcu_read_lock_sched_notrace();
		ltt_chan_for_each_channel(ltt_chanbuf_idle_switch,
					  smp_processor_id());
		rcu_read_unlock_sched_notrace();
	}
	return 0;
}

struct notifier_block pm_idle_entry_notifier = {
	.notifier_call = pm_idle_entry_callback,
	.priority = ~0U,	/* smallest prio, run after tracing events */
};

static
void ltt_relay_print_written(struct ltt_chan *chan, long cons_off,
			     unsigned int cpu)
{
	struct ltt_chanbuf *buf = per_cpu_ptr(chan->a.buf, cpu);
	long cons_idx, events_count;

	cons_idx = SUBBUF_INDEX(cons_off, chan);
	events_count = local_read(&buf->commit_count[cons_idx].events);

	if (events_count)
		printk(KERN_INFO
			"LTT: %lu events written in channel %s "
			"(cpu %u, index %lu)\n",
			events_count, chan->a.filename, cpu, cons_idx);
}

static
void ltt_relay_print_subbuffer_errors(struct ltt_chanbuf *buf,
				      struct ltt_chan *chan, long cons_off,
				      unsigned int cpu)
{
	long cons_idx, commit_count, commit_count_sb, write_offset;

	cons_idx = SUBBUF_INDEX(cons_off, chan);
	commit_count = local_read(&buf->commit_count[cons_idx].cc);
	commit_count_sb = local_read(&buf->commit_count[cons_idx].cc_sb);
	/*
	 * No need to order commit_count and write_offset reads because we
	 * execute after trace is stopped when there are no readers left.
	 */
	write_offset = local_read(&buf->offset);
	printk(KERN_WARNING
	       "LTT : unread channel %s offset is %ld "
	       "and cons_off : %ld (cpu %u)\n",
	       chan->a.filename, write_offset, cons_off, cpu);
	/* Check each sub-buffer for non filled commit count */
	if (((commit_count - chan->a.sb_size) & chan->commit_count_mask)
	    - (BUFFER_TRUNC(cons_off, chan) >> chan->a.n_sb_order)
	    != 0)
		printk(KERN_ALERT
		       "LTT : %s : subbuffer %lu has non filled "
		       "commit count [cc, cc_sb] [%lu,%lu].\n",
		       chan->a.filename, cons_idx, commit_count,
		       commit_count_sb);
	printk(KERN_ALERT "LTT : %s : commit count : %lu, subbuf size %lu\n",
	       chan->a.filename, commit_count, chan->a.sb_size);
}

static
void ltt_relay_print_errors(struct ltt_chanbuf *buf, struct ltt_chan *chan,
			    struct ltt_trace *trace, int cpu)
{
	long cons_off;

	/*
	 * Can be called in the error path of allocation when
	 * trans_channel_data is not yet set.
	 */
	if (!chan)
		return;
	for (cons_off = 0; cons_off < chan->a.buf_size;
	     cons_off = SUBBUF_ALIGN(cons_off, chan))
		ltt_relay_print_written(chan, cons_off, cpu);
	for (cons_off = atomic_long_read(&buf->consumed);
			(SUBBUF_TRUNC(local_read(&buf->offset), chan)
			 - cons_off) > 0;
			cons_off = SUBBUF_ALIGN(cons_off, chan))
		ltt_relay_print_subbuffer_errors(buf, chan, cons_off, cpu);
}

static
void ltt_relay_print_buffer_errors(struct ltt_chan *chan, unsigned int cpu)
{
	struct ltt_trace *trace = chan->a.trace;
	struct ltt_chanbuf *buf = per_cpu_ptr(chan->a.buf, cpu);

	if (local_read(&buf->events_lost))
		printk(KERN_ALERT
		       "LTT : %s : %ld events lost "
		       "in %s channel (cpu %u).\n",
		       chan->a.filename, local_read(&buf->events_lost),
		       chan->a.filename, cpu);
	if (local_read(&buf->corrupted_subbuffers))
		printk(KERN_ALERT
		       "LTT : %s : %ld corrupted subbuffers "
		       "in %s channel (cpu %u).\n",
		       chan->a.filename,
		       local_read(&buf->corrupted_subbuffers),
		       chan->a.filename, cpu);

	ltt_relay_print_errors(buf, chan, trace, cpu);
}

static void ltt_relay_remove_dirs(struct ltt_trace *trace)
{
	ltt_ascii_remove_dir(trace);
	debugfs_remove(trace->dentry.trace_root);
}

static int ltt_relay_create_dirs(struct ltt_trace *new_trace)
{
	struct dentry *ltt_root_dentry;
	int ret;

	ltt_root_dentry = get_ltt_root();
	if (!ltt_root_dentry)
		return ENOENT;

	new_trace->dentry.trace_root = debugfs_create_dir(new_trace->trace_name,
							  ltt_root_dentry);
	put_ltt_root();
	if (new_trace->dentry.trace_root == NULL) {
		printk(KERN_ERR "LTT : Trace directory name %s already taken\n",
		       new_trace->trace_name);
		return EEXIST;
	}
	ret = ltt_ascii_create_dir(new_trace);
	if (ret)
		printk(KERN_WARNING "LTT : Unable to create ascii output file "
				    "for trace %s\n", new_trace->trace_name);

	return 0;
}

static notrace void ltt_relay_buffer_flush(struct ltt_chanbuf *buf)
{
	buf->finalized = 1;
	ltt_force_switch(buf, FORCE_FLUSH);
}

static void ltt_relay_async_wakeup_chan(struct ltt_chan *chan)
{
	unsigned int i;

	for_each_possible_cpu(i) {
		struct ltt_chanbuf *buf;

		buf = per_cpu_ptr(chan->a.buf, i);
		if (!buf->a.allocated)
			continue;
		/*
		 * Ensure the buffer has been allocated before reading its
		 * content. Sync cpu hotplug vs async wakeup.
		 */
		smp_rmb();
		if (ltt_poll_deliver(buf, chan))
			wake_up_interruptible(&buf->read_wait);
	}
}

static void ltt_relay_finish_buffer(struct ltt_chan *chan, unsigned int cpu)
{
	struct ltt_chanbuf *buf = per_cpu_ptr(chan->a.buf, cpu);

	if (buf->a.allocated) {
		ltt_relay_buffer_flush(buf);
		ltt_relay_wake_writers(buf);
	}
}


static void ltt_relay_finish_channel(struct ltt_chan *chan)
{
	unsigned int i;

	for_each_possible_cpu(i)
		ltt_relay_finish_buffer(chan, i);
}

static
int ltt_relay_user_blocking(struct ltt_trace *trace, unsigned int chan_index,
			    size_t data_size, struct user_dbg_data *dbg)
{
	struct ltt_chanbuf *buf;
	struct ltt_chan *chan;
	int cpu;
	DECLARE_WAITQUEUE(wait, current);

	chan = &trace->channels[chan_index];
	cpu = smp_processor_id();
	buf = per_cpu_ptr(chan->a.buf, cpu);

	/*
	 * Check if data is too big for the channel : do not
	 * block for it.
	 */
	if (LTT_RESERVE_CRITICAL + data_size > chan->a.sb_size)
		return 0;

	/*
	 * If free space too low, we block. We restart from the
	 * beginning after we resume (cpu id may have changed
	 * while preemption is active).
	 */
	spin_lock(&buf->full_lock);
	if (!chan->overwrite) {
		dbg->write = local_read(&buf->offset);
		dbg->read = atomic_long_read(&buf->consumed);
		dbg->avail_size = dbg->write + LTT_RESERVE_CRITICAL + data_size
				  - SUBBUF_TRUNC(dbg->read, chan);
		if (dbg->avail_size > chan->a.buf_size) {
			__set_current_state(TASK_INTERRUPTIBLE);
			add_wait_queue(&buf->write_wait, &wait);
			spin_unlock(&buf->full_lock);
			preempt_enable();
			schedule();
			__set_current_state(TASK_RUNNING);
			remove_wait_queue(&buf->write_wait, &wait);
			if (signal_pending(current))
				return -ERESTARTSYS;
			preempt_disable();
			return 1;
		}
	}
	spin_unlock(&buf->full_lock);
	return 0;
}

static
void ltt_relay_print_user_errors(struct ltt_trace *trace,
				 unsigned int chan_index, size_t data_size,
				 struct user_dbg_data *dbg, int cpu)
{
	struct ltt_chanbuf *buf;
	struct ltt_chan *chan;

	chan = &trace->channels[chan_index];
	buf = per_cpu_ptr(chan->a.buf, cpu);

	printk(KERN_ERR "Error in LTT usertrace : "
	       "buffer full : event lost in blocking "
	       "mode. Increase LTT_RESERVE_CRITICAL.\n");
	printk(KERN_ERR "LTT nesting level is %u.\n",
	       per_cpu(ltt_nesting, cpu));
	printk(KERN_ERR "LTT available size %lu.\n",
	       dbg->avail_size);
	printk(KERN_ERR "available write : %lu, read : %lu\n",
	       dbg->write, dbg->read);

	dbg->write = local_read(&buf->offset);
	dbg->read = atomic_long_read(&buf->consumed);

	printk(KERN_ERR "LTT current size %lu.\n",
		dbg->write + LTT_RESERVE_CRITICAL + data_size
		- SUBBUF_TRUNC(dbg->read, chan));
	printk(KERN_ERR "current write : %lu, read : %lu\n",
			dbg->write, dbg->read);
}

static
void ltt_reserve_switch_old_subbuf(struct ltt_chanbuf *buf,
				   struct ltt_chan *chan,
				   struct ltt_reserve_switch_offsets *offsets,
				   u64 *tsc)
{
	long oldidx = SUBBUF_INDEX(offsets->old - 1, chan);
	long commit_count, padding_size;

	padding_size = chan->a.sb_size
			- (SUBBUF_OFFSET(offsets->old - 1, chan) + 1);
	ltt_buffer_end(buf, *tsc, offsets->old, oldidx);

	/*
	 * Must write slot data before incrementing commit count.
	 * This compiler barrier is upgraded into a smp_wmb() by the IPI
	 * sent by get_subbuf() when it does its smp_rmb().
	 */
	barrier();
	local_add(padding_size, &buf->commit_count[oldidx].cc);
	commit_count = local_read(&buf->commit_count[oldidx].cc);
	ltt_check_deliver(buf, chan, offsets->old - 1, commit_count, oldidx);
	ltt_write_commit_counter(buf, chan, oldidx, offsets->old, commit_count,
				 padding_size);
}

static
void ltt_reserve_switch_new_subbuf(struct ltt_chanbuf *buf,
				   struct ltt_chan *chan,
				   struct ltt_reserve_switch_offsets *offsets,
				   u64 *tsc)
{
	long beginidx = SUBBUF_INDEX(offsets->begin, chan);
	long commit_count;

	ltt_buffer_begin(buf, *tsc, beginidx);

	/*
	 * Must write slot data before incrementing commit count.
	 * This compiler barrier is upgraded into a smp_wmb() by the IPI
	 * sent by get_subbuf() when it does its smp_rmb().
	 */
	barrier();
	local_add(ltt_sb_header_size(), &buf->commit_count[beginidx].cc);
	commit_count = local_read(&buf->commit_count[beginidx].cc);
	/* Check if the written buffer has to be delivered */
	ltt_check_deliver(buf, chan, offsets->begin, commit_count, beginidx);
	ltt_write_commit_counter(buf, chan, beginidx, offsets->begin,
				 commit_count, ltt_sb_header_size());
}


static
void ltt_reserve_end_switch_current(struct ltt_chanbuf *buf,
				    struct ltt_chan *chan,
				    struct ltt_reserve_switch_offsets *offsets,
				    u64 *tsc)
{
	long endidx = SUBBUF_INDEX(offsets->end - 1, chan);
	long commit_count, padding_size;

	padding_size = chan->a.sb_size
			- (SUBBUF_OFFSET(offsets->end - 1, chan) + 1);

	ltt_buffer_end(buf, *tsc, offsets->end, endidx);

	/*
	 * Must write slot data before incrementing commit count.
	 * This compiler barrier is upgraded into a smp_wmb() by the IPI
	 * sent by get_subbuf() when it does its smp_rmb().
	 */
	barrier();
	local_add(padding_size, &buf->commit_count[endidx].cc);
	commit_count = local_read(&buf->commit_count[endidx].cc);
	ltt_check_deliver(buf, chan, offsets->end - 1, commit_count, endidx);
	ltt_write_commit_counter(buf, chan, endidx, offsets->end, commit_count,
				 padding_size);
}

static
int ltt_relay_try_switch_slow(enum force_switch_mode mode,
			      struct ltt_chanbuf *buf, struct ltt_chan *chan,
			      struct ltt_reserve_switch_offsets *offsets,
			      u64 *tsc)
{
	long sb_index;
	long reserve_commit_diff;
	long off;

	offsets->begin = local_read(&buf->offset);
	offsets->old = offsets->begin;
	offsets->begin_switch = 0;
	offsets->end_switch_old = 0;

	*tsc = trace_clock_read64();

	off = SUBBUF_OFFSET(offsets->begin, chan);
	if ((mode != FORCE_ACTIVE && off > 0) || off > ltt_sb_header_size()) {
		offsets->begin = SUBBUF_ALIGN(offsets->begin, chan);
		offsets->end_switch_old = 1;
	} else {
		/* we do not have to switch : buffer is empty */
		return -1;
	}
	if (mode == FORCE_ACTIVE)
		offsets->begin += ltt_sb_header_size();
	/*
	 * Always begin_switch in FORCE_ACTIVE mode.
	 * Test new buffer integrity
	 */
	sb_index = SUBBUF_INDEX(offsets->begin, chan);
	reserve_commit_diff =
		(BUFFER_TRUNC(offsets->begin, chan)
		 >> chan->a.n_sb_order)
		- (local_read(&buf->commit_count[sb_index].cc_sb)
			& chan->commit_count_mask);
	if (reserve_commit_diff == 0) {
		/* Next buffer not corrupted. */
		if (mode == FORCE_ACTIVE
		    && !chan->overwrite
		    && offsets->begin - atomic_long_read(&buf->consumed)
		       >= chan->a.buf_size) {
			/*
			 * We do not overwrite non consumed buffers and we are
			 * full : ignore switch while tracing is active.
			 */
			return -1;
		}
	} else {
		/*
		 * Next subbuffer corrupted. Force pushing reader even in normal
		 * mode
		 */
	}
	offsets->end = offsets->begin;
	return 0;
}

void ltt_force_switch_lockless_slow(struct ltt_chanbuf *buf,
				    enum force_switch_mode mode)
{
	struct ltt_chan *chan = container_of(buf->a.chan, struct ltt_chan, a);
	struct ltt_reserve_switch_offsets offsets;
	u64 tsc;

	offsets.size = 0;

	/*
	 * Perform retryable operations.
	 */
	do {
		if (ltt_relay_try_switch_slow(mode, buf, chan, &offsets, &tsc))
			return;
	} while (local_cmpxchg(&buf->offset, offsets.old, offsets.end)
		 != offsets.old);

	/*
	 * Atomically update last_tsc. This update races against concurrent
	 * atomic updates, but the race will always cause supplementary full TSC
	 * events, never the opposite (missing a full TSC event when it would be
	 * needed).
	 */
	save_last_tsc(buf, tsc);

	/*
	 * Push the reader if necessary
	 */
	if (mode == FORCE_ACTIVE) {
		ltt_reserve_push_reader(buf, chan, offsets.end - 1);
		ltt_clear_noref_flag(&buf->a, SUBBUF_INDEX(offsets.end - 1,
							   chan));
	}

	/*
	 * Switch old subbuffer if needed.
	 */
	if (offsets.end_switch_old) {
		ltt_clear_noref_flag(&buf->a, SUBBUF_INDEX(offsets.old - 1,
							   chan));
		ltt_reserve_switch_old_subbuf(buf, chan, &offsets, &tsc);
	}

	/*
	 * Populate new subbuffer.
	 */
	if (mode == FORCE_ACTIVE)
		ltt_reserve_switch_new_subbuf(buf, chan, &offsets, &tsc);
}
EXPORT_SYMBOL_GPL(ltt_force_switch_lockless_slow);

static
int ltt_relay_try_reserve_slow(struct ltt_chanbuf *buf, struct ltt_chan *chan,
			       struct ltt_reserve_switch_offsets *offsets,
			       size_t data_size, u64 *tsc, unsigned int *rflags,
			       int largest_align)
{
	long reserve_commit_diff;

	offsets->begin = local_read(&buf->offset);
	offsets->old = offsets->begin;
	offsets->begin_switch = 0;
	offsets->end_switch_current = 0;
	offsets->end_switch_old = 0;

	*tsc = trace_clock_read64();
	if (last_tsc_overflow(buf, *tsc))
		*rflags = LTT_RFLAG_ID_SIZE_TSC;

	if (unlikely(SUBBUF_OFFSET(offsets->begin, chan) == 0)) {
		offsets->begin_switch = 1;		/* For offsets->begin */
	} else {
		offsets->size = ltt_get_header_size(chan, offsets->begin,
						    data_size,
						    &offsets->before_hdr_pad,
						    *rflags);
		offsets->size += ltt_align(offsets->begin + offsets->size,
					   largest_align)
				 + data_size;
		if (unlikely((SUBBUF_OFFSET(offsets->begin, chan) +
			     offsets->size) > chan->a.sb_size)) {
			offsets->end_switch_old = 1;	/* For offsets->old */
			offsets->begin_switch = 1;	/* For offsets->begin */
		}
	}
	if (unlikely(offsets->begin_switch)) {
		long sb_index;

		/*
		 * We are typically not filling the previous buffer completely.
		 */
		if (likely(offsets->end_switch_old))
			offsets->begin = SUBBUF_ALIGN(offsets->begin, chan);
		offsets->begin = offsets->begin + ltt_sb_header_size();
		/* Test new buffer integrity */
		sb_index = SUBBUF_INDEX(offsets->begin, chan);
		reserve_commit_diff =
		  (BUFFER_TRUNC(offsets->begin, chan)
		   >> chan->a.n_sb_order)
		  - (local_read(&buf->commit_count[sb_index].cc_sb)
				& chan->commit_count_mask);
		if (likely(reserve_commit_diff == 0)) {
			/* Next buffer not corrupted. */
			if (unlikely(!chan->overwrite &&
				(SUBBUF_TRUNC(offsets->begin, chan)
				 - SUBBUF_TRUNC(atomic_long_read(&buf->consumed),
						chan))
				>= chan->a.buf_size)) {
				/*
				 * We do not overwrite non consumed buffers
				 * and we are full : event is lost.
				 */
				local_inc(&buf->events_lost);
				return -1;
			} else {
				/*
				 * next buffer not corrupted, we are either in
				 * overwrite mode or the buffer is not full.
				 * It's safe to write in this new subbuffer.
				 */
			}
		} else {
			/*
			 * Next subbuffer corrupted. Drop event in normal and
			 * overwrite mode. Caused by either a writer OOPS or
			 * too many nested writes over a reserve/commit pair.
			 */
			local_inc(&buf->events_lost);
			return -1;
		}
		offsets->size = ltt_get_header_size(chan, offsets->begin,
						    data_size,
						    &offsets->before_hdr_pad,
						    *rflags);
		offsets->size += ltt_align(offsets->begin + offsets->size,
					   largest_align)
				 + data_size;
		if (unlikely((SUBBUF_OFFSET(offsets->begin, chan)
			     + offsets->size) > chan->a.sb_size)) {
			/*
			 * Event too big for subbuffers, report error, don't
			 * complete the sub-buffer switch.
			 */
			local_inc(&buf->events_lost);
			return -1;
		} else {
			/*
			 * We just made a successful buffer switch and the event
			 * fits in the new subbuffer. Let's write.
			 */
		}
	} else {
		/*
		 * Event fits in the current buffer and we are not on a switch
		 * boundary. It's safe to write.
		 */
	}
	offsets->end = offsets->begin + offsets->size;

	if (unlikely((SUBBUF_OFFSET(offsets->end, chan)) == 0)) {
		/*
		 * The offset_end will fall at the very beginning of the next
		 * subbuffer.
		 */
		offsets->end_switch_current = 1;	/* For offsets->begin */
	}
	return 0;
}

int ltt_reserve_slot_lockless_slow(struct ltt_chan *chan,
				   struct ltt_trace *trace, size_t data_size,
				   int largest_align, int cpu,
				   struct ltt_chanbuf **ret_buf,
				   size_t *slot_size, long *buf_offset,
				   u64 *tsc, unsigned int *rflags)
{
	struct ltt_chanbuf *buf = *ret_buf = per_cpu_ptr(chan->a.buf, cpu);
	struct ltt_reserve_switch_offsets offsets;

	offsets.size = 0;

	do {
		if (unlikely(ltt_relay_try_reserve_slow(buf, chan, &offsets,
							data_size, tsc, rflags,
							largest_align)))
			return -ENOSPC;
	} while (unlikely(local_cmpxchg(&buf->offset, offsets.old, offsets.end)
			  != offsets.old));

	/*
	 * Atomically update last_tsc. This update races against concurrent
	 * atomic updates, but the race will always cause supplementary full TSC
	 * events, never the opposite (missing a full TSC event when it would be
	 * needed).
	 */
	save_last_tsc(buf, *tsc);

	/*
	 * Push the reader if necessary
	 */
	ltt_reserve_push_reader(buf, chan, offsets.end - 1);

	/*
	 * Clear noref flag for this subbuffer.
	 */
	ltt_clear_noref_flag(&buf->a, SUBBUF_INDEX(offsets.end - 1, chan));

	/*
	 * Switch old subbuffer if needed.
	 */
	if (unlikely(offsets.end_switch_old)) {
		ltt_clear_noref_flag(&buf->a, SUBBUF_INDEX(offsets.old - 1,
							  chan));
		ltt_reserve_switch_old_subbuf(buf, chan, &offsets, tsc);
	}

	/*
	 * Populate new subbuffer.
	 */
	if (unlikely(offsets.begin_switch))
		ltt_reserve_switch_new_subbuf(buf, chan, &offsets, tsc);

	if (unlikely(offsets.end_switch_current))
		ltt_reserve_end_switch_current(buf, chan, &offsets, tsc);

	*slot_size = offsets.size;
	*buf_offset = offsets.begin + offsets.before_hdr_pad;
	return 0;
}
EXPORT_SYMBOL_GPL(ltt_reserve_slot_lockless_slow);

static struct ltt_transport ltt_relay_transport = {
	.name = "relay",
	.owner = THIS_MODULE,
	.ops = {
		.create_dirs = ltt_relay_create_dirs,
		.remove_dirs = ltt_relay_remove_dirs,
		.create_channel = ltt_chan_create,
		.finish_channel = ltt_relay_finish_channel,
		.remove_channel = ltt_chan_free,
		.remove_channel_files = ltt_chan_remove_files,
		.wakeup_channel = ltt_relay_async_wakeup_chan,
		.user_blocking = ltt_relay_user_blocking,
		.user_errors = ltt_relay_print_user_errors,
		.start_switch_timer = ltt_chan_start_switch_timer,
		.stop_switch_timer = ltt_chan_stop_switch_timer,
	},
};

static struct notifier_block fn_ltt_chanbuf_hotcpu_callback = {
	.notifier_call = ltt_chanbuf_hotcpu_callback,
	.priority = 6,
};

int __init ltt_relay_init(void)
{
	printk(KERN_INFO "LTT : ltt-relay init\n");

	ltt_transport_register(&ltt_relay_transport);
	register_cpu_notifier(&fn_ltt_chanbuf_hotcpu_callback);
	register_idle_notifier(&pm_idle_entry_notifier);

	return 0;
}

void __exit ltt_relay_exit(void)
{
	printk(KERN_INFO "LTT : ltt-relay exit\n");

	unregister_idle_notifier(&pm_idle_entry_notifier);
	unregister_cpu_notifier(&fn_ltt_chanbuf_hotcpu_callback);
	ltt_transport_unregister(&ltt_relay_transport);
}

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mathieu Desnoyers");
MODULE_DESCRIPTION("Linux Trace Toolkit Next Generation Lockless Relay");
