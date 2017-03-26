// ========================================
// File: 2ch-cli.c
// A CLI-client for 2ch.hk imageboard written on C
// (Implementation)
// ========================================

#include "2ch-cli.h"

void pomogite() // Справка
{
	printf("2ch-cli "VERSION" - консольный клиент двача\n");
	printf("Использование:\n");
	printf(" -h - помощь\n");
	printf(" -s - запуск\n");
	printf(" -p - задать пасскод\n");
	printf(" -b - задать борду\n");
	printf(" -n - заранее задать номер треда\n");
}

int main (int argc, char **argv)
{

	char passcode[32] = "пасскода нет нихуя :("; // Пасскод
    char board_name[10] = "b"; // Имя борды
    long long post_number = 0; // Номер треда в борде

	//getopt
	int opt;
	bool start_or_not = false; // Проверяем, есть ли '-s' и нужно ли запускать

	if (argc == 1) /* Если аргументов нет, вывод помощи */ {
		pomogite();
		return RET_NOARGS;
	}
	while (( opt = getopt(argc, argv, "hp:b:n:s") ) != -1)
	{
		switch (opt)
		{
			case 'p':
				memset(passcode, '\0', sizeof(passcode));
                if ( sizeof(optarg) > sizeof(passcode) ) { //проверка
					printf("Не шути так больше\n");
					return ERR_ARGS;
				}
				memcpy(passcode, optarg, sizeof(passcode));
                printf("Разраб хуй, ещё не запилил\n");
				printf("Уже что-то могу: %s!\n", passcode);
				break;
			case 'b':
				memset(board_name, '\0', sizeof(board_name));
                if ( sizeof(optarg) > sizeof(board_name) ) { //проверка
					printf("Не шути так больше\n");
					return ERR_ARGS;
				}
				memcpy(board_name, optarg, sizeof(board_name));
				break;
			case 'n':
				post_number = atoi(optarg);
				break;
			case 's':
				start_or_not = true;
				break;
			default:
				pomogite();
				return ERR_ARGS;
		}
	}

	if (start_or_not == false) return RET_OK;

	setlocale (LC_ALL, "");

	makabaSetup();

	#ifdef CAPTCHA_TEST

	printf("%s\n", getCaptchaSettingsJSON(board_name) );
	char *captcha_png_url = get2chaptchaPicURL(parse2chaptchaId(get2chaptchaIdJSON(board_name, lint2str(post_number))));
	printf("%s\n", captcha_png_url);
	long int pic_size;
	char *captcha_png = get2chaptchaPicPNG(captcha_png_url, &pic_size);
	captcha_png = (char *) calloc(pic_size + 1, sizeof(char));       // Нельзя доверять буферу
	captcha_png = memcpy(captcha_png, CURL_BUFF_BODY, pic_size); // curl`а
	FILE *captcha_png_file = fopen("captcha.png", "w");
	fwrite(captcha_png, sizeof(char), pic_size, captcha_png_file);
	fclose(captcha_png_file);
	convert_img("captcha.png", "captcha-test.ansi", true);
	show_img("captcha-test.ansi");
	makabaCleanup();
	return RET_PREEXIT;

	#endif

	long int threadsize = 0;
	char *thread_recv_ch = getThreadJSON(board_name, post_number, &threadsize, false); // Получаем указатель на скачанный тред
	fprintf(stderr, "threadsize = %u\n", threadsize);
	char *thread_ch = (char *) calloc(threadsize + 1, sizeof(char)); // Заказываем память под собственный буфер треда
	if (thread_ch == NULL) {
		fprintf(stderr, "[main]! Error: 'thread_ch' memory allocation\n");
		return ERR_MEMORY;
	}
	// Копируем скачанный тред из буфера загрузок, т.к. хранить там ненадежно
	thread_ch = memcpy(thread_ch, thread_recv_ch, threadsize);
	thread_ch[threadsize] = 0;
	fprintf(stderr, "Get OK\n");

	struct thread* thread = initThread(thread_ch, threadsize, true);
	fprintf(stderr, "Init OK\n");

	ncurses_init();
	ncurses_print_help();
	bool should_exit = false;
	for (int cur_post = 0; should_exit == false; ) {
		bool done = 0;
		while (done == false)
			switch (getch())
			{
				case 'Q': case 'q':
					done = 1;
					should_exit = true;
					break;
				case 'P': case 'p':
					printw(">>>> Открой капчу в другом терминале. Прости.\n\n");
					break;
				case 'H': case 'h':
					ncurses_print_help();
					break;
				case KEY_RIGHT: case KEY_DOWN:
					if (cur_post < thread->nposts - 1) {
						cur_post ++;
						ncurses_print_post(thread, cur_post);
					}
					else {
						printw(">>>> Последний пост\n");
					}
					done = 1;
					break;
				case KEY_LEFT: case KEY_UP:
					if (cur_post > 0) {
						cur_post --;
						ncurses_print_post(thread, cur_post);
					}
					else {
						printw(">>>> Первый пост\n");
					}
					done = 1;
					break;
			}
	}
	ncurses_exit();

	freeThread(thread);
	free(thread_ch);
	makabaCleanup();
	fprintf(stderr, "Cleanup done, exiting\n");

	return RET_OK;
}

int printPost (struct post* post,const bool show_email,const bool show_files) {
	if (post->comment == NULL) {
		fprintf(stderr, "! ERROR @printPost: Null comment in struct post\n");
		return ERR_BROKEN_POST;
	}
	if (post->num == NULL) {
		fprintf(stderr, "! ERROR @printPost: Null num in struct post\n");
		return ERR_BROKEN_POST;
	}
	if (post->date == NULL) {
		fprintf(stderr, "! ERROR @printPost: Null date in struct post\n");
		return ERR_BROKEN_POST;
	}

	// Заголовок отдельно
	if (show_email && (post->email != NULL)) {
		printw ("[=== %s (%s) #%d %s ===]\n",
			post->name, post->email, post->num, post->date);
	}
	else {
		printw ("[=== %s #%d %s ===]\n",
			post->name, post->num, post->date);
	}
	// Комментарий
	printw("%s\n\n", post->comment);

	return 0;
}

void ncurses_init() {
	initscr();
	raw();
	keypad (stdscr, TRUE);
	noecho();
}

void ncurses_exit() {
	endwin();
}

void ncurses_print_help() {
	printw(">>>> [LEFT] / [UP] предыдущий пост, [RIGHT] / [DOWN] следующий пост, [H] помощь, [Q] выход\n");
}

void ncurses_print_post(const struct thread *thread, const int postnum) {
	clear();
	fprintf(stderr, "Printing post #%d\n", postnum);
	printPost(thread->posts[postnum], true, true);
	refresh();
}
