#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <libgen.h>

#define IMAGEDIR		"/sdcard/.securesd"
#define FSTAB				"/sdcard/.securesd/fstab"
#define CRYPTSETUP	"/system/xbin/cryptsetup"
#define BUSYBOX			"/system/xbin/busybox"
#define KEY					"/data/system/cryptsetup.key"
#define DEBUG				(0)

struct fstab_entry
{
	char *image;
	char *mountpoint;
	char *opts;
};

static char image_dir[PATH_MAX];
static struct fstab_entry fstab[32];
static int fstab_entries = 0;

#if (DEBUG == 1)
static void debug_print(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
#else
#define debug_print(fmt, ...) (void)0
#endif

/*
 * wait_for_mount() -- wait for the sdcard to be mounted
 */
static void wait_for_mount(int seconds)
{
	struct stat st;
	while (seconds == -1 || seconds-- > 0)
	{
		if (stat(IMAGEDIR, &st) != -1)
			break;
		sleep(1);
	}
}

/*
 * fs_readline() -- reads a line from a file descriptor
 */
static char *fd_readline(int fd)
{
	int i = 0;
	char c;
	static char buf[BUFSIZ];
	while (read(fd, &c, 1) == 1)
	{
		if (c == '\n')
			break;
		buf[i++] = c;
		if (i == BUFSIZ - 1)
			break;
	}
	buf[i] = '\0';
	if (i == 0)
		return NULL;
	return buf;
}

/*
 * parse_fstab_entry() -- parse an fstab entry
 */
static int parse_fstab_entry(char *entry, char **image, char **mountpoint, char **opts)
{
	while (strchr(" \t\n", *entry))
		entry++;
	if (*entry == '\n')
		return -1;
	if (*entry == '#')
		return -2;
	*image = entry;
	while (!strchr(" \t\n", *entry))
		entry++;
	if (*entry == '\n')
		return -1;
	*entry++ = '\0';
	
	while (strchr(" \t\n", *entry))
		entry++;
	if (*entry == '\n')
		return -1;
	*mountpoint = entry;
	while (!strchr(" \t\n", *entry))
		entry++;
	if (*entry == '\n')
	{
		*entry = '\0';
		*opts = NULL;
		return 0;
	}
	*entry++ = '\0';

	while (strchr(" \t\n", *entry))
		entry++;
	if (*entry == '\n')
	{
		*opts = NULL;
	}
	else
	{
		*opts = entry;
		while (!strchr(" \t\n", *entry))
			entry++;
		*entry = '\0';
	}
	return 0;
}

/*
 * parse_fstab() -- parse fstab file
 */
static int parse_fstab()
{
	int fd;
	char *fstab_line;

	if ((fd = open(FSTAB, O_RDONLY)) == -1)
	{
		fprintf(stderr, "securesd: Cannot read fstab\n");
		return -1;
	}

	fstab_entries = 0;

	while ((fstab_line = fd_readline(fd)) != NULL)
	{
		int ret;
		struct fstab_entry *cur;
		char *image, *mountpoint, *opts;
		if ((ret = parse_fstab_entry(fstab_line, &image, &mountpoint, &opts)) < 0)
		{
			if (ret == -2)
				continue;
			close(fd);
			return -1;
		}
		cur = &fstab[fstab_entries++];
		cur->image = strdup(image);
		cur->mountpoint = strdup(mountpoint);
		cur->opts = strdup(opts);
		if (cur->image == NULL || cur->mountpoint == NULL ||
			(cur->opts == NULL && opts != NULL))
		{
			fprintf(stderr, "securesd: out of memory\n");
			return -1;
		}
	}
	close(fd);
	return 0;
}

/*
 * hide_imagedir() -- hides the securesd imagedir
 */
static void hide_imagedir()
{
	char cmd[BUFSIZ];
	snprintf(cmd, BUFSIZ, "%s mount -o ro,relatime,size=0k,mode=000 -t tmpfs tmpfs %s",
		BUSYBOX, image_dir);
	debug_print("%s", cmd);
	if (system(cmd))
	{
		debug_print("securesd: command '%s' failed", cmd);
	}
}

/*
 * unhide_imagedir() -- unhides the securesd imagedir
 */
static void unhide_imagedir()
{
	int fd, hidden;
	char *mount_entry;
	char cmd[BUFSIZ];
	if ((fd = open("/proc/mounts", O_RDONLY)) == -1)
	{
		debug_print("securesd: Could not open /proc/mounts");
		return;
	}
	do
	{
		hidden = 0;
		if (lseek(fd, 0, SEEK_SET) == -1)
		{
			debug_print("securesd: lseek() failed");
			break;
		}
		while ((mount_entry = fd_readline(fd)) != NULL)
		{
			if (strstr(mount_entry, image_dir))
			{
				hidden = 1;
				snprintf(cmd, BUFSIZ, "%s umount %s", BUSYBOX, image_dir);
				debug_print("%s", cmd);
				if (system(cmd))
				{
					debug_print("securesd: command '%s' failed", cmd);
					break;
				}
			}
		}
	}
	while (hidden);
	close(fd);
}

/*
 * mount_image() -- mounts an image file
 */
static int mount_image(const char *image, const char *mountpoint, const char *opts)
{
	char cmd[BUFSIZ];
	char *image_name, *tmp;

	if ((tmp = strdup(image)) == NULL)
	{
		fprintf(stderr, "securesu: Out of memory\n");
		return -1;
	}
	if ((image_name = strdup(basename(tmp))) == NULL)
	{
		free(tmp);
		fprintf(stderr, "securesu: Out of memory\n");
		return -1;
	}
	free(tmp);
	tmp = image_name + strlen(image_name) - 1;
	while (*tmp != '.' && tmp > image_name)
		tmp--;
	if (tmp == image_name)
	{
		fprintf(stderr, "securesd: Invalid image name\n");
		return -1;
	}
	*tmp = '\0';
	snprintf(cmd, BUFSIZ, "%s luksOpen -d %s %s %s",
		CRYPTSETUP, KEY, image, image_name);
	debug_print("%s", cmd);
	if (system(cmd) != 0)
		return -1;
	if (opts)
	{
		snprintf(cmd, BUFSIZ, "%s mount -o %s /dev/mapper/%s %s",
			BUSYBOX, opts, image_name, mountpoint);
	}
	else
	{
		snprintf(cmd, BUFSIZ, "%s mount /dev/mapper/%s %s",
			BUSYBOX, image_name, mountpoint);
	}
	debug_print("%s", cmd);
	if (system(cmd) != 0)
		return -1;
	return 0;
}

static int unmount_image(char *image, const char *mountpoint)
{
	char *image_name;
	char cmd[BUFSIZ];
	image_name = image + strlen(image) - 1;
	while (*image_name != '.' && image_name > image)
		image_name--;
	if (image_name == image)
	{
		fprintf(stderr, "securesd: Invalid image name: %s\n", image);
		return -1;
	}
	*image_name-- = '\0';
	while (image_name > image && *image_name != '/')
		image_name--;
	if (*image_name == '/')
		image_name++;
	snprintf(cmd, BUFSIZ, "%s umount %s", BUSYBOX, mountpoint);
	debug_print("%s", cmd);
	if (system(cmd) == -1)
		return -1;
	snprintf(cmd, BUFSIZ, "%s luksClose %s", CRYPTSETUP, image_name);
	debug_print("%s", cmd);
	if (system(cmd) == -1)
		return -1;
	return 0;
}

/*
 * securesd_status() -- get the status
 */
static int securesd_status()
{
	int fd;
	char *mount_entry;
	if ((fd = open("/proc/mounts", O_RDONLY)) == -1)
	{
		fprintf(stderr, "securesd: Could not open /proc/mounts\n");
		return 1;
	}
	while ((mount_entry = fd_readline(fd)) != NULL)
	{
		if (strstr(mount_entry, "/dev/mapper/"))
		{
			close(fd);
			return 0;
		}
	}
	close(fd);
	return 1;
}

/*
 * mount_volumes() -- mount all volumes on fstab
 */
static int mount_volumes()
{
	int i;
	if (!securesd_status())
	{
		fprintf(stderr, "securesd: Already mounted\n");
		return -1;
	}
	unhide_imagedir();
	if (parse_fstab() == -1)
	{
		fprintf(stderr, "securesd: Invalid fstab\n");
		hide_imagedir();
		return -1;
	}
	for (i = 0; i < fstab_entries; i++)
	{
		if (mount_image(fstab[i].image, fstab[i].mountpoint, fstab[i].opts) == -1)
		{
			hide_imagedir();
			return -1;
		}
	}
	hide_imagedir();
	return 0;
}

/*
 * unmount volumes
 */
static int unmount_volumes()
{
	int i;
	if (securesd_status())
	{
		fprintf(stderr, "securesd: Already unmounted\n");
		return -1;
	}
	unhide_imagedir();
	if (parse_fstab() == -1)
	{
		fprintf(stderr, "securesd: Invalid fstab\n");
		hide_imagedir();
		return -1;
	}
	for (i = fstab_entries - 1; i >= 0; i--)
	{
		if (unmount_image(fstab[i].image, fstab[i].mountpoint) == -1)
		{
			hide_imagedir();
			return -1;
		}
	}
	hide_imagedir();
	return 0;
}

/*
 * main() -- entry point
 */
int main(int argc, char **argv)
{
	int first_arg = 1;

	if (!strcmp(argv[1], "wait"))
	{
		wait_for_mount(atoi(argv[2]));
		first_arg += 2;
	}

	if (realpath(IMAGEDIR, image_dir) == NULL)
	{
		fprintf(stderr, "securesd: Cannot resolve SD card path\n");
		return -1;
	}

	if (!strcmp(argv[first_arg], "mount"))
	{
		return mount_volumes();
	}
	else if (!strcmp(argv[first_arg], "unmount") || !strcmp(argv[first_arg], "umount"))
	{
		return unmount_volumes();
	}
	else if (!strcmp(argv[first_arg], "status"))
	{
		return securesd_status();
	}

	return 0;
}
