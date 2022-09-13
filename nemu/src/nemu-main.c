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
  word_t now = expr("(97896+((((((367395-4199)+114683/(2445-((85+(((((154419)))))))))+((((585*63))-25+((89/516232*849669+(((2))))))))*((((86))+(((7669)-94437)/(7+845-468795/9+(6))-68548-(91)*((5-83*4*645+186187-(891/619))+(34999)*9))/(9)-(141874))+(71*15834)*(17582)-2-((3144))*((((37865)-(3993*57)+68)))-(481994)/((31863+((134319)))+(5)*742173*6437))))))", &flag);
  if (!flag) printf("Bad!\n");
  else printf("%u\n", now);
  return 0;*/
  FILE *fp = fopen("src/input.txt", "r");
  assert(fp != NULL);
  uint32_t nowu;
  for (int i = 1; i <= 500; i++) {
    char arg[] = {};
    bool p = fscanf(fp, "%u %s", &nowu, arg);
    printf("%s ", arg);
    p = 1;
    assert(p);
    bool flag = 1;
    word_t now = expr(arg, &flag);
    printf("%u = %u\n", now, nowu);
    if (flag) {
      assert(now == nowu);
    }
  }
  return 0;
}
