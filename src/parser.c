// ========================================
// File: parser.c
// API answer parsing functions
// (Implementation)
// ========================================

#pragma once
#include "parser.h"

unsigned* findPostsInJSON (const char* src, unsigned* postcount_res, const bool v) {
	fprintf (stderr, "]] Starting findPostsInJSON");
	FILE* LOCAL_LOG = NULL;

	if (v) fprintf (stderr, " (verbose, log in ./log/findPostsInJSON)"); fprintf (stderr, "\n");
	if (v) LOCAL_LOG = fopen("log/findPostsInJSON.log", "a");
	if (v) fprintf(LOCAL_LOG, "\n\n<< New Thread >>\n");

	short srclen = strlen (src);
	int* temp = (int*) calloc (sizeof(int),srclen/8);

	short depth = 0;
	unsigned postcount = 0;
	bool comment_read = false;

	for (int i = 1; i < srclen; i+=1) {
		if (v) fprintf(LOCAL_LOG, "%c", src[i]);
		switch (depth) {
			case 2:
				if (comment_read) { // in: .post.files
					if (src[i] == ']') {
						if (v) fprintf (LOCAL_LOG, "Exiting files. ");
						depth -= 1;
					}
				}
				else { // in: .post.comment
					if ((src[i-1] != '\\') && (src[i] == '"')) {
						if (v) fprintf (LOCAL_LOG, "Exiting comment. ");
						comment_read = true;
						depth -= 1;
					}
				}
				continue;
			case 1: // in: .post
				if (src[i] == '}') { //@TODO reverse order of ifs in 'case 1'
					if (v) fprintf (LOCAL_LOG, "Exiting post (@%d).\n", i);
					postcount += 1;
					depth -= 1;
					comment_read = false;
				}
				if ((src[i-2]=='"')&&(src[i-1]=='f')&&(src[i]=='i')&&
					(src[i+1]=='l')&&(src[i+2]=='e')) {
					if (v) fprintf (LOCAL_LOG, "Entering files. ");
					depth += 1;
				}
				if ((src[i-2]=='"')&&(src[i-1]=='c')&&(src[i]=='o')&&
					(src[i+1]=='m')&&(src[i+2]=='m')) {
					if (v) fprintf (LOCAL_LOG, "Entering comment. ");
					depth += 1;
				}
				continue;
			case 0: // in: .
				if (src[i] == '{') {
					if (v) fprintf (LOCAL_LOG, "Entering post #%d ", postcount+1);
					temp[postcount] = i;
					if (v) fprintf (LOCAL_LOG, "(@%d)\n", temp[postcount]);
					depth += 1;
				}
				continue;
			default:
				fprintf (stderr, "! Error: Unusual depth (%d)\n", depth);
				free (temp);
				return ERR_PARTTHREAD_DEPTH;
		}
	}
	if (depth != 0) {
		fprintf (stderr, "! Error: Non-zero depth (%d) after cycle\n", depth);
		return ERR_PARTTHREAD_DEPTH;
	}

	if (v) fprintf (LOCAL_LOG, "%d posts found\n", postcount);

	unsigned* posts = (int*) calloc (sizeof(int), postcount);
	posts = memcpy (posts, temp, postcount*sizeof(int));
	free (temp);
	if (v) fprintf (LOCAL_LOG, "Posts begin at:\n");
	if (v) for (int i = 0; i < postcount; i++)
		fprintf (LOCAL_LOG, "] %2d: %5d\n", i, posts[i]);

	*postcount_res = postcount;

	if (v) fprintf(LOCAL_LOG, "<< End of Thread >>\n");
	if (v) fclose(LOCAL_LOG);
	fprintf(stderr, "]] Exiting findPostsInJSON\n");
	return posts;
}

struct post* initPost (const char* post_string, const unsigned postlen, const bool v) {
	fprintf(stderr, "]] Starting initPost");
	FILE* LOCAL_LOG = NULL;
	if (v) LOCAL_LOG = fopen("log/initPost.log", "a");
	if (v) fprintf(stderr, " (verbose, log in log/initPost.log)"); fprintf (stderr, "\n");
	if (v) fprintf(LOCAL_LOG, "\n\n<< New Thread >>\n]] Args:\n== | post_string = ");
	if (v) for (int i = 0; i < 100; i++)
		fprintf(LOCAL_LOG, "%c", post_string[i]);
	if (v) fprintf(LOCAL_LOG, "\n| postlen = %d\n", postlen);

	// Detecting comment:
	char* ptr_comment = strstr (post_string, PATTERN_COMMENT) + strlen (PATTERN_COMMENT);
	if (ptr_comment == NULL) {
		fprintf (stderr, "! Error: Bad post format: Comment pattern not found\n");
		return ERR_POST_FORMAT;
	}

	short comment_len = 0; bool stop = false;
	for (int i = ptr_comment-post_string; !stop && (i < postlen); i++) {
		if ((post_string[i-1] != '\\') && (post_string[i] == '\"') && (post_string[i+1] != 'u')) {
			stop = true;
		}
		comment_len++;
	}
	if (!stop) {
		if (v) fprintf(LOCAL_LOG, "\n=== ! Range violation! ===\n");
		fprintf(stderr, "! Error: Comment starts but not ends in post-length range\n");
		return ERR_COMMENT_FORMAT;
	}
	comment_len--;
	if (comment_len <= 0) {
		fprintf (stderr, "! Error: Bad post format: Null comment\n");
		return ERR_POST_FORMAT;
	}
	if (v) fprintf (LOCAL_LOG, "] Comment length: %d\n", comment_len);
	if (v) fprintf (LOCAL_LOG, "] Comment: \n== Start comment ==\n");
	if (v) for (int i = 0; i < comment_len; i++)
		fprintf (LOCAL_LOG, "%c", ptr_comment[i]);
	if (v) fprintf (LOCAL_LOG, "\n== End of comment ==\n");

	// Detect date:
	char* ptr_date = strstr (ptr_comment+comment_len, PATTERN_DATE)+strlen(PATTERN_DATE);
	if (ptr_date == NULL) {
		fprintf (stderr, "! Error: Bad post format: Date pattern not found\n");
		return ERR_POST_FORMAT;
	}
	short date_len = strstr (ptr_date, "\"")-ptr_date;
	if (v) {
		fprintf (LOCAL_LOG, "] Date length: %d\n", date_len);
		fprintf (LOCAL_LOG, "] Date: ");
		for (int i = 0; i < date_len; i++)
			fprintf (LOCAL_LOG, "%c", ptr_date[i]);
		fprintf (LOCAL_LOG, "\n");

	}

	// Detect email:
	char* ptr_email = strstr (ptr_date+date_len, PATTERN_EMAIL)+strlen(PATTERN_EMAIL);
	if (ptr_email == NULL) {
		fprintf (stderr, "! Error: Bad post format: Email pattern not found\n");
		return ERR_POST_FORMAT;
	}
	short email_len = strstr (ptr_email, "\"")-ptr_email;
	if (v) {
		if (email_len == 0) {
			fprintf (LOCAL_LOG, "] Email not specified\n");
		} else {
			fprintf (LOCAL_LOG, "] Email length: %d\n", email_len);
			fprintf (LOCAL_LOG, "] Email: ");
			for (int i = 0; i < email_len; i++)
				fprintf (LOCAL_LOG, "%c", ptr_email[i]);
			fprintf (LOCAL_LOG, "\n");
		}
	}

	// Detect files:
	char* ptr_files = strstr (ptr_email+email_len, PATTERN_FILES)+strlen(PATTERN_FILES);
	short files_len = 0;
	// If NULL, simply no files in post
	if (ptr_files-strlen(PATTERN_FILES) == NULL) {
		if (v) fprintf (LOCAL_LOG, "] Files not specified\n");
	} else {
		files_len = strstr (ptr_files, "}]")-ptr_files;
		if (files_len == 0) {
			fprintf (stderr, "! Error: Bad post format: Files field specified but null\n");
			return ERR_POST_FORMAT;
		} else
			if (v) {
				fprintf(LOCAL_LOG, "] Files length: %d\n", files_len);
				fprintf (LOCAL_LOG, "] Files: \n");
				for (int i = 0; i < files_len; i++) {
					fprintf (LOCAL_LOG, "%c", ptr_files[i]);
				}
				fprintf (LOCAL_LOG, "\n] End of files\n");
			}
	}

	// Detect name:
	char* ptr_name = 0; char* ptr_name_diff = 0; short name_diff_len = 0;
	if (ptr_files-strlen(PATTERN_FILES) == NULL) {
		ptr_name_diff = ptr_email; // Files field may include "\"name\":" substring,
		name_diff_len = email_len; // so we must ignore the files field, if it exists.
	} else {                     // However, if it is not specified, we cannot use
		ptr_name_diff = ptr_files; // ptr_files. So we split into 2 cases and use
		name_diff_len = files_len; // 2 variants of values for 2 new pointers.
	}
	ptr_name = strstr (ptr_name_diff+name_diff_len, PATTERN_NAME)+strlen(PATTERN_NAME);
	if (ptr_name == NULL) {
		fprintf (stderr, "! Error: Bad post format: Name pattern not found\n");
		return ERR_POST_FORMAT;
	}
	short name_len = strstr (ptr_name, "\"")-ptr_name;
	if (v) fprintf (LOCAL_LOG, "] Name length: %d\n", name_len);
	if (v) fprintf (LOCAL_LOG, "] Name: ");
	if (v) for (int i = 0; i < name_len; i++)
		fprintf (LOCAL_LOG, "%c", ptr_name[i]);
	if (v) fprintf (LOCAL_LOG, "\n");

	// Detect postnum
	char* ptr_num = strstr (ptr_name+name_len,PATTERN_NUM) + strlen(PATTERN_NUM);
	if (ptr_num == NULL) {
		fprintf (stderr, "! Error: Bad post format: Num pattern not found\n");
		return ERR_POST_FORMAT;
	}
	short num_len = strstr (ptr_num, ",")-ptr_num-1; // Exclude ending '"'
	if (v) {
		fprintf (LOCAL_LOG, "] Num length: %d\n", num_len);
		fprintf (LOCAL_LOG, "] Num: |");
		for (int i = 0; i < num_len; i++)
			fprintf (LOCAL_LOG, "%c", ptr_num[i]);
		fprintf (LOCAL_LOG, "|\n");
	}

	if (v) fprintf (LOCAL_LOG, "] = All main fields detected\n");

	// Init struct:
	struct post* post = (struct post*) calloc (1,sizeof(struct post));

	post->num = str2unsigned(ptr_num, num_len);

	char* comment_str = (char*) calloc (comment_len, sizeof(char));
	memcpy(comment_str, ptr_comment, comment_len);

	post->comment = parseComment (comment_str,true);
	if (post->comment == NULL) {
		fprintf (stderr, "! Error parsing comment\n");
		return ERR_COMMENT_PARSING;
	}
	free (comment_str);
	if (v) {
		fprintf (LOCAL_LOG, "!! post.comment.nrefs = %d\n", post->comment->nrefs);
		fprintf (LOCAL_LOG, "!! post.comment.refs[0].link = %s\n", post->comment->refs[0].link);
		fprintf (LOCAL_LOG, "!! post.comment.text = %s\n", post->comment->text);
	}

	post->date = (char*) calloc (sizeof(char), date_len+1);
	if (post->date == NULL) {
		fprintf (stderr, "! Error allocating memory (post.date)\n");
		return ERR_MEMORY;
	}
	memcpy (post->date, ptr_date, sizeof(char)*date_len);

	post->name = (char*) calloc (sizeof(char), name_len+1);
	if (post->name == NULL) {
		fprintf (stderr, "! Error allocating memory (post.name)\n");
		return ERR_MEMORY;
	}
	memcpy (post->name, ptr_name, sizeof(char)*name_len);
	if (email_len == 0) {
		post->email = NULL;
	} else {
		post->email = (char*) calloc (sizeof(char), email_len+1);
		if (post->email == NULL) {
		fprintf (stderr, "! Error allocating memory (post.email)\n");
		return ERR_MEMORY;
	}
		memcpy (post->email, ptr_email, sizeof(char)*email_len);
	}
	if (files_len == 0) {
		post->files = NULL;
	} else {
		post->files = (char*) calloc (sizeof(char), files_len+1);
		if (post->files == NULL) {
		fprintf (stderr, "! Error allocating memory (post.files)\n");
		return ERR_MEMORY;
	}
		memcpy (post->files, ptr_files, sizeof(char)*files_len);
	}
	
	if (v) {
		fprintf(LOCAL_LOG, "] = Init struct done\n");
		fprintf(LOCAL_LOG, "<< End of Thread >>\n");
		fclose(LOCAL_LOG);
	}
	fprintf (stderr, "]] Exiting initPost\n");
	return post;
}

struct comment* parseComment (char* comment, const bool v) {
	fprintf (stderr, "]] Started parseComment");
	FILE* LOCAL_LOG = NULL;
	if (v) LOCAL_LOG = fopen("log/parseComment.log", "a");
	if (v) fprintf (stderr, " (verbose, log in log/parseComment.log)"); fprintf(LOCAL_LOG, "\n");
	fprintf (stderr, "\n");

	if (v) fprintf(LOCAL_LOG, "\n\n << New Thread >>\n");
	if (v) fprintf(LOCAL_LOG, "]] Args:\n]]| %s\n", comment);
	
	unsigned comment_len = strlen (comment);
	int nrefs = 0;
	size_t pos = 0;
	struct list* refs = calloc (1, sizeof(struct list));
	refs->first = refs;
	refs->data = 0;
	refs->next = 0;
	struct ref_reply* cref = 0;
	unsigned clean_len = 0;
	char* clean_comment = (char*) calloc (comment_len, sizeof(char));
	char* previnst = NULL;
	
	char* cinst = strstr(comment,PATTERN_HREF_OPEN);
	if ((cinst != NULL) && (cinst != comment)) {
		fprintf(LOCAL_LOG, "]] Starts with text: clean_len = %d\n",
			comment, cinst, cinst-comment);
		memcpy(clean_comment,comment,cinst-comment);
		fprintf(LOCAL_LOG, "]]]] preCopied\n");
		/*
		for (int i = 0; i < cinst; i++) {
			fprintf(LOCAL_LOG, "%c", comment[i]);
		fprintf(LOCAL_LOG, "\n");
		*/
		clean_len += cinst-comment;
	} else {
		fprintf(LOCAL_LOG, "[!!] Doesnt start with text: comment = %p, cinst = %p, clen = %d\n",
			comment, cinst, cinst-comment);
	}
	fprintf(LOCAL_LOG, "clean_len = %d\n", clean_len);

	for ( ; cinst != NULL; cinst = strstr(pos+comment,PATTERN_HREF_OPEN)) {
		if (v) fprintf (LOCAL_LOG, "]]] New ref\n]]]] Opened @ [%d]\n", cinst-comment);
		pos = cinst-comment;
		char* cinst_end = strstr (cinst, PATTERN_HREF_CLOSE);
		if (cinst_end == NULL) {
			fprintf (stderr, "! Error: ref opened but not closed\n");
			fprintf (LOCAL_LOG, "-- Comment: \"%s\"\n", comment);
			return ERR_COMMENT_FORMAT;
		}
			if (v) fprintf (LOCAL_LOG, "]]]] Closed @ [%d]\n", cinst_end-comment);
			char* class = strstr(cinst, PATTERN_REPLY_CLASS);
			if (class != NULL) {
				if (v) fprintf (LOCAL_LOG, "]]]] Type: reply\n");
				nrefs++;
				cref = parseRef_Reply (cinst, cinst_end-cinst, false);
				memcpy (clean_comment+clean_len, ">>", sizeof(char)*2);
				clean_len += 2;
				short csize = strlen (unsigned2str(cref->num));
				//fprintf(LOCAL_LOG, "Num as string: |%s|\n", unsigned2str(cref->num));
				memcpy (clean_comment+clean_len, unsigned2str(cref->num), sizeof(char)*csize);
				if (v) fprintf(LOCAL_LOG, "]]]] replyCopied\n");
				/*
				if (v) for (int j = 0; j < csize; j++) {
					fprintf(LOCAL_LOG, "%c", clean_comment[clean_len+j]);
				}
				if (v) fprintf(LOCAL_LOG, "\n");
				*/
				clean_len += csize;
				if (v) fprintf (LOCAL_LOG, "]]]] Init done:\n");
				if (v) fprintf (LOCAL_LOG, "]]]]] link=\"%s\"\n]]]]] thread=%d\n]]]]] num=%d\n",
												cref->link, cref->thread, cref->num);
				if (v) fprintf(LOCAL_LOG, "Adding new ref #%d\n", nrefs);
				if (nrefs > 1) {
					refs->next = (struct list*) calloc (1, sizeof(struct list));
					refs->next->first = refs->first;
					refs->next->next = 0;
					refs->next->data = calloc (1, sizeof(struct ref_reply));
					memcpy (refs->next->data, (void*) cref, sizeof(struct ref_reply));
					refs = refs->next;
					//if (v) fprintf(LOCAL_LOG, "List member state: 1st = %p, cur = %p, next = %p\n", refs->first, refs, refs->next);
				} else {
					refs->next = 0;
					refs->data = calloc (1, sizeof(struct ref_reply));
					memcpy (refs->data, (void*) cref, sizeof(struct ref_reply));
					//if (v) fprintf(LOCAL_LOG, "Filling 1st list member\n");
 					//if (v) fprintf(LOCAL_LOG, "List member state: 1st = %p, cur = %p, next = %p\n", refs->first, refs, refs->next);
				}
			} else {
				fprintf (LOCAL_LOG, "]]]] Unknown ref type, treat as text\n");
				short csize = 0;
				if (previnst != NULL) {
					csize = cinst-previnst;
				} else {
					csize = cinst-comment;
				}
				if (v) fprintf(LOCAL_LOG, "[!] From #%d: %s\n", pos, comment+pos);
				if (v) fprintf(LOCAL_LOG, "[!] To: %p+%d\n", clean_comment, clean_len);
				if (v) fprintf(LOCAL_LOG, "[!] Size: %d\n", csize);
				if (csize > 0) {
					memcpy (clean_comment+clean_len, comment+pos, sizeof(char)*csize);
					if (v) fprintf(LOCAL_LOG, "]]]] otherCopied\n");
					/*
					if (v) for (int j = 0; j < csize; j++) {
						fprintf(LOCAL_LOG, "%c", clean_comment[clean_len+j]);
					}
					if (v) fprintf(LOCAL_LOG, "\n");
					*/
					clean_len += csize;
				}
			}


			pos = cinst_end-comment+strlen(PATTERN_HREF_CLOSE);
			free (cref);
			previnst = cinst;
	}
	short csize = comment_len-pos;
	if (v) fprintf(LOCAL_LOG, "[!] From #%d: %s\n", pos, comment+pos);
	if (v) fprintf(LOCAL_LOG, "[!] To: %p+%d\n", clean_comment, clean_len);
	if (v) fprintf(LOCAL_LOG, "[!] Size: %d\n", csize);
	if (csize > 0) {
		memcpy (clean_comment+clean_len, comment+pos, sizeof(char)*csize);
		if (v) fprintf(LOCAL_LOG, "]] afterCopied\n");
		/*
		if (v) for (int j = 0; j < csize; j++) {
			fprintf(LOCAL_LOG, "%c", clean_comment[clean_len+j]);
		}
		if (v) fprintf(LOCAL_LOG, "\n");
		*/
		clean_len += csize;
	}
	char* finally_clean_comment = cleanupComment (clean_comment,clean_len,false);
	if (v) fprintf(LOCAL_LOG, "]]] Total: %d refs\n", nrefs);
	struct comment* parsed = (struct comment*) calloc (1, sizeof(struct comment));
	parsed->text = (char*) calloc (comment_len, sizeof(char));
	memcpy (parsed->text, finally_clean_comment, sizeof(char)*strlen(finally_clean_comment));
	free (clean_comment);
	if (v) fprintf(LOCAL_LOG, "Final text: %s\n", parsed->text);
	parsed->nrefs = nrefs;
	if (v) fprintf(LOCAL_LOG, "Final number of refs: %d\n", parsed->nrefs);
	parsed->refs = (struct ref_reply*) calloc (nrefs, sizeof(struct ref_reply));
	refs = refs->first;
	for (int i = 0; i < nrefs; i++) {
		//if (v) fprintf(LOCAL_LOG, "Copy ref #%d, first = %p, current = %p, next = %p\n", i+1, refs->first, refs, refs->next);
		memcpy(parsed->refs+i*sizeof(struct ref_reply), (void*) refs->data, sizeof(struct ref_reply));
	}

	if (v) fprintf(LOCAL_LOG, "<< End of Thread >>\n");
	if (v) fclose(LOCAL_LOG);
	fprintf(stderr, "]] Exiting parseComment\n");
	return parsed;
}

char* cleanupComment (const char* src, const unsigned src_len, const bool v) {
	fprintf(stderr, "]] Started cleanupComment");
	FILE* LOCAL_LOG = NULL;
	if (v) LOCAL_LOG = fopen("log/cleanupComment.log", "a");
	if (v) fprintf(stderr, " (verbose, log in log/cleanupComment.log");
	fprintf(stderr, "\n");
	if (v) fprintf(LOCAL_LOG, "\n\n=== New Thread ===\nArgs:\n| src = %s\n| src_len = %d\n", src, src_len);

	char* buf = (char*) calloc (src_len, sizeof(char));
	unsigned pos = 0, len = 0;
	for ( ; pos < src_len; ) {
		if ((src[pos] == '\\')   && (src[pos+1] == 'u') && (src[pos+2] == '0') && (src[pos+3] == '0')
			&& (src[pos+4] == '3') && (src[pos+5] == 'c') && (src[pos+6] == 'b') && (src[pos+7] == 'r')) {
			pos += 14;
			buf[len] = '\n';
			len++;
		} else if ((src[pos   ] == '\\') && (src[pos+1 ] == 'u' ) && (src[pos+2 ] == '0' ) && (src[pos+3 ] == '0')
				    && (src[pos+4 ] == '3' ) && (src[pos+5 ] == 'c' ) && (src[pos+6 ] == 's' ) && (src[pos+7 ] == 'p')
					  && (src[pos+8 ] == 'a' ) && (src[pos+9 ] == 'n' ) && (src[pos+10] == ' ' ) && (src[pos+11] == 'c')
					  && (src[pos+12] == 'l' ) && (src[pos+13] == 'a' ) && (src[pos+14] == 's' ) && (src[pos+15] == 's')
					  && (src[pos+16] == '=' ) && (src[pos+17] == '\\') && (src[pos+18] == '"' ) && (src[pos+19] == 'u')
					  && (src[pos+20] == 'n' ) && (src[pos+21] == 'k' ) && (src[pos+22] == 'f' ) && (src[pos+23] == 'u')
					  && (src[pos+24] == 'n' ) && (src[pos+25] == 'c' ) && (src[pos+26] == '\\') && (src[pos+27] == '"')) {
				pos += 36; // TEST!
				// Color: green
		} else if ((src[pos   ] == '\\') && (src[pos+1 ] == 'u' ) && (src[pos+2 ] == '0' ) && (src[pos+3 ] == '0')
				    && (src[pos+4 ] == '3' ) && (src[pos+5 ] == 'c' ) && (src[pos+6 ] == '/' ) && (src[pos+7 ] == 's')
					  && (src[pos+8 ] == 'p' ) && (src[pos+9 ] == 'a' ) && (src[pos+10] == 'n' )) {
				pos += 17;
		} else if ((src[pos   ] == '\\') && (src[pos+1 ] == '\\' ) && (src[pos+2 ] == 'r') && (src[pos+3 ] == '\\')
						&& (src[pos+4 ] == '\\') && (src[pos+5 ] == 'n')) {
			pos += 6;
			buf[len] = '\n';
			len++;
		} else {
			buf[len] = src[pos];
			len++;
			pos++;
		}
	}

	char* clean = (char*) calloc (len, sizeof(char));
	memcpy(clean, buf, sizeof(char)*len);
	free (buf);
	if (v) fprintf(LOCAL_LOG, "=== End of Thread ===\n");
	if (v) fclose(LOCAL_LOG);
	fprintf(stderr, "]] Exiting cleanupComment\n");
	return clean;
}

struct ref_reply* parseRef_Reply (const char* ch_ref, const unsigned ref_len, const bool v) {
	fprintf (stderr, "-]] Started parseRef_Reply");
	FILE* LOCAL_LOG = NULL;
	if (v) {
		LOCAL_LOG = fopen("log/parseRef_Reply.log", "a");
		fprintf(stderr, " (verbose, log in log/parseRef_Reply.log)"); fprintf (stderr, "\n");
		fprintf(LOCAL_LOG, "\n\n<< New Thread >>\n]] Args:\n== | ch_ref = ");
		for (int i = 0; i < 100; i++)
			fprintf(LOCAL_LOG, "%c", ch_ref[i]);
		fprintf(LOCAL_LOG, "\n| ref_len = %d\n", ref_len);
	}

	char* link_start = ch_ref + strlen (PATTERN_HREF_OPEN);
	short link_len = strstr (ch_ref, PATTERN_REPLY_CLASS) - 3 - link_start;
													// '-3' to exclude JSON 

	char* data_thread_start = strstr (ch_ref, PATTERN_REPLY_THREAD) + strlen(PATTERN_REPLY_THREAD);
	if (data_thread_start == NULL) { // '+ strlen' to exclude opening string
		fprintf (stderr, "! Error: no data-thread in reply-ref\n");
		return ERR_REF_FORMAT;
	}
	char* data_thread_end = strstr (data_thread_start, "\\\"") - 1;
	if (data_thread_end == NULL-1) {  // '- 1' to exclude '\'
		fprintf (stderr, "! Error: data-thread opened but not closed\n");
		return ERR_REF_FORMAT;
	}
	unsigned data_thread = str2unsigned (data_thread_start, data_thread_end-data_thread_start+1);
	if (v) fprintf (LOCAL_LOG, "-]]] Thread: %d\n", data_thread);

	char* data_num_start = strstr (ch_ref, PATTERN_REPLY_NUM) + strlen(PATTERN_REPLY_NUM);
	if (data_num_start == NULL) { // '+ strlen' to exclude opening string
		fprintf (stderr, "! Error: no data-num in reply-ref\n");
		return ERR_REF_FORMAT;
	}
	char* data_num_end = strstr (data_num_start, "\\\"") - 1;
	if (data_thread_end == NULL-1) {  // '- 1' to exclude '\'
		fprintf (stderr, "! Error: data-num opened but not closed\n");
		return ERR_REF_FORMAT;
	}
	unsigned data_num = str2unsigned (data_num_start, data_num_end-data_num_start+1);
	if (v) fprintf (LOCAL_LOG, "-]]] Postnum: %d\n", data_num);

	struct ref_reply* ref_parsed = (struct ref_reply*) calloc (1, sizeof(struct ref_reply));
	ref_parsed->link = (char*) calloc (link_len, sizeof(char) + 1);
	memcpy (ref_parsed->link, link_start, link_len*sizeof(char));
	ref_parsed->thread = data_thread;
	ref_parsed->num = data_num;

	if (v) {
		fprintf(LOCAL_LOG, "<< End of Thread >>\n");
		fclose(LOCAL_LOG);
	}
	fprintf (stderr, "-]] Exiting parseRef_Reply\n");

	return ref_parsed;
}

struct thread* initThread (const char* thread_string, const unsigned thread_len, const bool v) {
	fprintf(stderr, "]] Started initThread ");
	FILE* LOCAL_LOG = NULL;
	if (v) {
		LOCAL_LOG = fopen("log/initThread.log", "a");
		fprintf(stderr, " (verbose, log in log/initThread.log)\n");
		fprintf(LOCAL_LOG, "\n\n== New Thread ==\n]] Args:\n| thread_string = %p\n| thread_len = %d\n",
			thread_string, thread_len);
	}

	unsigned nposts = 0;
	unsigned* post_diffs = findPostsInJSON(thread_string,&nposts,false);
	if (v) fprintf(LOCAL_LOG, "] nposts = %d\n", nposts);

	struct post** posts = (struct post**) calloc(nposts,sizeof(struct post*));
	for (int i = 0; i < nposts-1; i++) {
		if (v) fprintf(LOCAL_LOG, "] Calling initPost #%d\n", i);
		posts[i] = initPost( 
							thread_string + post_diffs[i],
							post_diffs[i+1] - post_diffs[i],
							false
							);
	}
	posts[nposts-1] = initPost( 
							   thread_string + post_diffs[nposts-1],
							   strlen(thread_string) - post_diffs[nposts-1],
							   false
							   );

	struct thread* thread = (struct thread*) calloc (1, sizeof(struct thread));
	thread->num = posts[0]->num;
	thread->nposts = nposts;
	thread->posts = posts;

	if (v) {
		fprintf(LOCAL_LOG, "] Struct init done:\n| num = %d\n| nposts = %d\n| posts = %p\n",
			thread->num, thread->nposts, thread->posts);
		fprintf(LOCAL_LOG, "== End of Thread ==\n");
		fclose(LOCAL_LOG);
	}
	fprintf(stderr, "]] Exiting initThread\n");

	return thread;
}

// ========================================
// Freeing functions
// ========================================

void freeRefReply (struct ref_reply* ref) {
	free (ref->link);
}

void freeComment (struct comment* arg) {
	free (arg->text);
	free (arg->refs);
}

void freePost (struct post* post) {
	freeComment (post->comment);
	free (post->date);
	free (post->name);
	if (post->email != -1)
		free (post->email);
	if (post->files != -1)
		free (post->files);
	free (post);
}

void freeThread (struct thread* thread) {
	for (int i = 1; i < thread->nposts; i++) {
		freePost (thread->posts[i]);
	}
	free(thread->posts);
	free(thread);
}