/* 
 * request.c -- part of botnetop.mod
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

static struct request_t *find_request(char *handle, char *chan)
{
  struct request_t *r = NULL;

  for (r = request_start; r != NULL; r = r->next) {
    if (!egg_strcasecmp(r->handle, handle) && !rfc_casecmp(r->chan, chan))
      return r;
  }

  return NULL;
}

static struct request_t *add_request(char *handle, char *chan)
{
  struct request_t *r = NULL;

  r = (struct request_t *) nmalloc(sizeof(struct request_t));
  if (r == NULL)
    return NULL;

  r->handle = (char *) nmalloc(strlen(handle) + 1);
  if (r->handle == NULL) {
    nfree(r);
    return NULL;
  }

  r->chan = (char *) nmalloc(strlen(chan) + 1);
  if (r->chan == NULL) {
    nfree(r->handle);
    nfree(r);
    return NULL;
  }

  strncpyz(r->handle, handle, strlen(handle) + 1);
  strncpyz(r->chan, chan, strlen(chan) + 1);

  r->next = request_start;
  request_start = r;

  putlog(LOG_DEBUG, "*", "botnetop.mod: new request record created for %s on %s (address: %" PRIuPTR ")", r->handle, r->chan, (uintptr_t) r);

  return r;
}

static void del_request(struct request_t *request)
{
  struct request_t *r = NULL, *old = NULL;

  for (r = request_start; r != NULL; old = r, r = r->next) {
    if (r == request) {
      if (old != NULL)
        old->next = r->next;
      else
        request_start = r->next;
      putlog(LOG_DEBUG, "*", "botnetop.mod: request record removed from %s on %s (address: %" PRIuPTR ")", r->handle, r->chan, (uintptr_t) r);
      if (r->handle != NULL)
        nfree(r->handle);
      if (r->chan != NULL)
        nfree(r->chan);
      nfree(r);
      break;
    }
  }
}

static int check_request_status(char *handle, char *chan)
{
  if (!find_request(handle, chan)) {
    (void) add_request(handle, chan);
    return 0;
  } else
    return 1;
}

static void request_free_mem()
{
  struct request_t *r = NULL, *rnext = NULL;

  for (r = request_start; r != NULL; r = rnext) {
    rnext = r->next;
    if (r->handle != NULL)
      nfree(r->handle);
    if (r->chan != NULL)
      nfree(r->chan);
    nfree(r);
  }
  request_start = NULL;
}

static int request_expmem()
{
  int size = 0;
  struct request_t *r = NULL;

  for (r = request_start; r != NULL; r = r->next) {
    if (r->handle != NULL)
      size += strlen(r->handle) + 1;
    if (r->chan != NULL)
      size += strlen(r->chan) + 1;
    size += sizeof(struct request_t);
  }

  return size;
}
