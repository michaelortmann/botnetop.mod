/*
 * tclcmds.c -- part of botnetop.mod
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

#ifdef ENABLE_TCL_COMMANDS

static int onchan(struct userrec *u, struct chanset_t *chan)
{
  char s[UHOSTLEN];
  memberlist *m = NULL;

  for (m = chan->channel.member; m && m->nick[0]; m = m->next) {
    snprintf(s, sizeof s, "%s!%s", m->nick, m->userhost);
    if ((u == get_user_by_host(s)))
      return 1;
  }

  return 0;
}

static int tcl_bop_reqop STDVAR
{
  struct chanset_t *chan = NULL;

  BADARGS(3, 3, " channel need");
  if (!(chan = findchan_by_dname(argv[1]))) {
    Tcl_AppendResult(irp, "invalid channel: ", argv[1], NULL);
    return TCL_ERROR;
  }
  if (strcmp(argv[2], "op") && strcmp(argv[2], "key") && strcmp(argv[2], "invite") &&
      strcmp(argv[2], "limit") && strcmp(argv[2], "unban")) {
    Tcl_AppendResult(irp, "unknown need type: should be one of: ",
                     "op, key, invite, limit, unban", NULL);
    return TCL_ERROR;
  }
  if (!strcmp(argv[2], "op")) {
    if (isop(botname, chan)) {
      Tcl_AppendResult(irp, "already opped on channel: ", argv[1], NULL);
      return TCL_ERROR;
    }
  } else {
    if (ismember(chan, botname)) {
      Tcl_AppendResult(irp, "already on channel: ", argv[1], NULL);
      return TCL_ERROR;
    }
  }
  bnop_reqop(argv[1], argv[2]);

  return TCL_OK;
}

static int tcl_bop_askbot STDVAR
{
  struct chanset_t *chan = NULL;
  struct userrec *u = NULL;

  BADARGS(3, 3, " handle channel");
  if (!(chan = findchan_by_dname(argv[2]))) {
    Tcl_AppendResult(irp, "invalid channel: ", argv[2], NULL);
    return TCL_ERROR;
  }
  if (!(u = get_user_by_handle(userlist, argv[1])) || !(u->flags & USER_BOT)) {
    Tcl_AppendResult(irp, "invalid user: ", argv[1], NULL);
    return TCL_ERROR;
  }
  if (!onchan(u, chan)) {
    Tcl_AppendResult(irp, "user not on channel: ", argv[2], NULL);
    return TCL_ERROR;
  }
  if (!ismember(chan, botname)) {
    Tcl_AppendResult(irp, "not on channel: ", argv[2], NULL);
    return TCL_ERROR;
  }
  if (!isop(botname, chan)) {
    Tcl_AppendResult(irp, "not opped on channel: ", argv[2], NULL);
    return TCL_ERROR;
  }
  if (nextbot(argv[1]) < 0) {
    Tcl_AppendResult(irp, "not linked to: ", argv[1], NULL);
    return TCL_ERROR;
  }
  bnop_askbot(argv[1], argv[2]);

  return TCL_OK;
}

static int tcl_bop_letmein STDVAR
{
  struct chanset_t *chan = NULL;

  BADARGS(3, 3, " channel need");
  if (!(chan = findchan_by_dname(argv[1]))) {
    Tcl_AppendResult(irp, "invalid channel: ", argv[1], NULL);
    return TCL_ERROR;
  }
  if (strcmp(argv[2], "key") && strcmp(argv[2], "invite") && strcmp(argv[2], "limit") 
      && strcmp(argv[2], "unban")) {
    Tcl_AppendResult(irp, "unknown need type: should be one of: ",
                     "key, invite, limit, unban", NULL);
    return TCL_ERROR;
  }
  if (ismember(chan, botname)) {
    Tcl_AppendResult(irp, "already on channel: ", argv[1], NULL);
    return TCL_ERROR;
  }
  bnop_letmein(chan, argv[2]);

  return TCL_OK;
}

static int tcl_bop_lowbots STDVAR
{
  struct chanset_t *chan = NULL;

  BADARGS(2, 2, " channel");
  if (!(chan = findchan_by_dname(argv[1]))) {
    Tcl_AppendResult(irp, "invalid channel: ", argv[1], NULL);
    return TCL_ERROR;
  }
  if (lowbots(chan))
    Tcl_SetResult(irp, "1", TCL_STATIC);
  else
    Tcl_SetResult(irp, "0", TCL_STATIC);

  return TCL_OK;
}

static tcl_cmds botnetop_tcl_cmds[] =
{
  {"bop_reqop",		tcl_bop_reqop},
  {"bop_askbot",	tcl_bop_askbot},
  {"bop_letmein",	tcl_bop_letmein},
  {"bop_lowbots",	tcl_bop_lowbots},
  {0,			0}
};

#endif
