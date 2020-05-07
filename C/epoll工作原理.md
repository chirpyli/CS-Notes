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

### epoll是如何实现的

#### epoll_creat1
我们看一下系统调用`epoll_create1`的源码实现：
```c++
/*
 * Open an eventpoll file descriptor.
 */
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

SYSCALL_DEFINE1(epoll_create, int, size) {
	if (size <= 0)
		return -EINVAL;

	return sys_epoll_create1(0);
}
```

```c++

static int ep_alloc(struct eventpoll **pep)
{
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

#### epoll_ctl
我们先看一下`epoll_ctl(EPOLL_CTL_ADD)`是如何将对应的文件描述符插入到`eventpoll`中的，其他操作原理类似。
```c++
/*
 * The following function implements the controller interface for
 * the eventpoll file that enables the insertion/removal/change of
 * file descriptors inside the interest set.
 */
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
具体实现插入的函数：
```c++
/*
 * Must be called with "mtx" held.
 */
static int ep_insert(struct eventpoll *ep, struct epoll_event *event,
		     struct file *tfile, int fd, int full_check)
{
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
