#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define IMAGEDIR		"/sdcard/.securesd"
#define FSTAB				"/sdcard/.securesd/fstab"
#define CRYPTSETUP	"/system/xbin/cryptsetup"
#define KEY					"/data/system/securesd.key"

struct fstab_entry
{
	char *image;
	char *mountpoint;
	char *opts;
};

static char image_dir[PATH_MAX];
static struct fstab_entry fstab[32];
static int fstab_entries = 0;

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
 * mount_volumes() -- mounts all volumes on fstab
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
 * mount_volumes() -- mount all volumes on fstab
 */
static int mount_volumes()
{
	int i;
	if (parse_fstab() == -1)
	{
		fprintf(stderr, "securesd: Invalid fstab\n");
		return -1;
	}
	for (i = 0; i < fstab_entries; i++)
	{
		fprintf(stderr, "'%s' '%s' '%s'\n",
			fstab[i].image, fstab[i].mountpoint, fstab[i].opts);
	}
	return 0;
}

/*
 * unmount volumes
 */
static int unmount_volumes()
{
	int i;
	if (parse_fstab() == -1)
	{
		fprintf(stderr, "securesd: Invalid fstab\n");
		return -1;
	}
	for (i = fstab_entries - 1; i >= 0; i--)
	{
		fprintf(stderr, "'%s' '%s' '%s'\n",
			fstab[i].image, fstab[i].mountpoint, fstab[i].opts);
	}
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

	if (!strcmp(argv[first_arg], "mount"))
	{
		return mount_volumes();
	}
	else if (!strcmp(argv[first_arg], "unmount") || !strcmp(argv[first_arg], "umount"))
	{
		return unmount_volumes();
	}

	if (realpath(IMAGEDIR, image_dir) == NULL)
	{
		fprintf(stderr, "securesd: Cannot resolve SD card path\n");
		return -1;
	}
	fprintf(stdout, "securesd: sdcard path: %s\n", image_dir);
	return 0;
}
