// ========================================
// File: makaba.c
// Makaba API-related functions
// (Implementation)
// ========================================
// TODO:
//[ ] captcha
//[x] makabaSetup()
//[x] FIX CURL BUFF ZERO-END
// ========================================

#pragma once
#include "makaba.h"

// ========================================
// General info getting
// ========================================

int getBoardsList (const char* resFile, const bool v) {

	return 0;
}

char* getBoardPageJSON (const char* board, const long int page, const bool v) {
	fprintf (stderr, "]] Starting getBoardPage");
	if (v) fprintf (stderr, " (verbose)"); fprintf (stderr, "\n");
	if (v) fprintf (stderr, "] initializing curl handle\n");
	CURL* curl_handle = curl_easy_init();
	CURLcode request_status = 0;
	if (curl_handle) {
		if (v) fprintf (stderr, "] curl handle initialized\n");
		char* page_string = lint2str (page);
		if (v) fprintf (stderr, "page number (string) = %s\n", page_string);
		short URL_length = strlen(BASE_URL)+strlen(board)+1+strlen(page_string)+5;
		if (v) fprintf (stderr, "URL length = %d\n", URL_length);
		// URL format: 2ch.hk/$board/$page.json
		char* URL = (char*) calloc (URL_length, sizeof(char));
		if (URL != NULL) {
			if (v) fprintf (stderr, "memory allocated (URL)\n");
		}
		else {
			fprintf (stderr, "[getBoardPage]! Error allocating memory (URL)\n");
			curl_easy_cleanup (curl_handle);
			return ERR_MEMORY;
		}

		if (v) fprintf (stderr, "] Forming URL\n");
		URL = strcpy (URL, BASE_URL);
		if (v) fprintf (stderr, "URL state 0: %s\n", URL);
		URL = strcat (URL, board);
		if (v) fprintf (stderr, "URL state 1: %s\n", URL);
		URL = strcat (URL, "/");
		if (v) fprintf (stderr, "URL state 2: %s\n", URL);
		URL = strcat (URL, page_string);
		if (v) fprintf (stderr, "URL state 3: %s\n", URL);
		URL = strcat (URL, ".json");
		if (v) fprintf (stderr, "URL state 4: %s\n", URL);
		if (v) fprintf (stderr, "] URL formed\n");
		curl_easy_setopt (curl_handle, CURLOPT_URL, URL);
		if (v) fprintf (stderr, "] option URL set\n");

		if (CURL_BUFF_BODY == NULL) {
			fprintf(stderr, "[getBoardPage]! Error: curl body buffer not allocated\n");
			return ERR_MAKABA_SETUP;
		}
		curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, CURL_BUFF_BODY);
		if (v) fprintf (stderr, "] option WRITEDATA set\n");

		curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, CURL_writeToBuff);
		if (v) fprintf (stderr, "] option WRITEFUNCTION set\n");

		request_status = curl_easy_perform (curl_handle);
		if (v) fprintf (stderr, "] curl request performed\n");
		CURL_BUFF_BODY[CURL_BUFF_POS] = 0;
		CURL_BUFF_POS = 0;
		if (v) fprintf (stderr, "] buffer pos set to 0\n");
		if (request_status == CURLE_OK) {
			if (v) fprintf (stderr, "request status: OK\n");
			printf ("%s\n", CURL_BUFF_BODY);
		}
		else {
			fprintf (stderr, "[getBoardPage]! Error @ curl_easy_perform: %s\n",
				curl_easy_strerror(request_status));
			curl_easy_cleanup (curl_handle);
			free (URL);
			return ERR_CURL_PERFORM;
		}

		curl_easy_cleanup (curl_handle);
		if (v) fprintf (stderr, "] curl cleanup done\n");
		free (URL);
		fprintf (stderr, "]] Exiting getBoardPage\n");
	}
	else {
		fprintf (stderr, "! Error initializing curl handle\n");
		return ERR_CURL_INIT;
	}

	return CURL_BUFF_BODY;
}

char* getBoardCatalogJSON (const char* board, const bool v) {
	fprintf (stderr, "]] Starting getBoardCatalog");
	if (v) fprintf (stderr, " (verbose)"); fprintf (stderr, "\n");
	if (v) fprintf (stderr, "] initializing curl handle\n");
	CURL* curl_handle = curl_easy_init();
	CURLcode request_status = 0;
	if (curl_handle) {
		if (v) fprintf (stderr, "] curl handle initialized\n");
		short URL_length = strlen(BASE_URL)+strlen(board)+1+7+5;
		// URL format: 2ch.hk/$board/catalog.json
		if (v) fprintf (stderr, "URL length = %d\n", URL_length);
		char* URL = (char*) calloc (URL_length, sizeof(char));
		if (URL != NULL) {
			if (v) fprintf (stderr, "memory allocated (URL)\n");
		}
		else {
			fprintf (stderr, "[getBoardCatalog]! Error allocating memory (URL)\n");
			curl_easy_cleanup (curl_handle);
			return ERR_MEMORY;
		}

		if (v) fprintf (stderr, "] Forming URL\n");
		URL = strcpy (URL, BASE_URL);
		if (v) fprintf (stderr, "URL state 0: %s\n", URL);
		URL = strcat (URL, board);
		if (v) fprintf (stderr, "URL state 1: %s\n", URL);
		URL = strcat (URL, "/catalog.json");
		if (v) fprintf (stderr, "URL state 2: %s\n", URL);
		if (v) fprintf (stderr, "] URL formed\n");
		curl_easy_setopt (curl_handle, CURLOPT_URL, URL);
		if (v) fprintf (stderr, "] option URL set\n");

		if (CURL_BUFF_BODY == NULL) {
			fprintf(stderr, "[getBoardCatalog]! Error: curl body buffer not allocated\n");
			return ERR_MAKABA_SETUP;
		}
		curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, CURL_BUFF_BODY);
		if (v) fprintf (stderr, "] option WRITEDATA set\n");

		curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, CURL_writeToBuff);
		if (v) fprintf (stderr, "] option WRITEFUNCTION set\n");

		request_status = curl_easy_perform (curl_handle);
		if (v) fprintf (stderr, "] curl request performed\n");
		if (request_status == CURLE_OK) {
			if (v) fprintf (stderr, "request status: OK\n");
			printf ("%s\n", CURL_BUFF_BODY);
		}
		else {
			fprintf (stderr, "[getBoardCatalog]! Error @ curl_easy_perform: %s\n",
				curl_easy_strerror(request_status));
				curl_easy_cleanup (curl_handle);
			free (URL);
			return ERR_CURL_PERFORM;
		}

		curl_easy_cleanup (curl_handle);
		if (v) fprintf (stderr, "] curl cleanup done\n");
		free (URL);
		fprintf (stderr, "] memory free done\n");
		fprintf (stderr, "]] Exiting getBoardCatalog\n");
	}
	else {
		fprintf (stderr, "[getBoardCatalog]! Error initializing curl handle\n");
		return ERR_CURL_INIT;
	}

	return CURL_BUFF_BODY;
}

char* getThreadJSON (const char* board, const long int threadnum, long int* threadsize, const bool v) {
	fprintf (stderr, "]] Starting getThread");
	if (v) fprintf (stderr, " (verbose)"); fprintf (stderr, "\n");
	if (v) fprintf (stderr, "] initializing curl handle\n");
	CURL* curl_handle = curl_easy_init();
	CURLcode request_status = 0;
	if (curl_handle) {
		if (v) fprintf (stderr, "] curl handle initialized\n");
		char* threadnum_string = lint2str (threadnum);
		if (v) fprintf (stderr, "thread number (string) = %s\n", threadnum_string);
		const short URL_length = strlen(BASE_URL)+strlen(MOBILE_API);
		if (v) fprintf (stderr, "URL length = %d\n", URL_length);
		// API URL: 2ch.hk/makaba/mobile.fcgi
		char* URL = (char*) calloc (URL_length, sizeof(char));
		if (URL != NULL) {
			if (v) fprintf (stderr, "memory allocated (URL)\n");
		}
		else {
			fprintf (stderr, "[getThread]! Error allocating memory (URL)\n");
			curl_easy_cleanup (curl_handle);
			return ERR_MEMORY;
		}

		curl_easy_setopt (curl_handle, CURLOPT_POST, 1);
		if (v) fprintf (stderr, "] option POST set\n");

		if (v) fprintf (stderr, "] Forming URL\n");
		URL = strcpy (URL, BASE_URL);
		if (v) fprintf (stderr, "URL state 0: %s\n", URL);
		URL = strcat (URL, MOBILE_API);
		if (v) fprintf (stderr, "URL state 1: %s\n", URL);
		if (v) fprintf (stderr, "] URL formed\n");
		curl_easy_setopt (curl_handle, CURLOPT_URL, URL);
		if (v) fprintf (stderr, "] option URL set\n");

		const short postfields_length = 15+1+6+strlen(board)+1+7+strlen(threadnum_string)+1+6;
		// POST data format: task=get_thread&board=$board&thread=$threadnum&post=0

		curl_easy_setopt (curl_handle, CURLOPT_POSTFIELDSIZE, postfields_length);
		if (v) fprintf (stderr, "] Option POSTFIELDSIZE set\n");

		const char* postfields = (char*) calloc (postfields_length, sizeof(char));
		if (postfields != NULL) {
			if (v) fprintf (stderr, "memory allocated (POST data)\n");
		}
		else {
			fprintf (stderr, "[getThread]! Error allocating memory (POST data)\n");
			curl_easy_cleanup (curl_handle);
			free (URL);
			return ERR_MEMORY;
		}
		if (v) fprintf (stderr, "] Forming POST data\n");
		postfields = strcpy (postfields, "task=get_thread");
		if (v) fprintf (stderr, "POST data state 0: %s\n", postfields);
		postfields = strcat (postfields, "&board=");
		if (v) fprintf (stderr, "POST data state 1: %s\n", postfields);
		postfields = strcat (postfields, board);
		if (v) fprintf (stderr, "POST data state 2: %s\n", postfields);
		postfields = strcat (postfields, "&thread=");
		if (v) fprintf (stderr, "POST data state 3: %s\n", postfields);
		postfields = strcat (postfields, threadnum_string);
		if (v) fprintf (stderr, "POST data state 4: %s\n", postfields);
		postfields = strcat (postfields, "&post=0");
		if (v) fprintf (stderr, "POST data state 5: %s\n", postfields);
		if (v) fprintf (stderr, "] POST data formed\n");
		curl_easy_setopt (curl_handle, CURLOPT_POSTFIELDS, postfields);
		if (v) fprintf (stderr, "] Option POSTFIELDS set\n");

		if (CURL_BUFF_BODY == NULL) {
			fprintf(stderr, "[getThread]! Error: curl body buffer not allocated\n");
			return ERR_MAKABA_SETUP;
		}
		curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, CURL_BUFF_BODY);
		if (v) fprintf (stderr, "] option WRITEDATA set\n");

		curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, CURL_writeToBuff);
		if (v) fprintf (stderr, "] option WRITEFUNCTION set\n");

		request_status = curl_easy_perform (curl_handle);
		if (v) fprintf (stderr, "] curl request performed\n");
		CURL_BUFF_BODY[CURL_BUFF_POS] = 0;

		*threadsize = (long int)CURL_BUFF_POS; // Кладём по данному указателю размер
		CURL_BUFF_POS = 0;           // буфера с тредом
		if (v) fprintf (stderr, "] buffer pos set to 0\n");
		if (request_status == CURLE_OK) {
			if (v) fprintf (stderr, "request status: OK\n");
		}
		else {
			fprintf (stderr, "[getThread]! Error @ curl_easy_perform: %s\n",
				curl_easy_strerror(request_status));
			curl_easy_cleanup (curl_handle);
			free (URL);
			free (postfields);
			return ERR_CURL_PERFORM;
		}

		curl_easy_cleanup (curl_handle);
		if (v) fprintf (stderr, "] curl cleanup done\n");
		free (URL);
		free (postfields);
		if (v) fprintf (stderr, "] memory free done\n");
		fprintf (stderr, "]] Exiting getThread\n");
	}
	else {
		fprintf (stderr, "[getThread]! Error initializing curl handle\n");
		return ERR_CURL_INIT;
	}

	return CURL_BUFF_BODY;
}

// ========================================
// Captcha
// ========================================

char* getCaptchaSettingsJSON (const char* board) {
	fprintf (stderr, "]] Starting getCaptchaSettings\n");

	CURL* curl_handle = curl_easy_init();
	CURLcode request_status = 0;
	if (curl_handle) {

		const short URL_length = strlen(BASE_URL)+strlen(CAPTCHA_SETTINGS)+strlen(board)+1;
		char* URL = (char*) calloc (URL_length, sizeof(char));
		if (URL == NULL) {
			fprintf (stderr, "[getCaptchaSettings]! Error allocating memory (URL)\n");
			curl_easy_cleanup (curl_handle);
			return ERR_MEMORY;
		}
		URL = strcpy (URL, BASE_URL);
		URL = strcat (URL, CAPTCHA_SETTINGS);
		URL = strcat (URL, board);
		curl_easy_setopt (curl_handle, CURLOPT_URL, URL);

		if (CURL_BUFF_BODY == NULL) {
			fprintf(stderr, "[getCaptchaSettings]! Error: curl body buffer not allocated\n");
			return ERR_MAKABA_SETUP;
		}
		curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, CURL_BUFF_BODY);

		curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, CURL_writeToBuff);

		request_status = curl_easy_perform (curl_handle);
		CURL_BUFF_BODY[CURL_BUFF_POS] = 0;
		CURL_BUFF_POS = 0;
		if (request_status != CURLE_OK) {
			fprintf (stderr, "[getCaptchaSettings]! Error @ curl_easy_perform: %s\n",
				curl_easy_strerror(request_status));
			curl_easy_cleanup (curl_handle);
			free (URL);
			return ERR_CURL_PERFORM;
		}

		curl_easy_cleanup (curl_handle);
		free (URL);
	}
	else {
		fprintf (stderr, "[getCaptchaSettings]! Error initializing curl handle\n");
		return ERR_CURL_INIT;
	}

	fprintf(stderr, "]] Exiting getCaptchaSettings\n");

	return CURL_BUFF_BODY;
}


char *get2chaptchaIdJSON (const char *board, const char *thread) {
	fprintf (stderr, "]] Starting get2chaptchaID");

	CURL* curl_handle = curl_easy_init();
	CURLcode request_status = 0;
	if (curl_handle) {
		const short URL_length = strlen(BASE_URL)+strlen(CAPTCHA_2CHAPTCHA)+2+1;
		char* URL = (char*) calloc (URL_length, sizeof(char));
		if (URL == NULL) {
			fprintf (stderr, "[get2chaptchaID]! Error allocating memory (URL)\n");
			curl_easy_cleanup (curl_handle);
			return ERR_MEMORY;
		}
		URL = strcpy (URL, BASE_URL);
		URL = strcat (URL, CAPTCHA_2CHAPTCHA);
		URL = strcat (URL, "id");
		curl_easy_setopt (curl_handle, CURLOPT_URL, URL);

		const short postfields_length = 6+strlen(board)+8+strlen(thread)+1;
		char* postfields = (char*) calloc (postfields_length, sizeof(char));
		if (postfields == NULL) {
			fprintf (stderr, "[get2chaptchaID]! Error allocating memory (POST data)\n");
			curl_easy_cleanup (curl_handle);
			free (URL);
			return ERR_MEMORY;
		}
		postfields = strcpy (postfields, "board=");
		postfields = strcat (postfields, board);
		if (thread != NULL) {
			postfields = strcat (postfields, "&thread=");
			postfields = strcat (postfields, thread);
		}
		curl_easy_setopt (curl_handle, CURLOPT_POSTFIELDS, postfields);

		if (CURL_BUFF_BODY == NULL) {
			fprintf(stderr, "[get2chaptchaID]! Error: curl body buffer not allocated\n");
			return ERR_MAKABA_SETUP;
		}
		curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, CURL_BUFF_BODY);

		curl_easy_setopt (curl_handle, CURLOPT_WRITEFUNCTION, CURL_writeToBuff);

		request_status = curl_easy_perform (curl_handle);
		CURL_BUFF_BODY[CURL_BUFF_POS] = 0;
		CURL_BUFF_POS = 0;
		if (request_status != CURLE_OK) {
			fprintf (stderr, "[get2chaptchaID]! Error @ curl_easy_perform: %s\n",
				curl_easy_strerror(request_status));
			curl_easy_cleanup (curl_handle);
			free (URL);
			free (postfields);
			return ERR_CURL_PERFORM;
		}

		curl_easy_cleanup (curl_handle);
		free (URL);
		free (postfields);
	}
	else {
		fprintf (stderr, "[get2chaptchaID]! Error initializing curl handle\n");
		return ERR_CURL_INIT;
	}

	fprintf(stderr, "]] Exiting get2chaptchaID\n");

	return CURL_BUFF_BODY;
}

char *get2chaptchaPicURL (const char *id) {
	fprintf (stderr, "]] Starting get2chaptchaPic");

	const long int URL_length = strlen(BASE_URL)+strlen(CAPTCHA_2CHAPTCHA)+6+strlen(id)+1;
	char* URL = (char*) calloc (URL_length, sizeof(char));
	if (URL == NULL) {
			fprintf (stderr, "[get2chaptchaPic]! Error allocating memory (URL)\n");
			return ERR_MEMORY;
	}
	URL = strcpy (URL, BASE_URL);
	URL = strcat (URL, CAPTCHA_2CHAPTCHA);
	URL = strcat (URL, "image/");
	URL = strcat (URL, id);

	fprintf(stderr, "]] Exiting get2chaptchaID\n");

	return URL;
}

// ========================================
// Misc utility functions
// ========================================

void makabaSetup() {
	if (CURL_BUFF_BODY == NULL)
		CURL_BUFF_BODY = (char*) calloc (CURL_BUFF_BODY_SIZE, sizeof(char));
}

void makabaCleanup() {
	if (CURL_BUFF_BODY != NULL)
		free(CURL_BUFF_BODY);
}

size_t CURL_writeToBuff (const char* src, const size_t block_size, const size_t nmemb, void* dest) {
	if (src==NULL || CURL_BUFF_POS+block_size*nmemb > CURL_BUFF_BODY_SIZE) {
		return 0;
	}
	else {
		memcpy (dest+CURL_BUFF_POS, src, block_size*nmemb);
		CURL_BUFF_POS += block_size*nmemb;
		return block_size*nmemb;
	}
}

char* lint2str (const long int val) {
	short length = 0;
	for (int k = 1; k <= val; k*=10) {
		length += 1;
	}
	char* res = (char*) calloc (length+1, sizeof(char));
	if (res != NULL) {
	}
	else {
		fprintf (stderr, "[lint2str]! Error allocating memory (res)\n");
		return NULL;
	}
	for (int i = 1, k = 1; i <= length; i+=1, k*=10) {
		res[length-i] = '0' + ((val/k)%10);
	}
	return res;
}

long int str2lint (const char* str, const long int len) {
	long int res = 0;
	for (long int i = 1, k = 1; i <= len; i+=1, k*=10) {
		res += k * (str[len-i] - '0');
	}
	return res;
}
