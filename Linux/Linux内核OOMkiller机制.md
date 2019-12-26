程序运行了一段时间，有个进程挂掉了，正常情况下进程不会主动挂掉，简单分析后认为可能是运行时某段时间内存占用过大，系统内存不足导致触发了Linux操作系统OOM killer机制，将运行中的进程杀掉了。

### 一、Linux内核OOM killer机制
Linux 内核有个机制叫OOM killer(Out Of Memory killer)，该机制会监控那些占用内存过大，尤其是瞬间占用内存很快的进程，然后防止内存耗尽而自动把该进程杀掉。内核检测到系统内存不足、挑选并杀掉某个进程的过程可以参考内核源代码[linux/mm/oom_kill.c](https://github.com/torvalds/linux/blob/master/mm/oom_kill.c)，当系统内存不足的时候，```out_of_memory()```被触发，然后调用```select_bad_process()```选择一个”bad”进程杀掉。如何判断和选择一个”bad进程呢？linux选择”bad”进程是通过调用```oom_badness()```，挑选的算法和想法都很简单很朴实：最bad的那个进程就是那个最占用内存的进程。

#### 0、什么时候触发?
内核在触发OOM机制时会调用到`out_of_memory()`函数，此函数的调用顺序如下：
```c
__alloc_pages  //内存分配时调用

    |-->__alloc_pages_nodemask

       |--> __alloc_pages_slowpath

           |--> __alloc_pages_may_oom

              | --> out_of_memory   //触发
```
以上函数`__alloc_pages_may_oom()`在调用之前会先判断`oom_killer_disabled`的值，如果有值，则不会触发OOM机制；

布尔型变量`oom_killer_disabled`定义在文件[mm/page_alloc.c](https://github.com/torvalds/linux/blob/master/mm/page_alloc.c)中，并没有提供外部接口更改此值，但是在内核中此值默认为`0`，表示打开OOM-kill。

Linux中内存都是以页的形式管理的，所以不管是怎么申请内存，都会调用`alloc_page()`函数，最终调用到函数`out_of_memory()`，触发OOM机制。

#### 【1】内核监测到系统内存不足时，该函数被触发执行：
```c
/**
 * out_of_memory - kill the "best" process when we run out of memory
 * @oc: pointer to struct oom_control
 *
 * If we run out of memory, we have the choice between either
 * killing a random task (bad), letting the system crash (worse)
 * OR try to be smart about which process to kill. Note that we
 * don't have to be perfect here, we just have to be good.
 */
bool out_of_memory(struct oom_control *oc)
{
	unsigned long freed = 0;
	enum oom_constraint constraint = CONSTRAINT_NONE;

	if (oom_killer_disabled)
		return false;

	if (!is_memcg_oom(oc)) {
		blocking_notifier_call_chain(&oom_notify_list, 0, &freed);
		if (freed > 0)
			/* Got some memory back in the last second. */
			return true;
	}

	/*
	 * If current has a pending SIGKILL or is exiting, then automatically
	 * select it.  The goal is to allow it to allocate so that it may
	 * quickly exit and free its memory.
	 */
	if (task_will_free_mem(current)) {
		mark_oom_victim(current);
		wake_oom_reaper(current);
		return true;
	}

	/*
	 * The OOM killer does not compensate for IO-less reclaim.
	 * pagefault_out_of_memory lost its gfp context so we have to
	 * make sure exclude 0 mask - all other users should have at least
	 * ___GFP_DIRECT_RECLAIM to get here.
	 */
	if (oc->gfp_mask && !(oc->gfp_mask & __GFP_FS))
		return true;

	/*
	 * Check if there were limitations on the allocation (only relevant for
	 * NUMA and memcg) that may require different handling.
	 */
	constraint = constrained_alloc(oc);
	if (constraint != CONSTRAINT_MEMORY_POLICY)
		oc->nodemask = NULL;
	check_panic_on_oom(oc, constraint);

	if (!is_memcg_oom(oc) && sysctl_oom_kill_allocating_task &&
	    current->mm && !oom_unkillable_task(current, NULL, oc->nodemask) &&
	    current->signal->oom_score_adj != OOM_SCORE_ADJ_MIN) {
		get_task_struct(current);
		oc->chosen = current;
		oom_kill_process(oc, "Out of memory (oom_kill_allocating_task)");
		return true;
	}

	select_bad_process(oc); //选择一个“最坏的”进程杀掉。
	/* Found nothing?!?! */
	if (!oc->chosen) {
		dump_header(oc, NULL);
		pr_warn("Out of memory and no killable processes...\n");
		/*
		 * If we got here due to an actual allocation at the
		 * system level, we cannot survive this and will enter
		 * an endless loop in the allocator. Bail out now.
		 */
		if (!is_sysrq_oom(oc) && !is_memcg_oom(oc))
			panic("System is deadlocked on memory\n");
	}
	if (oc->chosen && oc->chosen != (void *)-1UL)
		oom_kill_process(oc, !is_memcg_oom(oc) ? "Out of memory" :
				 "Memory cgroup out of memory");
	return !!oc->chosen;
}

```

#### 【2】选择一个“最坏的”进程
```c
/*
 * Simple selection loop. We choose the process with the highest number of
 * 'points'. In case scan was aborted, oc->chosen is set to -1.
 */
static void select_bad_process(struct oom_control *oc)
{
	if (is_memcg_oom(oc))
		mem_cgroup_scan_tasks(oc->memcg, oom_evaluate_task, oc);
	else {
		struct task_struct *p;

		rcu_read_lock();
		for_each_process(p)
			if (oom_evaluate_task(p, oc))
				break;
		rcu_read_unlock();
	}

	oc->chosen_points = oc->chosen_points * 1000 / oc->totalpages;
}
```
#### 【3】杀掉进程
```c
static void oom_kill_process(struct oom_control *oc, const char *message)
{
	struct task_struct *victim = oc->chosen;
	struct mem_cgroup *oom_group;
	static DEFINE_RATELIMIT_STATE(oom_rs, DEFAULT_RATELIMIT_INTERVAL,
					      DEFAULT_RATELIMIT_BURST);

	/*
	 * If the task is already exiting, don't alarm the sysadmin or kill
	 * its children or threads, just give it access to memory reserves
	 * so it can die quickly
	 */
	task_lock(victim);
	if (task_will_free_mem(victim)) {
		mark_oom_victim(victim);
		wake_oom_reaper(victim);
		task_unlock(victim);
		put_task_struct(victim);
		return;
	}
	task_unlock(victim);

	if (__ratelimit(&oom_rs))
		dump_header(oc, victim);

	/*
	 * Do we need to kill the entire memory cgroup?
	 * Or even one of the ancestor memory cgroups?
	 * Check this out before killing the victim task.
	 */
	oom_group = mem_cgroup_get_oom_group(victim, oc->memcg);

	__oom_kill_process(victim, message);

	/*
	 * If necessary, kill all tasks in the selected memory cgroup.
	 */
	if (oom_group) {
		mem_cgroup_print_oom_group(oom_group);
		mem_cgroup_scan_tasks(oom_group, oom_kill_memcg_member,
				      (void*)message);
		mem_cgroup_put(oom_group);
	}
}
```

下面附加两个函数是```select_bad_process()```函数的实现细节，可不看。
```c
static int oom_evaluate_task(struct task_struct *task, void *arg)
{
	struct oom_control *oc = arg;
	unsigned long points;

	if (oom_unkillable_task(task, NULL, oc->nodemask))
		goto next;

	/*
	 * This task already has access to memory reserves and is being killed.
	 * Don't allow any other task to have access to the reserves unless
	 * the task has MMF_OOM_SKIP because chances that it would release
	 * any memory is quite low.
	 */
	if (!is_sysrq_oom(oc) && tsk_is_oom_victim(task)) {
		if (test_bit(MMF_OOM_SKIP, &task->signal->oom_mm->flags))
			goto next;
		goto abort;
	}

	/*
	 * If task is allocating a lot of memory and has been marked to be
	 * killed first if it triggers an oom, then select it.
	 */
	if (oom_task_origin(task)) {
		points = ULONG_MAX;
		goto select;
	}

	points = oom_badness(task, NULL, oc->nodemask, oc->totalpages);
	if (!points || points < oc->chosen_points)
		goto next;

	/* Prefer thread group leaders for display purposes */
	if (points == oc->chosen_points && thread_group_leader(oc->chosen))
		goto next;
select:
	if (oc->chosen)
		put_task_struct(oc->chosen);
	get_task_struct(task);
	oc->chosen = task;
	oc->chosen_points = points;
next:
	return 0;
abort:
	if (oc->chosen)
		put_task_struct(oc->chosen);
	oc->chosen = (void *)-1UL;
	return 1;
}

/**
 * oom_badness - heuristic function to determine which candidate task to kill
 * @p: task struct of which task we should calculate
 * @totalpages: total present RAM allowed for page allocation
 * @memcg: task's memory controller, if constrained
 * @nodemask: nodemask passed to page allocator for mempolicy ooms
 *
 * The heuristic for determining which task to kill is made to be as simple and
 * predictable as possible.  The goal is to return the highest value for the
 * task consuming the most memory to avoid subsequent oom failures.
 */
unsigned long oom_badness(struct task_struct *p, struct mem_cgroup *memcg,
			  const nodemask_t *nodemask, unsigned long totalpages)
{
	long points;
	long adj;

	if (oom_unkillable_task(p, memcg, nodemask))
		return 0;

	p = find_lock_task_mm(p);
	if (!p)
		return 0;

	/*
	 * Do not even consider tasks which are explicitly marked oom
	 * unkillable or have been already oom reaped or the are in
	 * the middle of vfork
	 */
	adj = (long)p->signal->oom_score_adj;
	if (adj == OOM_SCORE_ADJ_MIN ||
			test_bit(MMF_OOM_SKIP, &p->mm->flags) ||
			in_vfork(p)) {
		task_unlock(p);
		return 0;
	}

	/*
	 * The baseline for the badness score is the proportion of RAM that each
	 * task's rss, pagetable and swap space use.
	 */
	points = get_mm_rss(p->mm) + get_mm_counter(p->mm, MM_SWAPENTS) +
		mm_pgtables_bytes(p->mm) / PAGE_SIZE;
	task_unlock(p);

	/* Normalize to oom_score_adj units */
	adj *= totalpages / 1000;
	points += adj;

	/*
	 * Never return 0 for an eligible task regardless of the root bonus and
	 * oom_score_adj (oom_score_adj can't be OOM_SCORE_ADJ_MIN here).
	 */
	return points > 0 ? points : 1;
}
```

### 二、查看系统日志方法：
运行`egrep -i -r 'killed process' /var/log`命令，结果如下：
```s
/var/log/syslog.1:May  6 10:02:51 iZuf66b59tpzdaxbchl3d4Z kernel: [1467990.340288] Killed process 17909 (procon) total-vm:5312000kB, anon-rss:4543100kB, file-rss:0kB
```

也可运行`dmesg`命令，结果如下： 
```s
[1471454.635492] Out of memory: Kill process 17907 (procon) score 143 or sacrifice child
[1471454.636345] Killed process 17907 (procon) total-vm:5617060kB, anon-rss:4848752kB, file-rss:0kB
```
显示可读时间的话可用`dmesg -T`查看：
```s
[Wed May 15 14:03:08 2019] Out of memory: Kill process 83446 (machine) score 250 or sacrifice child
[Wed May 15 14:03:08 2019] Killed process 83446 (machine) total-vm:1920560kB, anon-rss:1177488kB, file-rss:1600kB
```
### 三、 附录
#### 【1】附加`__alloc_pages_nodemask()`函数如下：
```c
/*
 * This is the 'heart' of the zoned buddy allocator.
 */
struct page *
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order, int preferred_nid,
							nodemask_t *nodemask)
{
	struct page *page;
	unsigned int alloc_flags = ALLOC_WMARK_LOW;
	gfp_t alloc_mask; /* The gfp_t that was actually used for allocation */
	struct alloc_context ac = { };

	/*
	 * There are several places where we assume that the order value is sane
	 * so bail out early if the request is out of bound.
	 */
	if (unlikely(order >= MAX_ORDER)) {
		WARN_ON_ONCE(!(gfp_mask & __GFP_NOWARN));
		return NULL;
	}

	gfp_mask &= gfp_allowed_mask;
	alloc_mask = gfp_mask;
	if (!prepare_alloc_pages(gfp_mask, order, preferred_nid, nodemask, &ac, &alloc_mask, &alloc_flags))
		return NULL;

	finalise_ac(gfp_mask, &ac);

	/*
	 * Forbid the first pass from falling back to types that fragment
	 * memory until all local zones are considered.
	 */
	alloc_flags |= alloc_flags_nofragment(ac.preferred_zoneref->zone, gfp_mask);

	/* First allocation attempt */
	page = get_page_from_freelist(alloc_mask, order, alloc_flags, &ac);
	if (likely(page))
		goto out;

	/*
	 * Apply scoped allocation constraints. This is mainly about GFP_NOFS
	 * resp. GFP_NOIO which has to be inherited for all allocation requests
	 * from a particular context which has been marked by
	 * memalloc_no{fs,io}_{save,restore}.
	 */
	alloc_mask = current_gfp_context(gfp_mask);
	ac.spread_dirty_pages = false;

	/*
	 * Restore the original nodemask if it was potentially replaced with
	 * &cpuset_current_mems_allowed to optimize the fast-path attempt.
	 */
	if (unlikely(ac.nodemask != nodemask))
		ac.nodemask = nodemask;

	page = __alloc_pages_slowpath(alloc_mask, order, &ac);

out:
	if (memcg_kmem_enabled() && (gfp_mask & __GFP_ACCOUNT) && page &&
	    unlikely(__memcg_kmem_charge(page, gfp_mask, order) != 0)) {
		__free_pages(page, order);
		page = NULL;
	}

	trace_mm_page_alloc(page, order, alloc_mask, ac.migratetype);

	return page;
}
```


---

最后，简单分析一下你的进程被Linux杀掉几个可能的原因：一种是内存泄露；一种是你的进程所需要的内存资源太大，系统无法满足，应该在设计时对进程需要的资源有个最大限制，不能让他无限增长；当然，也不一定全是你的问题，也有可能是同一主机的其他进程占用资源过多，但是Linux OOM选择“最坏“进程杀掉的算法是很简单粗暴的，就选中你的进程杀掉，也是有可能的。