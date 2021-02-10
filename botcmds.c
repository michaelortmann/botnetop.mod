/*
 * botcmds.c -- part of botnetop.mod
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

static int bnop_reqop(char *chname, char *need) 
{
  int i, bufsize;
  char *buf = NULL;
  tand_t *bot = NULL;
  struct chanset_t *chan = NULL;
  struct userrec *u = NULL;

  if (!(chan = findchan_by_dname(chname)))
    return 0;
  if (!strcmp(need, "op")) {
    bufsize = strlen(chan->dname) + 7 + 1;
    buf = (char *) nmalloc(bufsize);
    if (buf == NULL)
      return 0;
    egg_snprintf(buf, bufsize, "reqops %s", chan->dname);
    for (bot = tandbot; bot != NULL; bot = bot->next) {
      if (!(u = get_user_by_handle(userlist, bot->bot)))
        continue;
      if (handisop(bot->bot, chan) && matchattr(u, "o|o", chan->dname) && ((i = nextbot(bot->bot)) >= 0))
        botnet_send_zapf(i, botnetnick, bot->bot, buf);
    }
    nfree(buf);
  } else 
    bnop_letmein(chan, need);

  return 0;
}

static void bnop_askbot(char *hand, char *chname)
{
  int i, bufsize;
  char *buf = NULL;
  struct chanset_t *chan = NULL;
  struct userrec *u = NULL;
  struct request_t *r = NULL;

  if ((r = find_request(hand, chname)))
    del_request(r);
  if (!(chan = findchan_by_dname(chname)) || !(u = get_user_by_handle(userlist, hand)))
    return;
  if (!ismember(chan, botname) || !isop(botname, chan))
    return;
  if ((i = nextbot(hand)) >= 0) {
    bufsize = strlen(chan->dname) + strlen(botname) + 13 + 1;
    buf = (char *) nmalloc(bufsize);
    if (buf == NULL)
      return;
    egg_snprintf(buf, bufsize, "doyawantops %s %s", chan->dname, botname);
    botnet_send_zapf(i, botnetnick, hand, buf);
    nfree(buf);
  }
}

static void bnop_invite(struct chanset_t *chan, char *nick, char *hand, char *uhost)
{
  if (bop_icheck && uhost[0]) {
    if (!find_who(nick)) {
      if (!add_who(nick, chan->dname, hand, uhost))
        return;
    }
    dprintf(DP_MODE, "USERHOST %s\n", nick); /* Need to use DP_MODE queue here because
                                                eggdrop can't stack USERHOST properly. */
  } else {
    dprintf(DP_SERVER, "INVITE %s %s\n", nick, chan->dname);
    if (bop_log >= 1) {
      if (egg_strcasecmp(nick, hand))
        putlog(LOG_MISC, "*", "botnetop.mod " BOTNETOP_INVITED2, hand, nick, chan->dname);
      else
        putlog(LOG_MISC, "*", "botnetop.mod " BOTNETOP_INVITED, hand, chan->dname);
    }
  }
}

static void bnop_letmein(struct chanset_t *chan, char *need)
{
  int i, bufsize, bots = 0;
  char *buf = NULL, log[LOGSIZE];
  tand_t *bot = NULL;
  struct userrec *u = NULL;

  if (ismember(chan, botname))
    return;

  bufsize = strlen(chan->dname) + strlen(botname) + strlen(botname) + strlen(botuserhost) + 13 + 1;
  buf = (char *) nmalloc(bufsize);
  if (buf == NULL)
    return;

  if (!strcmp(need, "key")) {
      egg_snprintf(buf, bufsize, "wantkey %s %s %s", chan->dname, botname, 
                   strchr("~+-^=", botuserhost[0]) ? botuserhost + 1 : botuserhost);
      strncpyz(log, "botnetop.mod: " BOTNETOP_REQKEY, sizeof log);
  } else if (!strcmp(need, "invite")) {
      egg_snprintf(buf, bufsize, "wantinvite %s %s %s", chan->dname, botname, 
                   strchr("~+-^=", botuserhost[0]) ? botuserhost + 1 : botuserhost);
      strncpyz(log, "botnetop.mod: " BOTNETOP_REQINVITE, sizeof log);
  } else if (!strcmp(need, "limit")) {
      egg_snprintf(buf, bufsize, "wantlimit %s %s %s", chan->dname, botname, 
                   strchr("~+-^=", botuserhost[0]) ? botuserhost + 1 : botuserhost);
      strncpyz(log, "botnetop.mod: " BOTNETOP_REQLIMIT, sizeof log);
  } else if (!strcmp(need, "unban")) {
      egg_snprintf(buf, bufsize, "wantunban %s %s %s!%s", chan->dname, 
                   botname, botname, botuserhost);
      strncpyz(log, "botnetop.mod: " BOTNETOP_REQUNBAN, sizeof log);
  }

  if (buf[0]) {
    for (bot = tandbot; bot != NULL; bot = bot->next) {
      if (!(u = get_user_by_handle(userlist, bot->bot)))
        continue;
      if (!matchattr(u, "b|-", chan->dname) || !matchattr(u, "o|o", chan->dname))
        continue;
      if ((i = nextbot(bot->bot)) >= 0) {
        botnet_send_zapf(i, botnetnick, bot->bot, buf);
        bots++;
      }
    }
    nfree(buf);
    if ((bop_log >= 1) && (bots > 0))
      putlog(LOG_MISC, "*", log, chan->dname, bots, bots == 1 ? "" : "s");
  }
}

static cmd_t botnetop_need[] =
{
  {"*",	"",	(IntFunc) bnop_reqop,	"bop_reqop"},
  {0,	0,	0,			0}
};
