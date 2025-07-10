#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/*
change@/devices/platform/soc@3000000/7130000.e906_rproc/remoteproc/remoteproc0/remoteproc0-wdt
ACTION=change
DEVPATH=/devices/platform/soc@3000000/7130000.e906_rproc/remoteproc/remoteproc0/remoteproc0-wdt
SUBSYSTEM=rproc_wdt
EVENT=WDT_RUNING
RESET_TYPE=rst_core
TIMEOUT_MS=6000
TRY_TIMES=0
TIMEOUT_CNT=0
SEQNUM=1737
*/

#define RPROC_WDT_UEVENT_EXT_ITEM

struct uevent_msg {
	int seqnum;
	const char *sub_system;
	const char *action;
	const char *dev_path;
#ifdef RPROC_WDT_UEVENT_EXT_ITEM
	const char *event;
	const char *reset_type;
	int timeout_ms;
	int try_times;
	int timeout_cnt;
#endif
};

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof(x[0]))
#define offsetof(TYPE, MEMBER)	((size_t) &((TYPE *)0)->MEMBER)
#define TYPE_INT		(0)
#define TYPE_STR		(1)

struct match_table_t {
	unsigned short type;
	unsigned short off;
	const char *str;
};

static const struct match_table_t match_table[] = {
	{TYPE_INT, offsetof(struct uevent_msg, seqnum),		"SEQNUM="},
	{TYPE_STR, offsetof(struct uevent_msg, sub_system),	"SUBSYSTEM="},
	{TYPE_STR, offsetof(struct uevent_msg, action), 	"ACTION="},
	{TYPE_STR, offsetof(struct uevent_msg, dev_path),	"DEVPATH="},
#ifdef RPROC_WDT_UEVENT_EXT_ITEM
	{TYPE_STR, offsetof(struct uevent_msg, event),		"EVENT="},
	{TYPE_STR, offsetof(struct uevent_msg, reset_type),	"RESET_TYPE="},
	{TYPE_INT, offsetof(struct uevent_msg, timeout_ms),	"TIMEOUT_MS="},
	{TYPE_INT, offsetof(struct uevent_msg, try_times),	"TRY_TIMES="},
	{TYPE_INT, offsetof(struct uevent_msg, timeout_cnt),	"TIMEOUT_CNT="},
#endif
};

static inline int open_uevent_socket(void)
{
	struct sockaddr_nl addr;
	int sz = 64 * 1024;
	int s, ret;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xffffffff;
	//addr.nl_groups = NETLINK_KOBJECT_UEVENT;

	printf("addr.nl_groups: %lx\n", (unsigned long)addr.nl_groups);

	s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (s < 0) {
		printf("socket failed, ret: %d\n", s);
		ret = s;
		goto err_out;
	}

	ret = setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));
	if (ret) {
		printf("setsockopt failed, ret: %d\n", ret);
		goto err_out;
	}

	ret = bind(s, (struct sockaddr *) &addr, sizeof(addr));
	if (ret < 0) {
		printf("bind failed, ret: %d\n", ret);
		goto err_out;
	}

	return s;
err_out:
	if (s > 0)
		close(s);
	return ret;
}

static inline void clean_uevent(struct uevent_msg *uevent)
{
	uevent->seqnum = -1;;
	uevent->sub_system = "";
	uevent->action = "";
	uevent->dev_path = "";
#ifdef RPROC_WDT_UEVENT_EXT_ITEM
	uevent->event = "";
	uevent->reset_type = "";
	uevent->timeout_ms = -1;
	uevent->try_times = -1;
	uevent->timeout_cnt = -1;
#endif
}

static inline int parse_item(const char *msg, void *base)
{
	int i, len, off, type;
	const char *str;

	for (i = 0; i < ARRAY_SIZE(match_table); i++) {
		str = match_table[i].str;
		off = match_table[i].off;
		type = match_table[i].type;
		len = strlen(str);

		if (!strncmp(msg, str, len)) {
			msg += len;
			if (type == TYPE_INT) {
				int *ptr = off + base;
				*ptr = atoi(msg);
			} else if (type == TYPE_STR) {
				const char **ptr = off + base;
				*ptr = msg;
			}
			break;
		}
		len = 0;
	}

	return len;
}

static inline void parse_uevent(const char *msg, struct uevent_msg *uevent)
{
	while (*msg) {
		msg += parse_item(msg, uevent);
		while(*msg++);
	}
}

static inline const char *path_to_name(const char *path)
{
	const char *tmp = strchr(path, '/');

	while (tmp) {
		path = tmp + 1;
		tmp = strchr(path, '/');
	}

	return path;
}

static void show_uevent(struct uevent_msg *uevent)
{
	printf("%5d: %-32s, %-10s, %-8s",
		uevent->seqnum,
		path_to_name(uevent->dev_path),
		uevent->sub_system,
		uevent->action);
#ifdef RPROC_WDT_UEVENT_EXT_ITEM
	printf(", %-12s, (type: %-10s, timeout_ms: %6d, try_times: %3d, timeout_cnt: %3d)",
		uevent->event,
		uevent->reset_type,
		uevent->timeout_ms,
		uevent->try_times,
		uevent->timeout_cnt);
#endif
	printf("\n");
}

#define UEVENT_MSG_LEN 2048
static char msg[UEVENT_MSG_LEN+2];

int main(int argc, char* argv[])
{
	int socket_fd = -1;
	int recv_size, i;
	struct uevent_msg uevent;

	socket_fd = open_uevent_socket();
	printf("socket_fd = %d\n", socket_fd);

	if (socket_fd < 0)
		return -1;

	while (1) {
		clean_uevent(&uevent);
		memset(msg, 0, sizeof(msg));
		recv_size = recv(socket_fd, msg, UEVENT_MSG_LEN, 0);
		if (recv_size < 0) {
			if (errno == EAGAIN)
				continue;
			else
				break;
		} else if (recv_size == UEVENT_MSG_LEN) {
			printf("recv_size too long\n");
			continue;
		}

		parse_uevent(msg, &uevent);
		if (argc > 1) {
			for(i = 1; i < argc; i++) {
				if (!strcmp(argv[i], uevent.sub_system)) {
					show_uevent(&uevent);
					break;
				}
			}
		} else {
			show_uevent(&uevent);
		}
	}

	close(socket_fd);
	return 0;
}
