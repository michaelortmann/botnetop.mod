/* 
 * botnetop.c -- part of botnetop.mod
 */
/*
 * Copyright (C) 2000, 2001, 2002  Teemu Hjelt <temex@iki.fi>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define MODULE_NAME "botnetop"
#define MAKING_BOTNETOP
#define MODULE_VERSION "1.1.0-gamma"
#include "../module.h"
#include "../irc.mod/irc.h"
#include "../server.mod/server.h"
#include "../channels.mod/channels.h"

#undef global
static Function *global = NULL, *irc_funcs = NULL, *server_funcs = NULL, *channels_funcs = NULL;

static struct delay_t *delay_start = NULL;
static struct who_t *who_start = NULL;
static struct flood_t *flood_start = NULL;
static struct request_t *request_start = NULL;

/* Default variables */
static int strict_host = 0;
static int bop_delay = 20;
static int bop_maxreq = 3;
static int bop_modeop = 0;
static int bop_linkop = 1;
static int bop_icheck = 0;
static int bop_hcheck = 1;
static int bop_osync = 0;
static int bop_addhost = 0;
static int bop_log = 2;

/* Additional variables */
static int bop_modedeop = 0;
static int bop_onunban = 0; 
static int bop_oninvite = 0; 
static int bop_onlimit = 0;
static int bop_onkey = 0;
static int bop_lowbotslimit = 3;
static int bop_flood_amount = 10;
static int bop_flood_time = 20;
static int bop_ignore = 20;
static int bop_quickop = 0;

#include "botnetop.h"
#include "botbinds.c"
#include "botcmds.c"
#include "delay.c"
#include "flood.c"
#include "ircbinds.c"
#include "misc.c"
#include "request.c"
#include "tclcmds.c"
#include "who.c"

static void botnetop_hook_secondly()
{
  check_delay();
  check_who();
  check_flood();
}

static void botnetop_hook_hourly()
{
  struct delay_t *d = NULL, *dnext = NULL;
  struct flood_t *f = NULL, *fnext = NULL;

  for (d = delay_start; d != NULL; d = dnext) {
    dnext = d->next;
    if (!findchan_by_dname(d->chan))
      del_delay(d);
  }
  for (f = flood_start; f != NULL; f = fnext) {
    fnext = f->next;
    if (!findchan_by_dname(f->chan))
      del_flood(f);
  }
}

static int botnetop_expmem()
{
  return delay_expmem() + who_expmem() + flood_expmem() + request_expmem();
}

static void botnetop_report(int idx, int details)
{
  int dtot = 0, wtot = 0, ftot = 0, rtot = 0;
  struct delay_t *d = NULL;
  struct who_t *w = NULL;
  struct flood_t *f = NULL;
  struct request_t *r = NULL;

  for (d = delay_start; d != NULL; d = d->next)
    dtot++;
  for (w = who_start; w != NULL; w = w->next)
    wtot++;
  for (f = flood_start; f != NULL; f = f->next)
    ftot++;
  for (r = request_start; r != NULL; r = r->next)
    rtot++;
  if (details) {
    dprintf(idx, "    Module version: %s\n", MODULE_VERSION);
#ifdef ENABLE_TCL_COMMANDS
    dprintf(idx, "    Tcl commands are enabled.\n");
#else
    dprintf(idx, "    Tcl commands are disabled.\n");
#endif
    dprintf(idx, "    %d delay record%s using %d bytes\n", dtot, dtot == 1 ? "" : "s", delay_expmem());
    dprintf(idx, "    %d who record%s using %d bytes\n", wtot, wtot == 1 ? "" : "s", who_expmem());
    dprintf(idx, "    %d flood record%s using %d bytes\n", ftot, ftot == 1 ? "" : "s", flood_expmem());
    dprintf(idx, "    %d request record%s using %d bytes\n", rtot, rtot == 1 ? "" : "s", request_expmem());
    dprintf(idx, "    botnetop.mod is using altogether %d bytes\n", botnetop_expmem());
  }
}

static tcl_ints botnetop_tcl_ints[] =
{
  {"strict-host",	&strict_host,		0},
  {"bop_delay",		&bop_delay,		0},
  {"bop_maxreq",	&bop_maxreq,		0},
  {"bop_modeop",	&bop_modeop,		0},
  {"bop_linkop",	&bop_linkop,		0},
  {"bop_icheck",	&bop_icheck,		0},
  {"bop_hcheck",	&bop_hcheck,		0},
  {"bop_osync",		&bop_osync,		0},
  {"bop_addhost",	&bop_addhost,		0},
  {"bop_log",		&bop_log,		0},
  {"bop_modedeop",	&bop_modedeop,		0},
  {"bop_onunban",	&bop_onunban,		0},
  {"bop_oninvite",	&bop_oninvite,		0},
  {"bop_onlimit",	&bop_onlimit,		0},
  {"bop_onkey",		&bop_onkey,		0},
  {"bop_lowbotslimit",	&bop_lowbotslimit,	0},
  {"bop_ignore",	&bop_ignore,		0},
  {"bop_quickop",	&bop_quickop,		0},
  {0,			0,			0}
};

static tcl_coups botnetop_tcl_coups[] =
{
  {"bop_flood",	&bop_flood_amount,	&bop_flood_time},
  {0,		0,			0}
};

static char *botnetop_close()
{
  del_hook(HOOK_SECONDLY, (Function) botnetop_hook_secondly);
  del_hook(HOOK_HOURLY, (Function) botnetop_hook_hourly);
  rem_tcl_ints(botnetop_tcl_ints);
  rem_tcl_coups(botnetop_tcl_coups);
#ifdef ENABLE_TCL_COMMANDS
  rem_tcl_commands(botnetop_tcl_cmds);
#endif
  rem_builtins(H_join, botnetop_join);
  rem_builtins(H_mode, botnetop_mode);
  rem_builtins(H_raw, botnetop_raw);
  rem_builtins(H_link, botnetop_link);
  rem_builtins(H_bot, botnetop_bot);
  rem_builtins(H_need, botnetop_need);
  delay_free_mem();
  who_free_mem();
  flood_free_mem();
  request_free_mem();
  module_undepend(MODULE_NAME);

  return NULL;
}

/* Ugly hack to get it work on Cygwin dynamically. */
#if defined (__CYGWIN__) && !defined(STATIC)
  __declspec(dllexport) char * __cdecl botnetop_start();
#else
  char *botnetop_start();
#endif

static Function botnetop_table[] =
{
  (Function) botnetop_start,
  (Function) botnetop_close,
  (Function) botnetop_expmem,
  (Function) botnetop_report,
};

char *botnetop_start(Function * global_funcs)
{
  global = global_funcs;

  module_register(MODULE_NAME, botnetop_table, 1, 2);
  if (!module_depend(MODULE_NAME, "eggdrop", 108, 4)) {
        module_undepend(MODULE_NAME);
        return "This module requires Eggdrop 1.8.4 or later.";
  }
  if (!(irc_funcs = module_depend(MODULE_NAME, "irc", 1, 0))) {
    module_undepend(MODULE_NAME);
    return "You need the irc module to use the botnetop module.";
  }
  if (!(server_funcs = module_depend(MODULE_NAME, "server", 1, 0))) {
    module_undepend(MODULE_NAME);
    return "You need the server module to use the botnetop module.";
  }
  if (!(channels_funcs = module_depend(MODULE_NAME, "channels", 1, 0))) {
    module_undepend(MODULE_NAME);
    return "You need the channels module to use the botnetop module.";
  }
  add_hook(HOOK_SECONDLY, (Function) botnetop_hook_secondly);
  add_hook(HOOK_HOURLY, (Function) botnetop_hook_hourly);
  add_tcl_ints(botnetop_tcl_ints);
  add_tcl_coups(botnetop_tcl_coups);
#ifdef ENABLE_TCL_COMMANDS
  add_tcl_commands(botnetop_tcl_cmds);
#endif
  add_builtins(H_join, botnetop_join);
  add_builtins(H_mode, botnetop_mode);
  add_builtins(H_raw, botnetop_raw);
  add_builtins(H_link, botnetop_link);
  add_builtins(H_bot, botnetop_bot);
  add_builtins(H_need, botnetop_need);

  return NULL;
}
