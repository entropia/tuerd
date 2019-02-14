#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <git2.h>
#include <jansson.h>

#include "util.h"
#include "git.h"
#include "rfid.h"

#define LOG_SECTION "git"

static int check_lg2(int error, const char *message) {
	const git_error *lg2err;
	const char *lg2msg = "", *lg2spacer = "";

	if (!error)
		return 0;

	if ((lg2err = giterr_last()) != NULL && lg2err->message != NULL) {
		lg2msg = lg2err->message;
		lg2spacer = " - ";
	}

	debug("%s [%d]%s%s", message, error, lg2spacer, lg2msg);

	return -1;
}

static int read_file_git(const char *repo_path, const char *name, void **out, size_t *outlen) {
	int ret, retcode = -1;

	git_repository *repo;
	ret = git_repository_open_bare(&repo, repo_path);
	if(check_lg2(ret, "opening repo"))
		goto out;

	git_object *master;
	ret = git_revparse_single(&master, repo, "master");
	if(check_lg2(ret, "getting master branch"))
		goto out_repo;

	if(git_object_type(master) != GIT_OBJ_COMMIT) {
		debug("master is not a commit");
		goto out_master;
	}

	git_tree *tree;
	ret = git_commit_tree(&tree, (git_commit*)master);
	if(check_lg2(ret, "getting tree from commit"))
		goto out_master;

	const git_tree_entry *entry = git_tree_entry_byname(tree, name);
	if(!entry) {
		debug("entry %s not found", name);
		goto out_tree;
	}

	if(git_tree_entry_type(entry) != GIT_OBJ_BLOB) {
		debug("entry is not a blob");
		goto out_tree;
	}

	git_object *file;
	ret = git_tree_entry_to_object(&file, repo, entry);
	if(check_lg2(ret, "getting file from tree entry"))
		goto out_tree;

	const void *data = git_blob_rawcontent((git_blob*)file);
	*outlen = git_blob_rawsize((git_blob*)file);

	*out = malloc(*outlen);
	memcpy(*out, data, *outlen);

	retcode = 0;

	git_object_free(file);
out_tree:
	git_tree_free(tree);
out_master:
	git_object_free(master);
out_repo:
	git_repository_free(repo);
out:
	return retcode;
}

void git_init(void) {
	/*git_threads_init();
	atexit(git_threads_shutdown);*/
}

static int parse_key(uint8_t key[static 16], const char *data) {
	if(!data || strlen(data) != 32)
		return -1;

	for(int i=0; i < 16; i++) {
		char buf[3];
		buf[0] = (char) tolower(data[2 * i]);
		buf[1] = (char) tolower(data[2 * i + 1]);
		buf[2] = 0;

		sscanf(buf, "%hhx", &key[i]);
	}

	return 0;
}


/*
 * Check whether an UID is permitted and retrieve the associated keys.
 */
enum rfid_key_cb_result get_key_git(const char *uid, struct rfid_key *key_out) {
	int ret = KEY_CB_ERROR;
	const char *repopath = getenv("TUERD_REPO_PATH");
	char buf[128];

	unsigned int i;
	for(i = 0; i < (128 - strlen(".json")) && uid[i] != 0; i++)
		if('0' <= uid[i] && uid[i] <= '9')
			buf[i] = uid[i];
		else {
			char c = (char) (uid[i] & ~0x20);

			if('A' <= c && c <= 'F')
				buf[i] = c;
			else {
				log("invalid char '%c' in supplied UID", uid[i]);
				return KEY_CB_ERROR;
			}
		}

	buf[i] = 0;
	strcat(buf, ".json");

	// get the blob out of git
	void *out;
	size_t outlen;
	if(read_file_git(repopath, buf, &out, &outlen) < 0) {
		debug("couldn't find UID in git");
		return TAG_UNKNOWN;
	}

	// handle json
	json_error_t err;
	json_t *obj = json_loadb(out, outlen, 0, &err);
	if(!obj) {
		debug("parsing json failed: %s", err.text);
		goto out;
	}

	if(!json_is_object(obj)) {
		debug("didn't get an object");
		goto obj_out;
	}

	json_t *active = json_object_get(obj, "active");
	if(!active) {
		debug("'active' not found");
		goto obj_out;
	}

	if(!json_is_true(active)) {
		debug("policy: key is not active");

		ret = TAG_FORBIDDEN;

		goto obj_out;
	}

	json_t *ca0523_door_key = json_object_get(obj, "ca0523_door_key");
	if(!ca0523_door_key) {
		debug("ca0523_door_key not found");
		goto obj_out;
	}

	const char *keystr = json_string_value(ca0523_door_key);
	if(!keystr) {
		debug("ca0523_door_key is not a string");
		goto obj_out;
	}

	if(strlen(keystr) != 32) {
		debug("key has the wrong length");
		goto obj_out;
	}

        if(parse_key (key_out->key, keystr) < 0) {
		debug("Parsing key failed");
		goto obj_out;
	}

	ret = TAG_ALLOWED;
obj_out:
	json_decref(obj);
out:
	free(out);
	return ret;
}
