/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"
};

void isa_reg_display() {
  for (int i = 0; i < 32; i++) {
    printf("%s: 0x%08x\n", regs[i], isa_reg_str2val(regs[i], NULL));
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  word_t val = 0;
  if (s[0] == '$') {
    s++;
  }
  if (s[0] >= '0' && s[0] <= '9') {
    val = strtoul(s, NULL, 10);
  } else {
    for (int i = 0; i < 32; i++) {
      if (strcmp(s, regs[i]) == 0) {
        val = i;
        break;
      }
    }
  }
  if (success) {
    *success = (val < 32);
  }
  return gpr(val);
}