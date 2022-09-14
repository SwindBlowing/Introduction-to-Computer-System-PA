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
  char *args;
  word_t pre_val;
  bool Divided0;
  /* TODO: Add more members if necessary */

} WP;

int WP_num = 0, totId = 0;
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

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
    if (WP_num == NR_WP) {
      printf("No free watchpoint. Add failed!\n");
      return NULL;
    }
    WP_num++;
    wp_pool[WP_num - 1] = (WP){++totId, NULL, NULL, 0, 0};
    if (head == NULL) head = &wp_pool[WP_num - 1];
    else {
      wp_pool[WP_num - 1].next = head;
      head = &wp_pool[WP_num - 1];
    }
    return &wp_pool[WP_num - 1];
  }
  else {
    WP *now = free_;
    free_ = now->next;
    *now = (WP){++totId, NULL, NULL, 0, 0};
    if (head == NULL) head = now;
    else {
      now->next = head;
      head = now;
    }
    return now;
  }
  printf("New watchpoint NO.%d.\n", totId);
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
  if (now == head) 
    head = now->next;
  else 
    pre->next = now->next;
  *now = (WP){now->NO, NULL, NULL, 0, 0};
  if (free_ == NULL) free_ = now;
  else {
    now->next = free_;
    free_ = now;
  }
}

bool wp_pause()
{
  WP *now = head;
  now = now->next;
  while (now != NULL) {
    bool success = 1;
    word_t now_val = expr(now->args, &success);
    if (!success) {
      if (now->Divided0) ;
      else {
        now->Divided0 = 1;
        printf("Watchpoint %d changed, from now divided 0 to divided.\n", now->NO);
        return false;
      }
    }
    now->Divided0 = 0;
    if (now_val != now->pre_val) {
      printf("Watchpoint %d changed, from %u to %u.\n", now->NO, now->pre_val, now_val);
      now->pre_val = now_val;
      return false;
    }
  }
  return true;
}
/* TODO: Implement the functionality of watchpoint */

