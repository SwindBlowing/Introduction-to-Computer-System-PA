/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "memory/paddr.h"

static int is_batch_mode = false;

word_t expr(char *e, bool *success);

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char *args;
  word_t pre_val;
  bool Divided0;
  /* TODO: Add more members if necessary */

} WP;

void init_regex();
void init_wp_pool();
bool make_token(char *e);
WP* new_wp();
void free_wp(word_t N);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Pause the program after stepping through n steps (n defaluts to 1)", cmd_si},
  { "info", "Print register status or watchpoint information", cmd_info},
  { "x", "Scan memory", cmd_x},
  { "p", "Calculate the expression", cmd_p},
  { "w", "Add watchpoint", cmd_w},
  { "d", "Delete watchpoint", cmd_d},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

bool check_number(char *arg, int type)
{
  char *now = arg;
  if (now == NULL) return 0;
  if (type == 258) {
    while (*now) {
      if (*now < '0' || *now > '9') 
        return 0;
      now++;
    }
  }
  else {
    while (*now) {
      if ((*now < '0' || *now > '9') && (*now < 'a' || *now > 'f')) 
        return 0;
      now++;
    }
  }
  return 1;
}

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  uint64_t n = 1;
  if (arg == NULL) {
    cpu_exec(n);
    return 0;
  }
  bool flag = check_number(args, 258);
  if (flag) {
    sscanf(arg, "%lu", &n);
    cpu_exec(n);
  }
  else printf("Unknown command '%s'\n", arg);
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL || strlen(arg) > 1) {
    printf("Unknown command '%s'\n", arg);
    return 0;
  }
  switch (*arg)
  {
    case 'f':
      isa_reg_display();
      break;
    case 'w':
      break;
    default:
      printf("Unknown command '%s'\n", arg);
      break; 
  }
  return 0;
}

static int cmd_x(char *args)
{
  bool success = 1;
  char *arg1 = strtok(NULL, " ");
  char *arg2 = arg1 + strlen(arg1) + 1;
  uint32_t N = 0;
  sscanf(arg1, "%u", &N);
  paddr_t EXPR = 0;
  EXPR = expr(arg2, &success);
  if (!success) {
    printf("Bad expression '%s'\n", args);
    return 0;
  }
  for (int i = 0; i < N; i++) {
    printf("0x%x    0x", EXPR);
    for (int j = 3; j >= 0; j--) 
      printf("%02x", paddr_read(EXPR + j, 1));
    EXPR += 4;
    printf("\n");
  }
  return 0;
}

static int cmd_p(char *args)
{
  bool success = 1;
  word_t ans = 0;
  ans = expr(args, &success);
  if (!success) printf("Bad expression '%s'\n", args);
  else printf("%u\n", ans);
  return 0;
}

static int cmd_w(char *args)
{
  WP *now = new_wp();
  if (now != NULL) {
    now->args = args;
    bool success = 1;
    now->pre_val = expr(args, &success);
    if (!success) now->Divided0 = 1;
  }
  return 0;
}

static int cmd_d(char *args)
{
  bool success = 1;
  word_t N = expr(args, &success);
  if (success)
    free_wp(N);
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
