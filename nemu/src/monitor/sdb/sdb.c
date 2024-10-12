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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

#include "memory/paddr.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_x(char *args)
{
  char *n = strtok(args, " ");
  char *baseaddr = strtok(NULL, " ");
  int len = 0;
  paddr_t addr = 0;
  // Probably cause segmentation fault
  sscanf(n, "%d", &len);
  sscanf(baseaddr, "%x", &addr);
  for (int i = 0; i < len; i++)
  {
    printf("%x\n", paddr_read(addr, 4)); // addr len
    addr = addr + 4;
  }
  return 0;
}

static int cmd_si(char *args)
{
  uint64_t a = 0;
  for (; args && *args; args++)
  {
    if (*args >= '0' && *args <= '9')
    {
      a = a * 10 + (*args - '0');
    }
    else
    {
      break;
    }
  }
  if (a == 0)
  {
    a = 1;
  }
  cpu_exec(a);
  return 0;
}

static int cmd_info(char *args)
{
  extern void sdb_watchpoint_display();
  for (; args && *args; args++)
  {
    switch (*args)
    {
    case 'r':
      isa_reg_display();
      break;
    case 'w':
      sdb_watchpoint_display();
      break;
    default:
      break;
    }
  }
  return 0;
}

static int cmd_d(char *args)
{
  extern void delete_watchpoint(int wp_id);
  if (args == NULL)
    printf("No args.\n");
  else
    delete_watchpoint(atoi(args));

  return 0;
}

static int cmd_w(char *args)
{
  extern void create_watchpoint(char *args);
  create_watchpoint(args);
  return 0;
}

static int cmd_q(char *args)
{
  // Fix exit code -1
  nemu_state.state = NEMU_QUIT;
  return -1;
}

#define buff_max_size 65536

static int rand_choose_3(int first, int second, int third)
{
  double first_rate = (double)first / (first + second + third);
  double second_rate = (double)second / (first + second + third);

  double rand_num = rand() / (double)RAND_MAX;
  if (rand_num < first_rate)
    return 1;
  else if (rand_num < first_rate + second_rate)
    return 2;
  else
    return 3;
}

static int rand_choose_4(int first, int second, int third, int fourth)
{
  double first_rate = (double)first / (first + second + third + fourth);
  double second_rate = (double)second / (first + second + third + fourth);
  double third_rate = (double)third / (first + second + third + fourth);

  double rand_num = rand() / (double)RAND_MAX;
  if (rand_num < first_rate)
    return 1;
  else if (rand_num < first_rate + second_rate)
    return 2;
  else if (rand_num < first_rate + second_rate + third_rate)
    return 3;
  else
    return 4;
}

static void gen_num(char* buff, int* len)
{
  int num = rand() % 1000000;
  (*len) += sprintf(buff + *len, "%d", num);
}

static void gen(char c, char* buff, int* len)
{
  buff[*len] = c;
  (*len)++;
}

static void gen_rand_op(char* buff, int* len)
{
  switch (rand_choose_4(4, 4, 4, 4)) {
    case 4: gen('+', buff, len); break;
    case 1: gen('-', buff, len); break;
    case 2: gen('*', buff, len); break;
    case 3: gen('/', buff, len); break;
  }
}

static void gen_rand_expr(char* buff, int* len, int* depth)
{
  int first = 8;
  int second = 7;
  int third = 9;

  (*depth)++;

  if (*len * 1.16 > buff_max_size * 1.0) {
    first *= 10;
  } else if ((buff_max_size - *len < 30) || (*depth > 30)) {
    second = 0;
    third = 0;
  }
  switch (rand_choose_3(first, second, third)) {
    case 1: gen_num(buff, len); break;
    case 2: gen('(', buff, len); gen_rand_expr(buff, len, depth); gen(')', buff, len); break;
    default: gen_rand_expr(buff, len, depth); gen_rand_op(buff, len); gen_rand_expr(buff, len, depth); break;
  }
  (*depth)--;
}

static int cmd_rexp(char *args)
{
  srand(time(NULL));
  static char buff[buff_max_size];
  #undef buff_max_size
  int len = 0;
  int depth = 0;
  gen_rand_expr(buff, &len, &depth);
  buff[len] = '\0';
  bool success;
  word_t a = expr(buff, &success);
  printf("Random expression: %s\n", buff);
  if (success)
  {
    printf("expression: %d\n", a);
  }
  else
  {
    printf("Invalid expression.\n");
  }
  return 0;
}

static int cmd_p(char *args)
{
  bool success;
  word_t a = expr(args, &success);
  if (success)
  {
    printf("expression: %d\n", a);
  }
  return 0;
}

static int cmd_help(char *args);

static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "Single-step the execution of the program at the given number of cycles", cmd_si},
    {"info", "Display information about the system", cmd_info},
    {"p", "Evaluate an expression and display the result", cmd_p},
    // Probably cause segmentation fault
    {"x", "Read memory content", cmd_x},
    // Probably cause segmentation fault
    {"w", "Create a watchpoint", cmd_w},
    // Probably cause segmentation fault
    {"d", "Delete a watchpoint", cmd_d},
    // Generate random expression
    {"rexp", "Generate a random expression", cmd_rexp},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
