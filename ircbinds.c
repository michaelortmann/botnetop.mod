/* 
 * ircbinds.c -- part of botnetop.mod
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

static int bnop_jointmr(char *nick, char *uhost, char *hand, char *chname)
{
  struct chanset_t *chan = NULL;
  struct delay_t *d = NULL;
  struct userrec *u = NULL;

  if (!(chan = findchan_by_dname(chname)))
    return 0;
  if (rfc_casecmp(nick, botname)) {
    /* Somebody joined the channel - Check if it's a bot and suggest ops. */
    if (!(u = get_user_by_handle(userlist, hand)))
      return 0;
    if (!matchattr(u, "b|-", chan->dname) || !matchattr(u, "o|o", chan->dname) || matchattr(u, "d|d", chan->dname))
      return 0;
    if (check_request_status(hand, chan->dname))
      return 0;
    if (!bop_delay || lowbots(chan))
      bnop_askbot(hand, chan->dname);
    else {
      if (!(d = find_delay(chan->dname, hand))) {
        if (!(d = add_delay(chan->dname, hand)))
          return 0;
      }
      d->reqtime = now + (random() % bop_delay) + 5;
      d->handle = nrealloc(d->handle, strlen(hand) + 1);
      strcpy(d->handle, hand);
    }
  } else
    /* We joined the channel - Request ops from all linked bots. */
    bnop_reqop(chan->dname, "op");

  return 0;
}

static int bnop_modeop(char *nick, char *uhost, char *hand, char *chname, char *mode, char *victim)
{
  int i, bufsize;
  char s[NICKLEN + UHOSTLEN], *buf = NULL;
  struct chanset_t *chan = NULL;
  struct userrec *u = NULL;
  struct delay_t *d = NULL;
  memberlist *m = NULL;

  if (bop_modeop) {
    if (!(chan = findchan_by_dname(chname)))
      return 0;
    if (!rfc_casecmp(victim, botname)) {
      /* We got opped - Suggest ops for all the deopped bots on the channel. */
      for (m = chan->channel.member; m && m->nick[0]; m = m->next) {
        snprintf(s, sizeof s, "%s!%s", m->nick, m->userhost);
        if ((u = get_user_by_host(s)) && !chan_hasop(m)) {
          if (!matchattr(u, "b|-", chan->dname) || !matchattr(u, "o|o", chan->dname) || matchattr(u, "d|d", chan->dname))
            continue;
          if (check_request_status(u->handle, chan->dname))
            continue;
          if (!bop_delay || lowbots(chan))
            bnop_askbot(u->handle, chan->dname);
          else {
            if (!(d = find_delay(chan->dname, u->handle))) {
              if (!(d = add_delay(chan->dname, u->handle)))
                continue;
            }
            d->reqtime = now + (random() % bop_delay) + 5;
            d->handle = nrealloc(d->handle, strlen(u->handle) + 1);
            strcpy(d->handle, u->handle);
          }
        }
      }
    } else if (!isop(botname, chan) && rfc_casecmp(nick, botname)) {
      /* Some other got opped - Ask for ops if it was a bot. */
      if (!(m = ismember(chan, victim)))
        return 0;
      snprintf(s, sizeof s, "%s!%s", m->nick, m->userhost);
      if (!(u = get_user_by_host(s)))
        return 0;
      if (matchattr(u, "b|-", chan->dname) && matchattr(u, "o|o", chan->dname) && ((i = nextbot(u->handle)) >= 0)) {
        bufsize = strlen(chan->dname) + 7 + 1;
        buf = nmalloc(bufsize);
        snprintf(buf, bufsize, "reqops %s", chan->dname);
        botnet_send_zapf(i, botnetnick, u->handle, buf);
        nfree(buf);
      }
    }
  }

  return 0;
}

static int bnop_modedeop(char *nick, char *uhost, char *hand, char *chname, char *mode, char *victim) 
{
  char s[NICKLEN + UHOSTLEN];
  struct chanset_t *chan = NULL;
  struct userrec *u = NULL;
  struct delay_t *d = NULL;
  memberlist *m = NULL;

  if (bop_modedeop) {
    if (!(chan = findchan_by_dname(chname)))
      return 0;
    if (!rfc_casecmp(victim, botname)) 
      /* We got deopped - Ask for ops from all linked bots. */
      bnop_reqop(chan->dname, "op");
    else if (rfc_casecmp(nick, botname)) {
      /* Some other got deopped and the deopper wasn't us - Check if it was a bot and suggest ops. */
      if (!(m = ismember(chan, victim)))
        return 0;
      snprintf(s, sizeof s, "%s!%s", m->nick, m->userhost);
      if (!(u = get_user_by_host(s)))
        return 0;
      if (!matchattr(u, "b|-", chan->dname) || !matchattr(u, "o|o", chan->dname) || matchattr(u, "d|d", chan->dname))
        return 0;
      if (check_request_status(u->handle, chan->dname))
        return 0;
      if (!bop_delay || lowbots(chan))
        bnop_askbot(u->handle, chan->dname);
      else {
        if (!(d = find_delay(chan->dname, u->handle))) {
          if (!(d = add_delay(chan->dname, u->handle)))
            return 0;
        }
        d->reqtime = now + (random() % bop_delay) + 5;
        d->handle = nrealloc(d->handle, strlen(u->handle) + 1);
        strcpy(d->handle, u->handle);
      }
    }
  }
  
  return 0;
}

static int bnop_userhost(char *from, char *msg)
{
  char *reply = NULL, fromnick[NICKLEN], s[UHOSTLEN];
  struct who_t *w = NULL;
  struct chanset_t *chan = NULL;

  reply = nmalloc(strlen(msg) + 1);
  strcpy(reply, msg);
  newsplit(&reply);
  reply = newsplit(&reply);
  splitc(fromnick, reply + 1, '=');
  splitc(fromnick, fromnick, '*');
  if ((w = find_who(fromnick))) {
    if (!(chan = findchan_by_dname(w->chan)))
      return 0;
    strlcpy(s, strchr("~+-^=", reply[2]) ? reply + 3 : reply + 2, sizeof s);
    if (!strcasecmp(w->uhost, s)) {
      dprintf(DP_SERVER, "INVITE %s %s\n", fromnick, chan->dname);
      if (bop_log >= 1) {
        if (strcasecmp(fromnick, w->handle)) {
          putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_INVITED2, w->handle, fromnick, chan->dname);
        } else {
          putlog(LOG_MISC, "*", "botnetop.mod: " BOTNETOP_INVITED, w->handle, chan->dname);
        }
      }
    }
    del_who(w);
  }
  nfree(reply);

  return 0;
}

static cmd_t botnetop_mode[] =
{
  {"% +o",	"",	(IntFunc) bnop_modeop,		"bop_modeop"},
  {"% -o",	"",	(IntFunc) bnop_modedeop,	"bop_modedeop"},
  {0,		0,	0,				0}
};

static cmd_t botnetop_join[] =
{
  {"*",	"",	(IntFunc) bnop_jointmr,	"bop_jointmr"},
  {0,	0,	0,				0}
};

static cmd_t botnetop_raw[] =
{
  {"302",	"",	(IntFunc) bnop_userhost,	"bop_userhost"},
  {0,		0,	0,				0}
};
