/* 
 * flood.c -- part of botnetop.mod
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

static struct flood_t *find_flood(char *chan)
{
  struct flood_t *f = NULL;

  for (f = flood_start; f != NULL; f = f->next) {
    if (!rfc_casecmp(f->chan, chan))
      return f;
  }

  return NULL;
}

static struct flood_t *add_flood(char *chan)
{
  struct flood_t *f = NULL;

  f = (struct flood_t *) nmalloc(sizeof(struct flood_t));
  f->chan = nmalloc(strlen(chan) + 1);
  strcpy(f->chan, chan);
  f->time = now;
  f->amount = 0;
  f->ignore = 0;

  f->next = flood_start;
  flood_start = f;

  putlog(LOG_DEBUG, "*", "botnetop.mod: new flood record created for %s (address: %" PRIuPTR ")", f->chan, (uintptr_t) f);

  return f;
}

static void del_flood(struct flood_t *flood)
{
  struct flood_t *f = NULL, *old = NULL;

  for (f = flood_start; f != NULL; old = f, f = f->next) {
    if (f == flood) {
      if (old != NULL)
        old->next = f->next;
      else
        flood_start = f->next;
      putlog(LOG_DEBUG, "*", "botnetop.mod: flood record removed from %s (address: %" PRIuPTR ")", f->chan, (uintptr_t) f);
      if (f->chan != NULL)
        nfree(f->chan);
      nfree(f);
      break;
    }
  }
}

static int check_flood_status(char *chan)
{
  struct flood_t *f = NULL;

  if (bop_flood_amount) {
    if (!(f = find_flood(chan))) {
      if (!(f = add_flood(chan)))
        return 0;
    }
    
    if (f->ignore)
      return 1;  /* We're currently ignoring. */

    if (f->time < now - bop_flood_time) { 
      /* Flood timer has expired, reset variables. */
      f->time = now;
      f->amount = 0;
      return 0;
    }

    f->amount++;

    if (f->amount >= bop_flood_amount) {
      /* We got flooded, start ignoring. */
      f->amount = 0;
      f->time = 0;
      f->ignore = now;
      putlog(LOG_DEBUG, "*", "botnetop.mod: added %ds ignore for %s", bop_ignore, f->chan);
      return 1;
    }
  }

  return 0;
}

static void check_flood() 
{
  struct flood_t *f = NULL;
  
  for (f = flood_start; f != NULL; f = f->next) {
    if ((f->ignore > 0) && (f->ignore < now - bop_ignore)) {
      f->ignore = 0;
      putlog(LOG_DEBUG, "*", "botnetop.mod: removed ignore from %s", f->chan);
    }
  }
}

static void flood_free_mem()
{
  struct flood_t *f = NULL, *fnext = NULL;

  for (f = flood_start; f != NULL; f = fnext) {
    fnext = f->next;
    if (f->chan != NULL)
      nfree(f->chan);
    nfree(f);
  }
  flood_start = NULL;
}

static int flood_expmem()
{
  int size = 0;
  struct flood_t *f = NULL;

  for (f = flood_start; f != NULL; f = f->next) {
    if (f->chan != NULL)
      size += strlen(f->chan) + 1;
    size += sizeof(struct flood_t);
  }

  return size;
}

