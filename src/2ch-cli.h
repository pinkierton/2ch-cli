// ========================================
// File: 2ch-cli.h
// A CLI-client for 2ch.hk imageboard written on C
// (Headers)
// ========================================

#pragma once

#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "makaba.c"
#include "parser.c"

#define VERSION "v0.3test2"

const int ERR_ARGS = -1;

const int RET_OK = 0,
		  RET_NOARGS = 1,
		  RET_PREEXIT = 2;

const size_t Thread_size = 1000000;

int printPost (struct post* post,	const bool show_email, const bool show_files);


