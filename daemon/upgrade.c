#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "upgrade.h"
#include "util.h"

struct upgrade {
	int (*apply)(mf_interface *intf);
	int level;
};

struct upgrade *upgrades;
int upgrades_cnt;

static int upgrade_filter(const struct dirent *de) {
	const char *p = strrchr(de->d_name, '.');
	if(p && !strcmp(p, ".so"))
		return 1;

	return 0;
}

static int upgrade_sort(const void *a, const void *b) {
	const struct upgrade *ua = a, *ub = b;

	if(ua->level < ub->level)
		return -1;

	if(ua->level > ub->level)
		return 1;

	return 0;
}

int load_upgrades() {
	int ret = -1;

	const char *upgrade_dir = getenv("TUERD_UPGRADE_DIR");

	DIR *dh = opendir(upgrade_dir);
	if(!dh) {
		log_errno("Opening upgrade dir failed");
		goto out;
	}

	struct dirent *de;
	while((de = readdir(dh)) != NULL) {
		if(!upgrade_filter(de))
			continue;

		char upgrade_path[_POSIX_PATH_MAX];
		snprintf(upgrade_path, _POSIX_PATH_MAX, "%s/%s", upgrade_dir, de->d_name);

		void *dlh = dlopen(upgrade_path, RTLD_LAZY);
		if(!dlh) {
			log_errno("Opening upgrade %s failed", de->d_name);
			goto dir_out;
		}

		void *apply = dlsym(dlh, "apply");
		if(!apply) {
			log_errno("Loading symbol 'apply' from %s failed", de->d_name);
			goto dir_out;
		}

		int *level = dlsym(dlh, "level");
		if(!level) {
			log_errno("Loading symbol 'level' from %s failed", de->d_name);
			goto dir_out;
		}

		upgrades_cnt++;
		upgrades = realloc(upgrades, sizeof(struct upgrade) * upgrades_cnt);

		upgrades[upgrades_cnt-1].apply = apply;
		upgrades[upgrades_cnt-1].level = *level;
		debug("Successfully loaded upgrade %s to level %u", de->d_name, *level);
	}

	qsort(upgrades, upgrades_cnt, sizeof(struct upgrade), upgrade_sort);

	ret = 0;

dir_out:
	closedir(dh);
out:
	return ret;
}

static int64_t get_level(mf_interface *intf) {
	mf_err_t ret;
	uint32_t value;

	ret = mf_get_value(intf, NULL, 0, &value);
	if(ret == MF_ERR_FILE_NOT_FOUND) {
		// Pre-patchlevel cards have level 0
		return 0;
	}
	if(ret != MF_OK) {
		 debug("mf_get_version: %s", mf_error_str(ret));
		 return -1;
	}

	return value;
}

uint32_t do_upgrades(mf_interface *intf, mf_session *sess, uint8_t uid[static 7]) {
	int64_t level = get_level(intf);
	if(level < 0) {
		debug("Retrieving level failed");
		return -1;
	}

	int64_t old_level = level;
	for(int i = 0; i < upgrades_cnt; i++) {
		if(upgrades[i].level <= level)
			continue;

		debug("Applying upgrade to level %u", upgrades[i].level);
		if(upgrades[i].apply(intf) < 0) {
			debug("Upgrade failed, aborting");
			break;
		}

		level++;
	}

	if(level == old_level) { // no upgrades have been applied
		return 0;
	}

	mf_err_t ret;
	ret = mf_credit(intf, NULL, 0, level - old_level);
	if(ret != MF_OK) {
		log("Upping level failed: %s. UID %s is now inconsistent.",
				mf_error_str(ret), format_uid(uid));
		return -1;
	}

	ret = mf_commit_transaction(intf);
	if(ret != MF_OK) {
		log("Committing transaction failed: %s. UID %s is now inconsistent.",
				mf_error_str(ret), format_uid(uid));
		return -1;
	}

	return (uint32_t)level;
}
