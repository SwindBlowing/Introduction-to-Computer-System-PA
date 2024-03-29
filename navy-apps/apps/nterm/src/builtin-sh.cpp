#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);
void close_terminal();
void refresh_terminal();

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static char totPATH[256] = "/usr/bin:";
static char envPATHs[10][256];
int envPATHnum = 0;
static char const *BusyBoxCommand[30] = {
	"base64", "cat", "echo", "ed", "false",
	"printenv", "sleep", "true", "basename", "cksum",
	"cmp", "comm", "cut", "dirname", "expand",
	"expr", "fold", "head", "nl", "od", 
	"paste", "shuf", "sum", "tac", "tee",
	"tr", "unexpand", "uniq", "wc", "yes"
};


static void sh_handle_cmd(const char *cmd) {
	if (!envPATHnum) {
		envPATHnum++;
		strcpy(envPATHs[0], "/usr/bin");
		setenv("PATH", (const char *)totPATH, 0);
	}
	int now = 0;
	int bufNum = 0;
	char bufs[50][99];
	while (cmd[now]) {
		if (cmd[now] == '\n') break;
		while (cmd[now] == ' ') now++;
		if (cmd[now] == '\n') break;
		bufNum++;
		int bufSize = 0;
		while (cmd[now] != ' ' && cmd[now] != '\n') {
			bufs[bufNum - 1][bufSize++] = cmd[now++];
		}
		bufs[bufNum - 1][bufSize++] = 0;
	}

	char * nterm_argv[50];

	if (strcmp(bufs[0], "quit") == 0) exit(0);
	else if (strcmp(bufs[0], "cd") == 0) {
		//printf("Here\n");
		//printf("%s %s\n", bufs[0], bufs[1]);
		//if (bufs[1][0] != '.') setenv("PATH", (const char *)bufs[1], 0);
		bool flag = 0;
		for (int i = 0; i < envPATHnum; i++)
			if (strcmp(bufs[1], envPATHs[i]) == 0) flag = 1;
		if (!flag) {
			strcpy(envPATHs[envPATHnum++], bufs[1]);
			strcat(totPATH, bufs[1]);
			strcat(totPATH, ":");
			//printf("now PATH: %s\n", totPATH);
			setenv("PATH", (const char *)totPATH, 1);
		}
		//printf("Here\n");
		//printf("Now PATH:%s\n", bufs[1] + 1);
	}
	else {
		bool isBBCommand = 0;
		for (int i = 0; i < sizeof(BusyBoxCommand) / sizeof(char *); i++)
			if (strcmp(bufs[0], BusyBoxCommand[i]) == 0) isBBCommand = 1;
		if (isBBCommand) {
			//printf("arrived here.\n");
			for (int i = 0; i < bufNum; i++)
				nterm_argv[i + 1] = (bufs[i]);
			nterm_argv[bufNum + 1] = NULL;
			strcpy(bufs[bufNum], "/bin/busybox");
			nterm_argv[0] = bufs[bufNum];
			execve("/bin/busybox", (char * const*)nterm_argv, NULL);
		}
		else {
			bool flag = 0;
			for (int i = 0; bufs[0][i]; i++)
				if (bufs[0][i] == '/') {
					flag = 1;
					break;
				}
			//printf("%p\n", ((char * const*)nterm_argv)[0]);
			close_terminal();
			//printf("%p\n", nterm_argv);
			if (flag) {
				for (int i = 1; i < bufNum; i++)
					nterm_argv[i] = (bufs[i]);
				nterm_argv[bufNum] = NULL;
				if (bufs[0][0] == '.') {
					nterm_argv[0] = bufs[0] + 1;
					execve(bufs[0] + 1, (char * const*)nterm_argv, NULL);
				}
				else {
					nterm_argv[0] = bufs[0];
					execve(bufs[0], (char * const*)nterm_argv, NULL);
				}
			}
			else {
				for (int i = 1; i < bufNum; i++)
					nterm_argv[i] = (bufs[i]);
				nterm_argv[bufNum] = NULL;
				nterm_argv[0] = bufs[0];
				execvp(bufs[0], (char * const*)nterm_argv);
			}
			exit(0); //if execve/execvp failed, just restart nterm.
		}
	}
	//free(bufs);
	//printf("Here\n");
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
