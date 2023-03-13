/* 
 * who.c -- part of botnetop.mod
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

static struct who_t *find_who(char *nick)
{
  struct who_t *w = NULL;

  for (w = who_start; w != NULL; w = w->next) {
    if (!rfc_casecmp(w->nick, nick))
      return w;
  }

  return NULL;
}

static struct who_t *add_who(char *nick, char *chan, char *handle, char *uhost)
{
  struct who_t *w = NULL;

  w = (struct who_t *) nmalloc(sizeof(struct who_t));
  w->nick = nmalloc(strlen(nick) + 1);
  w->chan = nmalloc(strlen(chan) + 1);
  w->handle = nmalloc(strlen(handle) + 1);
  w->uhost = nmalloc(strlen(uhost) + 1);
  strcpy(w->chan, chan);
  strcpy(w->nick, nick);
  strcpy(w->handle, handle);
  strcpy(w->uhost, uhost);
  w->time = now;

  w->next = who_start;
  who_start = w;

  putlog(LOG_DEBUG, "*", "botnetop.mod: new who record created for %s on %s (address: %" PRIuPTR ")", w->nick, w->chan, (uintptr_t) w);

  return w;
}

static void del_who(struct who_t *who)
{
  struct who_t *w = NULL, *old = NULL;

  for (w = who_start; w != NULL; old = w, w = w->next) {
    if (w == who) {
      if (old != NULL)
        old->next = w->next;
      else
        who_start = w->next;
      putlog(LOG_DEBUG, "*", "botnetop.mod: who record removed from %s on %s (address: %" PRIuPTR ")", w->nick, w->chan, (uintptr_t) w);
      if (w->chan != NULL)
        nfree(w->chan);
      if (w->nick != NULL)
        nfree(w->nick);
      if (w->handle != NULL)
        nfree(w->handle);
      if (w->uhost)
        nfree(w->uhost);
      nfree(w);
      break;
    }
  }
}

static void check_who()
{
  struct who_t *w = NULL, *wnext = NULL;

  for (w = who_start; w != NULL; w = wnext) {
    wnext = w->next;
    if ((w->time > 0) && (w->time < now - 300))
      del_who(w);
  }
}

static void who_free_mem()
{
  struct who_t *w = NULL, *wnext = NULL;

  for (w = who_start; w != NULL; w = wnext) {
    wnext = w->next;
    if (w->chan != NULL)
      nfree(w->chan);
    if (w->nick != NULL)
      nfree(w->nick);
    if (w->handle != NULL)
      nfree(w->handle);
    if (w->uhost != NULL)
      nfree(w->uhost);
    nfree(w);
  }
  who_start = NULL;
}

static int who_expmem()
{
  int size = 0;
  struct who_t *w = NULL;

  for (w = who_start; w != NULL; w = w->next) {
    if (w->chan != NULL)
      size += strlen(w->chan) + 1;
    if (w->nick != NULL)
      size += strlen(w->nick) + 1;
    if (w->handle != NULL)
      size += strlen(w->handle) + 1;
    if (w->uhost != NULL)
      size += strlen(w->uhost) + 1;
    size += sizeof(struct who_t);
  }

  return size;
}
