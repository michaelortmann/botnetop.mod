/* 
 * delay.c -- part of botnetop.mod
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

static struct delay_t *find_delay(char *chan, char *handle)
{
  struct delay_t *d = NULL;

  for (d = delay_start; d != NULL; d = d->next) {
    if (!rfc_casecmp(d->chan, chan) && !egg_strcasecmp(d->handle, handle))
      return d;
  }

  return NULL;
}

static struct delay_t *add_delay(char *chan, char *handle)
{
  struct delay_t *d = NULL;

  d = (struct delay_t *) nmalloc(sizeof(struct delay_t));
  if (d == NULL)
    return NULL;

  d->chan = (char *) nmalloc(strlen(chan) + 1);
  if (d->chan == NULL) {
    nfree(d);
    return NULL;
  }

  d->handle = (char *) nmalloc(strlen(handle) + 1);
  if (d->handle == NULL) {
    nfree(d->chan);
    nfree(d);
    return NULL;
  }

  strncpyz(d->chan, chan, strlen(chan) + 1);
  strncpyz(d->handle, handle, strlen(handle) + 1);
  d->reqs = 0;
  d->reqtime = 0;
  d->resptime = now;

  d->next = delay_start;
  delay_start = d;

  putlog(LOG_DEBUG, "*", "botnetop.mod: new delay record created for %s on %s (address: %u)", d->handle, d->chan, d);

  return d;
}

static void del_delay(struct delay_t *delay)
{
  struct delay_t *d = NULL, *old = NULL;

  for (d = delay_start; d != NULL; old = d, d = d->next) {
    if (d == delay) {
      if (old != NULL)
        old->next = d->next;
      else
        delay_start = d->next;
      putlog(LOG_DEBUG, "*", "botnetop.mod: delay record removed from %s on %s (address: %u)", d->handle, d->chan, d);
      if (d->chan != NULL)
        nfree(d->chan);
      if (d->handle != NULL)
        nfree(d->handle);
      nfree(d);
      break;
    }
  }
}

static int check_delay_status(char *chan, char *handle)
{
  struct delay_t *d = NULL;

  if (bop_maxreq) {

    if (!(d = find_delay(chan, handle))) {
      if (!(d = add_delay(chan, handle)))
        return 0;
    }

    if (d->resptime < now - 30) {
      d->resptime = now;
      d->reqs = 0;
      return 0;
    }

    d->reqs++;

    if (d->reqs >= bop_maxreq) {
      d->resptime = 0;
      d->reqs = 0;
      return 1;
    }
  }

  return 0;
}

static void check_delay()
{
  struct delay_t *d = NULL;

  for (d = delay_start; d != NULL; d = d->next) {
    if ((d->reqtime > 0) && (d->reqtime < now)) {
      d->reqtime = 0;
      bnop_askbot(d->handle, d->chan);
    }
  }
}

static void delay_free_mem()
{
  struct delay_t *d = NULL, *dnext = NULL;

  for (d = delay_start; d != NULL; d = dnext) {
    dnext = d->next;
    if (d->chan != NULL)
      nfree(d->chan);
    if (d->handle != NULL)
      nfree(d->handle);
    nfree(d);
  }
  delay_start = NULL;
}

static int delay_expmem()
{
  int size = 0;
  struct delay_t *d = NULL;

  for (d = delay_start; d != NULL; d = d->next) {
    if (d->chan != NULL)
      size += strlen(d->chan) + 1;
    if (d->handle != NULL)
      size += strlen(d->handle) + 1;
    size += sizeof(struct delay_t);
  }

  return size;
}
