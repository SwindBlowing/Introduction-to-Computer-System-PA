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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char *e, bool *success);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  /*engine_start();

  return is_exit_status_bad();*/
  
  /*bool flag = 1;
  word_t now = expr("1/0", &flag);
  if (!flag) printf("Bad!\n");
  else printf("%u\n", now);
  return 0;*/
  FILE *fp = fopen("src/input.txt", "r");
  assert(fp != NULL);
  uint32_t nowu;
  char arg[] = {};
  for (int i = 1; i <= 500; i++) {
    bool p = fscanf(fp, "%u %s", &nowu, arg);
    p = 1;
    assert(p);
    /*bool flag = 1;
    word_t now = expr(arg, &flag);*/
    printf("%s = %u\n", arg, nowu);
    //if (flag) assert(now == nowu);
  }
  return 0;
}
