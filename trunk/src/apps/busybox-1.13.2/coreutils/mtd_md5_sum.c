/* vi: set sw=4 ts=4: */
/*
 *  Copyright (C) 2010 Ivan Martinazzoli
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */

#include "libbb.h"


/* This might be useful elsewhere */
static unsigned char *hash_bin_to_hex(unsigned char *hash_value,
				unsigned hash_length)
{
	/* xzalloc zero-terminates */
	char *hex_value = xzalloc((hash_length * 2) + 1);
	bin2hex(hex_value, (char*)hash_value, hash_length);
	return (unsigned char *)hex_value;
}

static uint8_t *mtd_md5_sum(const char *filename, unsigned int length)
{
	int src_fd, count;
	union _ctx_ {
		sha1_ctx_t sha1;
		md5_ctx_t md5;
	} context;
	uint8_t *hash_value = NULL;
	unsigned int total_readed=0;
	RESERVE_CONFIG_UBUFFER(in_buf, 4096);

	src_fd = open_or_warn_stdin(filename);
	if (src_fd < 0) {
		RELEASE_CONFIG_BUFFER(in_buf);
		return NULL;
	}

	md5_begin(&context.md5);

	if (!ENABLE_MD5SUM) {
		RELEASE_CONFIG_BUFFER(in_buf);
		bb_error_msg_and_die("algorithm not supported");
	}

	while (0 < (count = safe_read(src_fd, in_buf, 4096))) {
		total_readed += count;
		if ( total_readed > length )
			count -= (total_readed-length);

		md5_hash((const void*)in_buf, (size_t)count, (void*)&context);

		if ( total_readed > length ) break;
	}

	if ( (count == 0) || ( total_readed > length ) ) {
		md5_end((void*)in_buf, (void*)&context);
		hash_value = hash_bin_to_hex(in_buf, 16);
	}

	RELEASE_CONFIG_BUFFER(in_buf);

	if (src_fd != STDIN_FILENO) {
		close(src_fd);
	}

	return hash_value;
}


int mtd_md5_sum_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int mtd_md5_sum_main(int argc UNUSED_PARAM, char **argv)
{
	static const char keywords[] ALIGN1 = "if\0""length\0";

	enum {
		OP_if=0,
		OP_length,
	};

	int n;
	int return_value = EXIT_SUCCESS;
	uint8_t *hash_value;
	unsigned int length=0xFFFFFFFF;
	const char *infile=0;

	if (!argv[1]) {
		bb_show_usage();
	}

	for (n = 1; argv[n]; n++) {
		int what;
		char *val;
		char *arg = argv[n];

		val = strchr(arg, '=');
		if (val == NULL)
			bb_show_usage();

		*val = '\0';

		what = index_in_strings(keywords, arg);
		if (what < 0)
			bb_show_usage();

		/* *val = '='; - to preserve ps listing? */
		val++;

		if (what == OP_if) {
			infile = val;
			/*continue;*/
		}

		if (what == OP_length) {
			length = xatou(val);
			/*continue;*/
		}	
	} /* end of "for (argv[n])" */	

	if ( !infile )
		bb_show_usage();

	hash_value = mtd_md5_sum(infile, length);
	if (hash_value == NULL) {
		return_value = EXIT_FAILURE;
	} else {
		printf("%s\n", hash_value);
		free(hash_value);
	}

	return return_value;
}

