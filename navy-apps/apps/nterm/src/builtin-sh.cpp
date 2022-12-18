#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);
void close_terminal();

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

static char bufs[99][99];

static void sh_handle_cmd(const char *cmd) {
	int now = 0;
	int bufNum = 0;
	while (cmd[now]) {
		if (cmd[now] == '\n') break;
		while (cmd[now] == ' ') now++;
		if (cmd[now] == '\n') break;
		bufNum++;
		int bufSize = 0;
		while (cmd[now] != ' ' && cmd[now] != '\n') {
			bufs[bufNum - 1][bufSize++] = cmd[now++];
		}
	}

	if (strcmp(bufs[0], "quit") == 0) exit(1);
	else if (strcmp(bufs[0], "cd") == 0) {
		if (bufs[1][0] != '.') setenv("PATH", bufs[1], 0);
		else setenv("PATH", bufs[1] + 1, 0);
		//printf("Now PATH:%s\n", bufs[1] + 1);
	}
	else {
		bool flag = 0;
		for (int i = 0; bufs[0][i]; i++)
			if (bufs[0][i] == '/') {
				flag = 1;
				break;
			}
		close_terminal();
		if (flag) {
			if (bufs[0][0] == '.') execve(bufs[0] + 1, NULL, NULL);
			else execve(bufs[0], NULL, NULL);
		}
		else 
			execvp(bufs[0], NULL);
	}
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
