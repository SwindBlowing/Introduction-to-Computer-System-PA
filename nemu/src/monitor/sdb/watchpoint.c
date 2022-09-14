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

#include "sdb.h"

#define NR_WP 32

word_t expr(char *e, bool *success);

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char args[65524];
  word_t pre_val;
  bool Divided0;
  /* TODO: Add more members if necessary */

} WP;

static int totId = 0;
WP wp_pool[NR_WP] = {};
WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp()
{
  if (free_ == NULL) {
    printf("No free watchpoint. Add failed!\n");
    return NULL;
  }
  else {
    WP *now = free_;
    free_ = now->next;
    *now = (WP){++totId, NULL, "", 0, 0};
    now->next = head;
    head = now;
    //now->args = "1+1";
    //printf("%s\n", head->args);
    //printf("%p", now);
    //printf("%s=%d\n", now->args, now->pre_val);
    return now;
  }
  return NULL;
}

void free_wp(word_t N)
{
  WP *now = head, *pre = NULL;
  while (now != NULL && now->NO != N) pre = now, now = now->next;
  if (now == NULL) {
    printf("No target point. Free Failed!\n");
    return ;
  }
  printf("Deleted the watchpoint No.%d.", now->NO);
  if (now == head) 
    head = now->next;
  else 
    pre->next = now->next;
  *now = (WP){now->NO, NULL, "", 0, 0};
  now->next = free_;
  free_ = now;
}

bool wp_pause()
{
  //assert(head != NULL);
  WP *now = head;
  while (now != NULL) {
    bool success = 1;
    //printf("%p", now);
    //printf("%s\n", now->args);
    word_t now_val = expr(now->args, &success);
    if (!success) {
      if (now->Divided0) ;
      else {
        now->Divided0 = 1;
        printf("Watchpoint %d changed, from not divided 0 to divided.\n", now->NO);
        return false;
      }
    }
    now->Divided0 = 0;
    if (now_val != now->pre_val) {
      printf("Watchpoint %d changed, from %u to %u.\n", now->NO, now->pre_val, now_val);
      now->pre_val = now_val;
      return false;
    }
    now = now->next;
  }
  return true;
}
/* TODO: Implement the functionality of watchpoint */

