想弄明白epoll的源码还是挺难的，需要对Linux底层机制非常熟悉，慢慢学习吧，未完待续......
### epoll的使用
我们先看一下下面的echo服务端代码。理解epoll关键是要理解`epoll_create1`、`epoll_ctl`、`epoll_wait`这三个函数是如何实现的，我们先看他们是如何用的，下一节分析他们是怎么实现的。
```c++
/* echo server*/
#include<sys/epoll.h>
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>

#define MAX_EVENTS 1024
#define LISTEN_PORT 33333
#define MAX_BUF 1024


struct echo_data;
int setnonblocking(int sockfd);
int events_handle(int epfd, struct epoll_event ev);
void run();

int main(int _argc, char* _argv[]) {
    run();

    return 0;
}

void run() {
    int epfd = epoll_create1(0);		//【1】
    if (-1 == epfd) {
        perror("epoll_create1 failure.");
        exit(EXIT_FAILURE);
    }

    char str[INET_ADDRSTRLEN];
    struct sockaddr_in seraddr, cliaddr;
    socklen_t cliaddr_len = sizeof(cliaddr);
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(LISTEN_PORT);

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (-1 == bind(listen_sock, (struct sockaddr*)&seraddr, sizeof(seraddr))) {
        perror("bind server addr failure.");
        exit(EXIT_FAILURE);
    }
    listen(listen_sock, 5);

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev)) {	//【2】
        perror("epoll_ctl add listen_sock failure.");
        exit(EXIT_FAILURE);
    }

    int nfds = 0;
    while (1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);		//【3】
        if (-1 == nfds) {
            perror("epoll_wait failure.");
            exit(EXIT_FAILURE);
        }

        for ( int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                int conn_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &cliaddr_len);
                if (-1 == conn_sock) {
                    perror("accept failure.");
                    exit(EXIT_FAILURE);
                }
                printf("accept from %s:%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));

                setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;		//边缘触发，单线程下其实不用加EPOLLONESHOT
                ev.data.fd = conn_sock;
                if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock, &ev)) {
                    perror("epoll_ctl add conn_sock failure.");
                    exit(EXIT_FAILURE);
                }
            } else {
                events_handle(epfd, events[n]);
            }
        }
    }

    close(listen_sock);
    close(epfd);
}

int setnonblocking(int sockfd){
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

struct echo_data {
    char* data;
    int fd;
};

int events_handle(int epfd, struct epoll_event ev) {
    printf("events_handle, ev.events = %d\n", ev.events);
    int fd = ev.data.fd;
    if (ev.events & EPOLLIN) {
        char* buf = (char*)malloc(MAX_BUF);
        bzero(buf, MAX_BUF);
        int count = 0;
        int n = 0;
        /*
        边缘模式需要循环读取，但是在读取过程中，如果有新的事件到达，很可能触发了其他线程来处理这个socket，那就乱了。
        所以上面ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT; EPOLLONESHOT用来解决这个问题。
        EPOLLONESHOT的原理其实是，每次触发事件之后，就将事件注册从fd上清除了，也就不会再被追踪到；下次需要用epoll_ctl的EPOLL_CTL_MOD来手动加上才行。
        */    
        while (1) {
            n = read(fd, (buf + count), 5);
            printf("step in edge_trigger, read bytes:%d\n", n);
            if (n > 0) {
                count += n;
            } else if (0 == n) {
                break;
            } else if (n < 0 && EAGAIN == errno) {
                printf("errno == EAGAIN, break.\n");
                break;
            } else {
                perror("read failure.");
                exit(EXIT_FAILURE);
            }
        }

        if (0 == count) {
            if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev)) {
                perror("epoll_ctl del fd failure.");
                exit(EXIT_FAILURE);
            }
            close(fd);

            return 0;
        }

        printf("recv from client: %s\n", buf);
        struct echo_data* ed = (struct echo_data*)malloc(sizeof(struct echo_data));
        ed->data = buf;
        ed->fd = fd;
        ev.data.ptr = ed;
        ev.events = EPOLLOUT | EPOLLET;
        if (-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev)) {
            perror("epoll_ctl modify fd failure.");
            exit(EXIT_FAILURE);
        }

        return 0;
    } else if (ev.events & EPOLLOUT) {
        struct echo_data* data = (struct echo_data*)ev.data.ptr;
        printf("write data to client: %s", data->data);
        int ret = 0;
        int send_pos = 0;
        const int total = strlen(data->data);
        char* send_buf = data->data;
        while(1) {
            ret = write(data->fd, (send_buf + send_pos), total - send_pos);
            if (ret < 0) {
                if (EAGAIN == errno) {
                    sched_yield();
                    continue;
                }
                perror("write failure.");
                exit(EXIT_FAILURE);
            }
            send_pos += ret;
            if (total == send_pos) {
                break;
            }
        }

        ev.data.fd = data->fd;
        ev.events = EPOLLIN | EPOLLET;
        if (-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, data->fd, &ev)) {
            perror("epoll_ctl modify fd failure.");
            exit(EXIT_FAILURE);
        }

        free(data->data);
        free(data);
    }

    return 0;
}
```

### 理解epoll的预备知识
首先是理解Linux的文件系统，经常听到Linux一且皆文件这句话，不理解文件系统的话，epoll真没法往下学了。

我们先看一下Linux中`flie`的定义：
```c++
struct file {
	union {
		struct llist_node	fu_llist;
		struct rcu_head 	fu_rcuhead;
	} f_u;
	struct path		f_path;
	struct inode		*f_inode;	/* cached value */
	const struct file_operations	*f_op;		// 抽象了文件的操作

	/*
	 * Protects f_ep_links, f_flags.
	 * Must not be taken from IRQ context.
	 */
	spinlock_t		f_lock;
	atomic_long_t		f_count;
	unsigned int 		f_flags;
	fmode_t			f_mode;
	struct mutex		f_pos_lock;
	loff_t			f_pos;
	struct fown_struct	f_owner;
	const struct cred	*f_cred;
	struct file_ra_state	f_ra;

	u64			f_version;
#ifdef CONFIG_SECURITY
	void			*f_security;
#endif
	/* needed for tty driver, and maybe others */
	void			*private_data;

#ifdef CONFIG_EPOLL
	/* Used by fs/eventpoll.c to link all the hooks to this file */
	struct list_head	f_ep_links;
	struct list_head	f_tfile_llink;
#endif /* #ifdef CONFIG_EPOLL */
	struct address_space	*f_mapping;
} __attribute__((aligned(4)));	/* lest something weird decides that 2 is OK */
```
其中最重要的理解其文件的抽象，其实就相当于c++中的抽象类，用面向过程的方式实现面向对象的思想。对后面比较重要的是`poll`这个抽象，后面会用到。
```c++
struct file_operations {
	struct module *owner;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iterate) (struct file *, struct dir_context *);
	unsigned int (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*aio_fsync) (struct kiocb *, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	int (*setlease)(struct file *, long, struct file_lock **, void **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,
			  loff_t len);
	void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
	unsigned (*mmap_capabilities)(struct file *);
#endif
};
```

### epoll是如何实现的

#### epoll_creat1
我们看一下系统调用`epoll_create1`的源码实现：
```c++
// Open an eventpoll file descriptor.
SYSCALL_DEFINE1(epoll_create1, int, flags) {
	int error, fd;
	struct eventpoll *ep = NULL;
	struct file *file;

	/* Check the EPOLL_* constant for consistency.  */
	BUILD_BUG_ON(EPOLL_CLOEXEC != O_CLOEXEC);

	if (flags & ~EPOLL_CLOEXEC)
		return -EINVAL;
	/*
	 * Create the internal data structure ("struct eventpoll").
	 */
	error = ep_alloc(&ep);      // 创建eventpoll
	if (error < 0)
		return error;
	/*
	 * Creates all the items needed to setup an eventpoll file. That is,
	 * a file structure and a free file descriptor.
	 */
	fd = get_unused_fd_flags(O_RDWR | (flags & O_CLOEXEC));   // 获取尚未被使用的文件描述符，即描述符数组的槽位
	if (fd < 0) {
		error = fd;
		goto out_free_ep;
	}
	file = anon_inode_getfile("[eventpoll]", &eventpoll_fops, ep, O_RDWR | (flags & O_CLOEXEC));     // 在匿名inode文件系统中分配一个inode,并得到其file结构体
	if (IS_ERR(file)) {
		error = PTR_ERR(file);
		goto out_free_fd;
	}
	ep->file = file;
	fd_install(fd, file);   // 将file填入到对应的文件描述符数组的槽里面
	return fd;

out_free_fd:
	put_unused_fd(fd);
out_free_ep:
	ep_free(ep);
	return error;
}

static int ep_alloc(struct eventpoll **pep) {
	int error;
	struct user_struct *user;
	struct eventpoll *ep;

	user = get_current_user();
	error = -ENOMEM;
	ep = kzalloc(sizeof(*ep), GFP_KERNEL);  // 用的是内核空间
	if (unlikely(!ep))
		goto free_uid;

	spin_lock_init(&ep->lock);
	mutex_init(&ep->mtx);
	init_waitqueue_head(&ep->wq);
	init_waitqueue_head(&ep->poll_wait);
	INIT_LIST_HEAD(&ep->rdllist);
	ep->rbr = RB_ROOT;
	ep->ovflist = EP_UNACTIVE_PTR;
	ep->user = user;

	*pep = ep;

	return 0;

free_uid:
	free_uid(user);
	return error;
}
```
上面的过程太复杂了，我们简化一下，只保留核心调用，便于理解：
```c++
int epoll_create(int size) {
	struct eventpoll *ep = NULL;
	struct file *file;
	ep_alloc(&ep); // 创建eventpoll
	// Creates all the items needed to setup an eventpoll file. That is a file structure and a free file descriptor.
	fd = get_unused_fd_flags(O_RDWR | (flags & O_CLOEXEC));	// 获取尚未被使用的文件描述符，即描述符数组的槽位
	file = anon_inode_getfile("[eventpoll]", &eventpoll_fops, ep, O_RDWR | (flags & O_CLOEXEC));	// 在匿名inode文件系统中分配一个inode,并得到其file结构体
	ep->file = file;
	fd_install(fd, file);	// 将file填入到对应的文件描述符数组的槽里面
}
```
上面的整个调用，比较核心的就是创建下面这个`eventpoll`对象，里面有个红黑树，用于后面添加、删除、修改需要epoll监控的文件描述符。
```c++
/*
 * This structure is stored inside the "private_data" member of the file
 * structure and represents the main data structure for the eventpoll
 * interface.
 */
struct eventpoll {
	/* Protect the access to this structure */
	spinlock_t lock;

	/*
	 * This mutex is used to ensure that files are not removed
	 * while epoll is using them. This is held during the event
	 * collection loop, the file cleanup path, the epoll file exit
	 * code and the ctl operations.
	 */
	struct mutex mtx;

	/* Wait queue used by sys_epoll_wait() */
	wait_queue_head_t wq;

	/* Wait queue used by file->poll() */
	wait_queue_head_t poll_wait;

	/* List of ready file descriptors */
	struct list_head rdllist;   // 就绪列表

	/* RB tree root used to store monitored fd structs */
	struct rb_root rbr;     // 这里是一颗红黑树, 通过红黑树来组织当前epoll关注的文件描述符

	/*
	 * This is a single linked list that chains all the "struct epitem" that
	 * happened while transferring ready events to userspace w/out
	 * holding ->lock.
	 */
	struct epitem *ovflist;

	/* wakeup_source used when ep_scan_ready_list is running */
	struct wakeup_source *ws;

	/* The user that created the eventpoll descriptor */

	struct file *file;

	/* used to optimize loop detection check */
	int visited;
	struct list_head visited_list_link;
};
```
>参考文档：[The Implementation of epoll (1)](https://idndx.com/2014/09/01/the-implementation-of-epoll-1/)


#### epoll_ctl
上面创建完epoll实例后，面临一个问题，epoll是如何记住需要监控哪些文件描述符呢？ 上面讲到epoll中有颗红黑树，需要将待监控的文件描述符插入到红黑树中。

我们先看一下`epoll_ctl(EPOLL_CTL_ADD)`是如何将对应的文件描述符插入到`eventpoll`中的，其他操作原理类似。
```c++
// The following function implements the controller interface for the eventpoll file that enables the insertion/removal/change of file descriptors inside the interest set.
SYSCALL_DEFINE4(epoll_ctl, int, epfd, int, op, int, fd, struct epoll_event __user *, event) {
	int error;
	int full_check = 0;
	struct fd f, tf;
	struct eventpoll *ep;
	struct epitem *epi;
	struct epoll_event epds;
	struct eventpoll *tep = NULL;

	error = -EFAULT;
	if (ep_op_has_event(op) &&
	    copy_from_user(&epds, event, sizeof(struct epoll_event)))
		goto error_return;

	error = -EBADF;
	f = fdget(epfd);
	if (!f.file)
		goto error_return;

	/* Get the "struct file *" for the target file */
	tf = fdget(fd);
	if (!tf.file)
		goto error_fput;

	/* The target file descriptor must support poll */
	error = -EPERM;
	if (!tf.file->f_op->poll)
		goto error_tgt_fput;

	/* Check if EPOLLWAKEUP is allowed */
	if (ep_op_has_event(op))
		ep_take_care_of_epollwakeup(&epds);

	/*
	 * We have to check that the file structure underneath the file descriptor
	 * the user passed to us _is_ an eventpoll file. And also we do not permit
	 * adding an epoll file descriptor inside itself.
	 */
	error = -EINVAL;
	if (f.file == tf.file || !is_file_epoll(f.file))
		goto error_tgt_fput;

	/*
	 * At this point it is safe to assume that the "private_data" contains
	 * our own data structure.
	 */
	ep = f.file->private_data;

	/*
	 * When we insert an epoll file descriptor, inside another epoll file
	 * descriptor, there is the change of creating closed loops, which are
	 * better be handled here, than in more critical paths. While we are
	 * checking for loops we also determine the list of files reachable
	 * and hang them on the tfile_check_list, so we can check that we
	 * haven't created too many possible wakeup paths.
	 *
	 * We do not need to take the global 'epumutex' on EPOLL_CTL_ADD when
	 * the epoll file descriptor is attaching directly to a wakeup source,
	 * unless the epoll file descriptor is nested. The purpose of taking the
	 * 'epmutex' on add is to prevent complex toplogies such as loops and
	 * deep wakeup paths from forming in parallel through multiple
	 * EPOLL_CTL_ADD operations.
	 */
	mutex_lock_nested(&ep->mtx, 0);
	if (op == EPOLL_CTL_ADD) {
		if (!list_empty(&f.file->f_ep_links) ||
						is_file_epoll(tf.file)) {
			full_check = 1;
			mutex_unlock(&ep->mtx);
			mutex_lock(&epmutex);
			if (is_file_epoll(tf.file)) {
				error = -ELOOP;
				if (ep_loop_check(ep, tf.file) != 0) {
					clear_tfile_check_list();
					goto error_tgt_fput;
				}
			} else
				list_add(&tf.file->f_tfile_llink,
							&tfile_check_list);
			mutex_lock_nested(&ep->mtx, 0);
			if (is_file_epoll(tf.file)) {
				tep = tf.file->private_data;
				mutex_lock_nested(&tep->mtx, 1);
			}
		}
	}

	/*
	 * Try to lookup the file inside our RB tree, Since we grabbed "mtx"
	 * above, we can be sure to be able to use the item looked up by
	 * ep_find() till we release the mutex.
	 */
	epi = ep_find(ep, tf.file, fd);

	error = -EINVAL;
	switch (op) {
	case EPOLL_CTL_ADD:
		if (!epi) {
			epds.events |= POLLERR | POLLHUP;
			error = ep_insert(ep, &epds, tf.file, fd, full_check);      // 插入到红黑树中
		} else
			error = -EEXIST;
		if (full_check)
			clear_tfile_check_list();
		break;
	case EPOLL_CTL_DEL:
		if (epi)
			error = ep_remove(ep, epi);
		else
			error = -ENOENT;
		break;
	case EPOLL_CTL_MOD:
		if (epi) {
			epds.events |= POLLERR | POLLHUP;
			error = ep_modify(ep, epi, &epds);
		} else
			error = -ENOENT;
		break;
	}
	if (tep != NULL)
		mutex_unlock(&tep->mtx);
	mutex_unlock(&ep->mtx);

error_tgt_fput:
	if (full_check)
		mutex_unlock(&epmutex);

	fdput(tf);
error_fput:
	fdput(f);
error_return:

	return error;
}
```
上面代码太复杂，我们简化一下，只保留关键部分代码：
```c++
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
	struct fd f, tf;
	struct eventpoll *ep;
	struct epitem *epi;
	struct epoll_event epds;
	struct eventpoll *tep = NULL;

	// Try to lookup the file inside our RB tree, Since we grabbed "mtx" above, we can be sure to be able to use the item looked up by ep_find() till we release the mutex.
	epi = ep_find(ep, tf.file, fd);

	error = -EINVAL;
	switch (op) {
	case EPOLL_CTL_ADD:
		if (!epi) {
			epds.events |= POLLERR | POLLHUP;	// 在这里可以看到即使你没有指定监控EPOLLERR和EPOLLHUP，系统在这里也默认设置为监控的。
			error = ep_insert(ep, &epds, tf.file, fd, full_check);      // 插入到红黑树中
		} else
			error = -EEXIST;
		if (full_check)
			clear_tfile_check_list();
		break;
	case EPOLL_CTL_DEL:
		if (epi)
			error = ep_remove(ep, epi);
		else
			error = -ENOENT;
		break;
	case EPOLL_CTL_MOD:
		if (epi) {
			epds.events |= POLLERR | POLLHUP;
			error = ep_modify(ep, epi, &epds);
		} else
			error = -ENOENT;
		break;
	}

	return error;
}	
}
```
>EPOLLWAKEUP : 如果EPOLLONESHOT和EPOLLET清除了，并且进程拥有CAP_BLOCK_SUSPEND权限，那么这个标志能够保证事件在挂起或者处理的时候，系统不会挂起或休眠

具体实现插入的函数`ep_insert`，主要是将新的文件描述符插入到红黑树中，并设置相关的数据结构以及回调以接收事件通知。这个函数是epoll机制中最核心的实现之一，理解这一部分是非常非常重要的。而要理解这个函数，最关键的是要理解epoll是如何从监控的文件中接收事件的。

主要内容如下：
- 初始化`epitem`
- 注册设备驱动`poll`的回调函数`ep_ptable_queue_proc`，当调用`f_op->poll()`时，最终会调用该回调函数`ep_ptable_queue_proc()`
	- 在ep_ptable_queue_proc回调函数中，注册回调函数ep_poll_callback，ep_poll_callback表示当描述符fd上相应的事件发生时该如何告知进程。
	- 在ep_ptable_queue_proc回调函数中，检测是文件描述符fd对应的设备的epoll_event事件是否发生，如果发生则把fd及其epoll_event加入上面提到的就绪队列rdlist中
- 把当前文件描述符及其对应的事件（fd,epoll_event）加入到eventpoll中的红黑树中。

```c++
static int ep_insert(struct eventpoll *ep, struct epoll_event *event, struct file *tfile, int fd, int full_check) {
	int error, revents, pwake = 0;
	unsigned long flags;
	long user_watches;
	struct epitem *epi;
	struct ep_pqueue epq;

	user_watches = atomic_long_read(&ep->user->epoll_watches);
	if (unlikely(user_watches >= max_user_watches))
		return -ENOSPC;
	if (!(epi = kmem_cache_alloc(epi_cache, GFP_KERNEL)))
		return -ENOMEM;

	/* Item initialization follow here ... */
	INIT_LIST_HEAD(&epi->rdllink);
	INIT_LIST_HEAD(&epi->fllink);
	INIT_LIST_HEAD(&epi->pwqlist);
	epi->ep = ep;
	ep_set_ffd(&epi->ffd, tfile, fd);
	epi->event = *event;
	epi->nwait = 0;
	epi->next = EP_UNACTIVE_PTR;
	if (epi->event.events & EPOLLWAKEUP) {
		error = ep_create_wakeup_source(epi);
		if (error)
			goto error_create_wakeup_source;
	} else {
		RCU_INIT_POINTER(epi->ws, NULL);
	}

	/* Initialize the poll table using the queue callback */
	epq.epi = epi;
	init_poll_funcptr(&epq.pt, ep_ptable_queue_proc);

	/*
	 * Attach the item to the poll hooks and get current event bits.
	 * We can safely use the file* here because its usage count has
	 * been increased by the caller of this function. Note that after
	 * this operation completes, the poll callback can start hitting
	 * the new item.
	 */
	revents = ep_item_poll(epi, &epq.pt);

	/*
	 * We have to check if something went wrong during the poll wait queue
	 * install process. Namely an allocation for a wait queue failed due
	 * high memory pressure.
	 */
	error = -ENOMEM;
	if (epi->nwait < 0)
		goto error_unregister;

	/* Add the current item to the list of active epoll hook for this file */
	spin_lock(&tfile->f_lock);
	list_add_tail_rcu(&epi->fllink, &tfile->f_ep_links);
	spin_unlock(&tfile->f_lock);

	/*
	 * Add the current item to the RB tree. All RB tree operations are
	 * protected by "mtx", and ep_insert() is called with "mtx" held.
	 */
	ep_rbtree_insert(ep, epi); // 将epitem插入红黑树

	/* now check if we've created too many backpaths */
	error = -EINVAL;
	if (full_check && reverse_path_check())
		goto error_remove_epi;

	/* We have to drop the new item inside our item list to keep track of it */
	spin_lock_irqsave(&ep->lock, flags);

	/* If the file is already "ready" we drop it inside the ready list */
	if ((revents & event->events) && !ep_is_linked(&epi->rdllink)) {
		list_add_tail(&epi->rdllink, &ep->rdllist);
		ep_pm_stay_awake(epi);

		/* Notify waiting tasks that events are available */
		if (waitqueue_active(&ep->wq))
			wake_up_locked(&ep->wq);
		if (waitqueue_active(&ep->poll_wait))
			pwake++;
	}

	spin_unlock_irqrestore(&ep->lock, flags);

	atomic_long_inc(&ep->user->epoll_watches);

	/* We have to call this outside the lock */
	if (pwake)
		ep_poll_safewake(&ep->poll_wait);

	return 0;

error_remove_epi:
	spin_lock(&tfile->f_lock);
	list_del_rcu(&epi->fllink);
	spin_unlock(&tfile->f_lock);

	rb_erase(&epi->rbn, &ep->rbr);

error_unregister:
	ep_unregister_pollwait(ep, epi);

	/*
	 * We need to do this because an event could have been arrived on some
	 * allocated wait queue. Note that we don't care about the ep->ovflist
	 * list, since that is used/cleaned only inside a section bound by "mtx".
	 * And ep_insert() is called with "mtx" held.
	 */
	spin_lock_irqsave(&ep->lock, flags);
	if (ep_is_linked(&epi->rdllink))
		list_del_init(&epi->rdllink);
	spin_unlock_irqrestore(&ep->lock, flags);

	wakeup_source_unregister(ep_wakeup_source(epi));

error_create_wakeup_source:
	kmem_cache_free(epi_cache, epi);

	return error;
}
```
上面代码太多了，同样的，我们精简一下，重点在回调函数这块，
```c++
static int ep_insert(struct eventpoll *ep, struct epoll_event *event, struct file *tfile, int fd, int full_check) {
	struct epitem *epi;		// 创建epitem，待插入到红黑树的节点
	struct ep_pqueue epq;

	if (!(epi = kmem_cache_alloc(epi_cache, GFP_KERNEL)))
		return -ENOMEM;

	/* Item initialization follow here ... */  //初始化
	INIT_LIST_HEAD(&epi->rdllink);
	INIT_LIST_HEAD(&epi->fllink);
	INIT_LIST_HEAD(&epi->pwqlist);
	epi->ep = ep;
	ep_set_ffd(&epi->ffd, tfile, fd);
	epi->event = *event;
	epi->nwait = 0;
	epi->next = EP_UNACTIVE_PTR;

	// 注册回调函数ep_ptable_queue_proc
	epq.epi = epi;
	init_poll_funcptr(&epq.pt, ep_ptable_queue_proc);
	ep_item_poll(epi, &epq.pt);

	// Add the current item to the RB tree. 
	ep_rbtree_insert(ep, epi); // 将epitem插入红黑树
}

static inline unsigned int ep_item_poll(struct epitem *epi, poll_table *pt) {
	pt->_key = epi->event.events;
	// 这里要执行的是文件的抽象操作poll操作，根据具体传入的参数，不同的文件实例有其各自的实现
	// 可以看到，例如TCP在执行poll时，会传入这里注册的回调函数，ep_ptable_queue_proc。
	return epi->ffd.file->f_op->poll(epi->ffd.file, pt) & epi->event.events;
}

// 这里可以看到，我们之前注册的回调函数会在这里p->_qproc()执行
static inline void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p) {
	if (p && p->_qproc && wait_address)
		p->_qproc(filp, wait_address, p);
}

// 具体的文件类型比如TCP socket，在实现poll-->tcp_poll-->sock_poll_wait-->poll_wait(),都会调用到
static inline void sock_poll_wait(struct file *filp,
		wait_queue_head_t *wait_address, poll_table *p) {
	if (!poll_does_not_wait(p) && wait_address) {
		poll_wait(filp, wait_address, p);
		/* We need to be sure we are in sync with the
		 * socket flags modification.
		 *
		 * This memory barrier is paired in the wq_has_sleeper.
		 */
		smp_mb();
	}
}

// structures and helpers for f_op->poll implementations
typedef void (*poll_queue_proc)(struct file *, wait_queue_head_t *, struct poll_table_struct *);

/*
 * Do not touch the structure directly, use the access functions
 * poll_does_not_wait() and poll_requested_events() instead.
 */
typedef struct poll_table_struct {
	poll_queue_proc _qproc;
	unsigned long _key;
} poll_table;

// This is the callback that is used to add our wait queue to the target file wakeup lists.
static void ep_ptable_queue_proc(struct file *file, wait_queue_head_t *whead poll_table *pt) {
	struct epitem *epi = ep_item_from_epqueue(pt);
	struct eppoll_entry *pwq;

	if (epi->nwait >= 0 && (pwq = kmem_cache_alloc(pwq_cache, GFP_KERNEL))) {
		init_waitqueue_func_entry(&pwq->wait, ep_poll_callback);
		pwq->whead = whead;
		pwq->base = epi;
		add_wait_queue(whead, &pwq->wait);
		list_add_tail(&pwq->llink, &epi->pwqlist);
		epi->nwait++;
	} else {
		/* We have to signal that an error occurred */
		epi->nwait = -1;
	}
}

static void ep_rbtree_insert(struct eventpoll *ep, struct epitem *epi) {
	int kcmp;
	struct rb_node **p = &ep->rbr.rb_node, *parent = NULL;
	struct epitem *epic;

	while (*p) {
		parent = *p;
		epic = rb_entry(parent, struct epitem, rbn);
		kcmp = ep_cmp_ffd(&epi->ffd, &epic->ffd);
		if (kcmp > 0)
			p = &parent->rb_right;
		else
			p = &parent->rb_left;
	}
	rb_link_node(&epi->rbn, parent, p);
	rb_insert_color(&epi->rbn, &ep->rbr);
}
```
>可参考[The Implementation of epoll (2)](https://idndx.com/2014/09/03/the-implementation-of-epoll-2/)。
上面有讲到`epoll_ctl`中会注册回调函数`ep_ptable_queue_proc`，而回调函数中还有一个回调函数`ep_poll_callback`。
```c++
// This is the callback that is passed to the wait queue wakeup mechanism. It is called by the stored file descriptors when they have events to report.
static int ep_poll_callback(wait_queue_t *wait, unsigned mode, int sync, void *key) {
	int pwake = 0;
	unsigned long flags;
	struct epitem *epi = ep_item_from_wait(wait);
	struct eventpoll *ep = epi->ep;

	spin_lock_irqsave(&ep->lock, flags);

	/*
	 * If the event mask does not contain any poll(2) event, we consider the
	 * descriptor to be disabled. This condition is likely the effect of the
	 * EPOLLONESHOT bit that disables the descriptor when an event is received,
	 * until the next EPOLL_CTL_MOD will be issued.
	 */
	if (!(epi->event.events & ~EP_PRIVATE_BITS))
		goto out_unlock;

	/*
	 * Check the events coming with the callback. At this stage, not
	 * every device reports the events in the "key" parameter of the
	 * callback. We need to be able to handle both cases here, hence the
	 * test for "key" != NULL before the event match test.
	 */
	if (key && !((unsigned long) key & epi->event.events))
		goto out_unlock;

	/*
	 * If we are transferring events to userspace, we can hold no locks
	 * (because we're accessing user memory, and because of linux f_op->poll()
	 * semantics). All the events that happen during that period of time are
	 * chained in ep->ovflist and requeued later on.
	 */
	if (ep->ovflist != EP_UNACTIVE_PTR) {
		if (epi->next == EP_UNACTIVE_PTR) {
			epi->next = ep->ovflist;
			ep->ovflist = epi;
			if (epi->ws) {
				/*
				 * Activate ep->ws since epi->ws may get
				 * deactivated at any time.
				 */
				__pm_stay_awake(ep->ws);
			}

		}
		goto out_unlock;
	}

	/* If this file is already in the ready list we exit soon */
	if (!ep_is_linked(&epi->rdllink)) {
		list_add_tail(&epi->rdllink, &ep->rdllist);
		ep_pm_stay_awake_rcu(epi);
	}

	/*
	 * Wake up ( if active ) both the eventpoll wait list and the ->poll()
	 * wait list.
	 */
	if (waitqueue_active(&ep->wq))
		wake_up_locked(&ep->wq);
	if (waitqueue_active(&ep->poll_wait))
		pwake++;

out_unlock:
	spin_unlock_irqrestore(&ep->lock, flags);

	/* We have to call this outside the lock */
	if (pwake)
		ep_poll_safewake(&ep->poll_wait);


	if ((unsigned long)key & POLLFREE) {
		/*
		 * If we race with ep_remove_wait_queue() it can miss
		 * ->whead = NULL and do another remove_wait_queue() after
		 * us, so we can't use __remove_wait_queue().
		 */
		list_del_init(&wait->task_list);
		/*
		 * ->whead != NULL protects us from the race with ep_free()
		 * or ep_remove(), ep_remove_wait_queue() takes whead->lock
		 * held by the caller. Once we nullify it, nothing protects
		 * ep/epi or even wait.
		 */
		smp_store_release(&ep_pwq_from_wait(wait)->whead, NULL);
	}

	return 1;
}
```
到这里了，深深的体会到完全理解`epoll`的代码好难呀，上面`ep_poll_callback`函数核心功能是将被epoll监控的fd的就绪事件到来时，将fd对应的epitem实例添加到就绪队列。当应用调用`epoll_wait`时，内核会将就绪队列中的事件报告给应用。
>可参考[The Implementation of epoll (3)](https://idndx.com/2014/09/23/the-implementation-of-epoll-3/)

#### epoll_wait
我们先看一下它的源码：
```c++
// Implement the event wait interface for the eventpoll file. It is the kernel part of the user space epoll_wait(2).
SYSCALL_DEFINE4(epoll_wait, int, epfd, struct epoll_event __user *, events, int, maxevents, int, timeout) {
	int error;
	struct fd f;
	struct eventpoll *ep;

	/* The maximum number of event must be greater than zero */
	if (maxevents <= 0 || maxevents > EP_MAX_EVENTS)
		return -EINVAL;

	/* Verify that the area passed by the user is writeable */
	if (!access_ok(VERIFY_WRITE, events, maxevents * sizeof(struct epoll_event)))
		return -EFAULT;

	/* Get the "struct file *" for the eventpoll file */
	f = fdget(epfd);
	if (!f.file)
		return -EBADF;

	/*
	 * We have to check that the file structure underneath the fd
	 * the user passed to us _is_ an eventpoll file.
	 */
	error = -EINVAL;
	if (!is_file_epoll(f.file))
		goto error_fput;

	/*
	 * At this point it is safe to assume that the "private_data" contains
	 * our own data structure.
	 */
	ep = f.file->private_data;

	/* Time to fish for events ... */
	error = ep_poll(ep, events, maxevents, timeout);		// 实际干事情的

error_fput:
	fdput(f);
	return error;
}

/**
 * ep_poll - Retrieves ready events, and delivers them to the caller supplied
 *           event buffer.
 *
 * @ep: Pointer to the eventpoll context.
 * @events: Pointer to the userspace buffer where the ready events should be
 *          stored.
 * @maxevents: Size (in terms of number of events) of the caller event buffer.
 * @timeout: Maximum timeout for the ready events fetch operation, in
 *           milliseconds. If the @timeout is zero, the function will not block,
 *           while if the @timeout is less than zero, the function will block
 *           until at least one event has been retrieved (or an error
 *           occurred).
 *
 * Returns: Returns the number of ready events which have been fetched, or an
 *          error code, in case of error.
 */
static int ep_poll(struct eventpoll *ep, struct epoll_event __user *events, int maxevents, long timeout) {
	int res = 0, eavail, timed_out = 0;
	unsigned long flags;
	long slack = 0;
	wait_queue_t wait;
	ktime_t expires, *to = NULL;

	if (timeout > 0) {
		struct timespec end_time = ep_set_mstimeout(timeout);

		slack = select_estimate_accuracy(&end_time);
		to = &expires;
		*to = timespec_to_ktime(end_time);
	} else if (timeout == 0) {
		/*
		 * Avoid the unnecessary trip to the wait queue loop, if the
		 * caller specified a non blocking operation.
		 */
		timed_out = 1;
		spin_lock_irqsave(&ep->lock, flags);
		goto check_events;
	}

fetch_events:
	spin_lock_irqsave(&ep->lock, flags);

	if (!ep_events_available(ep)) {
		/*
		 * We don't have any available event to return to the caller.
		 * We need to sleep here, and we will be wake up by
		 * ep_poll_callback() when events will become available.
		 */
		init_waitqueue_entry(&wait, current);
		__add_wait_queue_exclusive(&ep->wq, &wait);

		for (;;) {
			/*
			 * We don't want to sleep if the ep_poll_callback() sends us
			 * a wakeup in between. That's why we set the task state
			 * to TASK_INTERRUPTIBLE before doing the checks.
			 */
			set_current_state(TASK_INTERRUPTIBLE);
			if (ep_events_available(ep) || timed_out)
				break;
			if (signal_pending(current)) {
				res = -EINTR;
				break;
			}

			spin_unlock_irqrestore(&ep->lock, flags);
			if (!schedule_hrtimeout_range(to, slack, HRTIMER_MODE_ABS))
				timed_out = 1;

			spin_lock_irqsave(&ep->lock, flags);
		}

		__remove_wait_queue(&ep->wq, &wait);
		__set_current_state(TASK_RUNNING);
	}
check_events:
	/* Is it worth to try to dig for events ? */
	eavail = ep_events_available(ep);

	spin_unlock_irqrestore(&ep->lock, flags);

	/*
	 * Try to transfer events to user space. In case we get 0 events and
	 * there's still timeout left over, we go trying again in search of
	 * more luck.
	 */
	if (!res && eavail &&
	    !(res = ep_send_events(ep, events, maxevents)) && !timed_out)
		goto fetch_events;

	return res;
}
```

>可参考[The Implementation of epoll (4)](https://idndx.com/2015/07/08/the-implementation-of-epoll-4/)