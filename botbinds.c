/*
 * botbinds.c -- part of botnetop.mod
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

static int bnop_linkop(char *bot, char *via)
{
  struct chanset_t *chan = NULL;

  if (bop_linkop && (!egg_strcasecmp(bot, botnetnick) || !egg_strcasecmp(via, botnetnick))) {
    for (chan = chanset; chan != NULL; chan = chan->next) {
      if (!isop(botname, chan))
        bnop_reqop(chan->dname, "op");
    }
  }

  return 0;
}

static int bnop_reqtmr(char *bot, char *code, char *par)
{
  char *chname = NULL;
  struct chanset_t *chan = NULL;
  struct delay_t *d = NULL;
  struct userrec *u = NULL;

  chname = newsplit(&par);
  if (!(chan = findchan_by_dname(chname)) || !(u = get_user_by_handle(userlist, bot)))
    return 0;
  if (!matchattr(u, "b|-", chan->dname) || !matchattr(u, "o|o", chan->dname) || matchattr(u, "d|d", chan->dname))
    return 0;
  if (check_request_status(bot, chan->dname))
    return 0;
  if (check_flood_status(chan->dname))
    return 0;
  if (!bop_delay || lowbots(chan))
    bnop_askbot(bot, chan->dname);
  else {
    if (!(d = find_delay(chan->dname, bot))) {
      if (!(d = add_delay(chan->dname, bot)))
        return 0;
    }
    d->reqtime = now + (random() % bop_delay) + 2;
    strncpyz(d->handle, bot, strlen(bot) + 1);
  }

  return 0;
}

static int bnop_doiwantops(char *bot, char *code, char *par)
{
  int i, bufsize;
  char *chname = NULL, *buf = NULL;
  struct chanset_t *chan = NULL;

  chname = newsplit(&par);
  newsplit(&par);
  if (!(chan = findchan_by_dname(chname)))
    return 0;
  if ((i = nextbot(bot)) >= 0) {
    if (check_flood_status(chan->dname))
      return 0;
    if (check_delay_status(chan->dname, bot))
      return 0;
    bufsize = strlen(chan->dname) + strlen(botname) + strlen(botuserhost) + 14 + 1;
    buf = (char *) nmalloc(bufsize);
    if (buf == NULL)
      return 0;
    egg_snprintf(buf, bufsize, "yesiwantops %s %s %s", chan->dname, botname, 
                 strchr("~+-^=", botuserhost[0]) ? botuserhost + 1 : botuserhost);
    botnet_send_zapf(i, botnetnick, bot, buf);
    nfree(buf);
    if (bop_log >= 2) 
      putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_REQOPS, bot, chan->dname);
  }

  return 0;
}

static int bnop_botwantsops(char *bot, char *code, char *par)
{
  char *chname = NULL, *fromnick = NULL, *fromhost = NULL, s[UHOSTLEN];
  struct chanset_t *chan = NULL;
  struct userrec *u = NULL;
  memberlist *m = NULL;

  chname = newsplit(&par);
  fromnick = newsplit(&par);
  fromhost = newsplit(&par);
  if (!(chan = findchan_by_dname(chname)) || !(u = get_user_by_handle(userlist, bot)))
    return 0;
  if (!ismember(chan, botname) || !isop(botname, chan))
    return 0;
  if (!(m = ismember(chan, fromnick)) || chan_issplit(m))
    return 0;
  if (!matchattr(u, "b|-", chan->dname) && !matchattr(u, "o|o", chan->dname) && matchattr(u, "d|d", chan->dname))
    return 0;
  egg_snprintf(s, sizeof s, "%s!%s", m->nick, m->userhost);
  if (!(u = get_user_by_host(s)))
    return 0;
  if (bop_hcheck && fromhost[0] && 
      egg_strcasecmp(fromhost, strchr("~^+=-", m->userhost[0]) ? m->userhost + 1 : m->userhost))
    return 0;
  if (!matchattr(u, "o|o", chan->dname)) {
    if (!fromhost[0] || !bop_addhost)
      return 0;
    if (strchr("~^+=-", m->userhost[0]))
      egg_snprintf(s, sizeof s, "*!%s%s", strict_host ? "?" : "", m->userhost + 1);
    else
      egg_snprintf(s, sizeof s, "*!%s", m->userhost);
    addhost_by_handle(bot, s);
    putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_ADDHOST, s, bot);
  }
  if (bop_quickop) {
    if (!isop(fromnick, chan) || bop_osync)
      dprintf(DP_MODE, "MODE %s +o %s\n", chan->dname, fromnick);
    else
      return 0;
  } else {
    if (!isop(fromnick, chan))
      add_mode(chan, '+', 'o', fromnick);
    else if (bop_osync)
      dprintf(DP_MODE, "MODE %s +o %s\n", chan->dname, fromnick);
    else
      return 0;
  }
  if (bop_log >= 2) {
    if (egg_strcasecmp(fromnick, bot))
      putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_GAVEOPS2, bot, fromnick, chan->dname);
    else
      putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_GAVEOPS, bot, chan->dname);
  }

  return 0;
}

static int bnop_botwantsin(char *bot, char *code, char *par)
{
  int i, bufsize, bans = 0;
  char *chname = NULL, *fromnick = NULL, *fromhost = NULL, *buf = NULL, *mask = NULL, s[UHOSTLEN];
  struct chanset_t *chan = NULL;
  struct userrec *u = NULL;
  masklist *b = NULL;

  chname = newsplit(&par);
  fromnick = newsplit(&par);
  fromhost = newsplit(&par);
  if (!(chan = findchan_by_dname(chname)) || !(u = get_user_by_handle(userlist, bot)))
    return 0;
  if (!isop(botname, chan) || !matchattr(u, "b|-", chan->dname) || !matchattr(u, "fo|fo", chan->dname))
    return 0;
  if (fromhost[0]) {
    if ((mask = strchr(fromhost, '!')))
      mask++;
    else
      mask = fromhost;
    egg_snprintf(s, sizeof s, "*!*%s", strchr("~^+=-", mask[0]) ? mask + 1 : mask);
    if (strlen(s) > 70) {
      s[69] = '*';
      s[70] = 0;
    }
  }
  if (!strcmp(code, "wantkey") && (chan->channel.mode & CHANKEY)) {
    if (bop_onkey == 1)
      bnop_invite(chan, fromnick, bot, fromhost);
    else {
      if ((i = nextbot(bot)) >= 0) {
        if (check_flood_status(chan->dname))
          return 0;
        bufsize = strlen(chan->dname) + strlen(chan->channel.key) + 8 + 1;
        buf = (char *) nmalloc(bufsize);
        if (buf == NULL)
          return 0;
        egg_snprintf(buf, bufsize, "thekey %s %s", chan->dname, chan->channel.key);
        botnet_send_zapf(i, botnetnick, bot, buf);
        nfree(buf);
        if (bop_log >= 1) 
          putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_GAVEKEY, chan->dname, bot);
      }
    }
  } else if (!strcmp(code, "wantinvite") && (chan->channel.mode & CHANINV)) {
    if ((bop_oninvite == 1) && s[0]) {
      add_mode(chan, '+', 'I', s);
      if (bop_log >= 1)
        putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_SETINVITE, s, chan->dname, bot);
   } else
      bnop_invite(chan, fromnick, bot, fromhost);
  } else if (!strcmp(code, "wantlimit") && (chan->channel.maxmembers != 0)) {
    if (bop_onlimit == 1)
      bnop_invite(chan, fromnick, bot, fromhost);
    else {
      add_mode(chan, '+', 'l', int_to_base10(chan->channel.members + 1));
      if (bop_log >= 1)
        putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_RAISEDLIMIT, chan->dname, bot);
    }
  } else if (!strcmp(code, "wantunban")) {
    if ((bop_onunban == 1) && s[0]) {
      add_mode(chan, '+', 'e', s);
      if (bop_log >= 1)
        putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_SETEXEMPT, s, chan->dname, bot);
    } else if (bop_onunban == 2)
      bnop_invite(chan, fromnick, bot, strchr("~+-^=", mask[0]) ? mask + 1 : mask);
    else {
      for (b = chan->channel.ban; b->mask[0]; b = b->next) {
        if (wild_match(b->mask, fromhost)) {
          add_mode(chan, '-', 'b', b->mask);
          bans++;
        }
      }
      if ((bop_log >= 1) && (bans > 0))
        putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_UNBANNED, bot, chan->dname, bans);
    }
  }

  return 0;
}

static int bnop_joinkey(char *bot, char *code, char *par)
{
  char *chname = NULL, *key = NULL;
  struct chanset_t *chan = NULL;

  chname = newsplit(&par);
  key = newsplit(&par);
  if (!(chan = findchan_by_dname(chname)) || ismember(chan, botname))
    return 0;
  dprintf(DP_SERVER, "JOIN %s %s\n", chan->dname, key);

  return 0;
}

static cmd_t botnetop_link[] =
{
  {"*",	"",	(Function) bnop_linkop,	"bop_linkop"},
  {0,	0,	0,			0}
};

static cmd_t botnetop_bot[] =
{
  {"doyawantops",	"",	(Function) bnop_doiwantops,	"bop_doiwantops"},
  {"yesiwantops",	"",	(Function) bnop_botwantsops,	"bop_botwantsops"},
  {"reqops",		"",	(Function) bnop_reqtmr,		"bop_reqtmr"},
  {"wantkey",		"",	(Function) bnop_botwantsin,	"bop_botwantsin"},
  {"wantinvite",	"",	(Function) bnop_botwantsin,	"bop_botwantsin"},
  {"wantlimit",		"",	(Function) bnop_botwantsin,	"bop_botwantsin"},
  {"wantunban",		"",	(Function) bnop_botwantsin,	"bop_botwantsin"},
  {"thekey",		"",	(Function) bnop_joinkey,	"bop_joinkey"},
  {0,			0,	0,				0}
};
