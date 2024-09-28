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
