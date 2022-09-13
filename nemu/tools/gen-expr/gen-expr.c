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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static int num = 0 ,num2 = 0;
static char buf[65536] = {};
static char ubuf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int choose(int Mod)
{
  return rand() % Mod;
}

static void gen_num()
{
  int len = choose(6) + 1;
  for (int i = 1; i <= len; i++) {
    buf[num++] = choose(9) + 1 + '0';
    ubuf[num2++] = buf[num - 1];
  }
  buf[num++] = 'u';
}

static void gen(char ch)
{
  buf[num++] = ch;
  ubuf[num2++] = ch;
}

static void gen_rand_op()
{
  int flag = choose(4);
  switch(flag) {
    case 0: buf[num++] = '+'; ubuf[num2++] = '+'; break;
    case 1: buf[num++] = '-'; ubuf[num2++] = '-'; break;
    case 2: buf[num++] = '*'; ubuf[num2++] = '*'; break;
    case 3: buf[num++] = '/'; ubuf[num2++] = '/'; break;
  }
}

static void gen_rand_expr(int d) {
  //buf[0] = '\0';
  if (d > 10) {
    gen_num();
    return ;
  }
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(d + 1); gen(')'); break;
    default: gen_rand_expr(d + 1); gen_rand_op(); gen_rand_expr(d + 1); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1000;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  FILE *newfp = fopen("data.txt", "w");
  assert(newfp != NULL);
  for (i = 0; i < loop; i ++) {
    num = 0; num2 = 0;
    gen_rand_expr(0);
    buf[num] = '\0';

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    int p = fscanf(fp, "%d", &result);
    p=1;
    assert(p);
    pclose(fp);
    fprintf(newfp, "%u %s\r\n", result, ubuf);
  }
  fclose(newfp);
  return 0;
}
