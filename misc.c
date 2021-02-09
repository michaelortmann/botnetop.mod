/*
 * misc.c -- part of botnetop.mod
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

static int matchattr(struct userrec *u, char *flags, char *chan) 
{
  struct flag_record plus, minus, user;
  int ok = 0, f;

  if (u && (!chan || findchan_by_dname(chan))) {
    user.match = FR_GLOBAL | (chan ? FR_CHAN : 0) | FR_BOT;
    get_user_flagrec(u, &user, chan);
    plus.match = user.match;
    break_down_flags(flags, &plus, &minus);
    f = (minus.global || minus.udef_global || minus.chan ||
         minus.udef_chan || minus.bot);
    if (flagrec_eq(&plus, &user)) {
      if (!f)
        ok = 1;
      else {
        minus.match = plus.match ^ (FR_AND | FR_OR);
        if (!flagrec_eq(&minus, &user))
          ok = 1;
      }
    }
  }

  return ok;
}

static int isop(char *nick, struct chanset_t *chan)
{
  memberlist *m = NULL;

  if ((m = ismember(chan, nick)) && chan_hasop(m))
    return 1;

  return 0;
}

static int handisop(char *handle, struct chanset_t *chan)
{
  char s[UHOSTLEN];
  struct userrec *u = NULL;
  memberlist *m = NULL;

  for (m = chan->channel.member; m && m->nick[0]; m = m->next) {
    egg_snprintf(s, sizeof s, "%s!%s", m->nick, m->userhost);
    if ((u = get_user_by_host(s)) && !egg_strcasecmp(u->handle, handle) && isop(m->nick, chan))
        return 1;
  }

  return 0;
}

static int lowbots(struct chanset_t *chan)
{
  int bots = 1;
  char s[UHOSTLEN];
  struct userrec *u = NULL;
  memberlist *m = NULL;

  if (!bop_lowbotslimit)
   return 0; /* Let's speed up things and return 0 right
                away if bop_lowbotslimit is set to 0. */

  for (m = chan->channel.member; m && m->nick[0]; m = m->next) {
    if (rfc_casecmp(m->nick, botname) && chan_hasop(m)) {
      egg_snprintf(s, sizeof s, "%s!%s", m->nick, m->userhost);
      if ((u = get_user_by_host(s)) && matchattr(u, "b|-", chan->dname))
        bots++;
    }
  }
  if (bots < bop_lowbotslimit)
    return 1;

  return 0;
}
