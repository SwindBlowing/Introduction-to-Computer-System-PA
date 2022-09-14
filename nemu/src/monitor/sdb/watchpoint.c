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

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char *args;
  word_t pre_val;
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
    WP_num++;
    wp_pool[WP_num - 1] = (WP){++totId, NULL, NULL, 0};
    if (head == NULL) head->next = &wp_pool[WP_num - 1];
    else {
      WP *pre_head = head->next;
      head->next = &wp_pool[WP_num - 1];
      wp_pool[WP_num - 1].next = pre_head;
    }
    return &wp_pool[WP_num - 1];
  }
  else {
    WP *now = free_->next;
    free_->next = now->next;
    *now = (WP){++totId, NULL, NULL, 0};
    if (head == NULL) head->next = now;
    else {
      WP *pre_head = head->next;
      head->next = now;
      now = pre_head;
    }
    return now;
  }
  return NULL;
}
/* TODO: Implement the functionality of watchpoint */

