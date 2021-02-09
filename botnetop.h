/* 
 * botnetop.h -- part of botnetop.mod
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

#ifndef _EGG_MOD_BOTNETOP_BOTNETOP_H
#define _EGG_MOD_BOTNETOP_BOTNETOP_H

#ifdef MAKING_BOTNETOP

/*
 * Language messages
 */

#define BOTNETOP_INVITED	"invited %s to %s"
#define BOTNETOP_INVITED2	"invited %s (using nick %s) to %s"
#define BOTNETOP_GAVEOPS	"gave ops to %s on %s"
#define BOTNETOP_GAVEOPS2	"gave ops to %s (using nick %s) on %s"
#define BOTNETOP_GAVEKEY	"gave key for %s to %s"
#define BOTNETOP_RAISEDLIMIT	"raised limit on %s as requested by %s"
#define BOTNETOP_UNBANNED	"unbanned %s on %s (removed %d bans)"
#define BOTNETOP_SETEXEMPT	"set exemption %s on %s for %s"
#define BOTNETOP_SETINVITE	"set invite %s on %s for %s"
#define BOTNETOP_REQKEY		"requested key for %s from %d bot%s"
#define BOTNETOP_REQINVITE	"requested invite to %s from %d bot%s"
#define BOTNETOP_REQLIMIT	"requested limit raise on %s from %d bot%s"
#define BOTNETOP_REQUNBAN	"requested unban on %s from %d bot%s"
#define BOTNETOP_REQOPS		"requested ops from %s on %s"
#define BOTNETOP_ADDHOST	"added host %s to %s"

/*
 * Misc definitions
 */

/* Size of the log entries. Getting a truncated log entry isn't
   that cruicial and 1024 should be enough for everybody anyway. */

#ifndef LOGSIZE
#  define LOGSIZE 1024+1
#endif

/* Define this if you want botnetop.mod to introduce
   few Tcl commands. */

#ifdef ENABLE_TCL_COMMANDS
#  undef ENABLE_TCL_COMMANDS
#endif

/*
 * Delay structure and functions (delay.c)
 */

struct delay_t {
  struct delay_t	*next;		/* Next delay record 				*/
  char 			*chan;		/* Name of the channel to request ops for 	*/
  char			*handle;	/* Handle of the bot to request ops from 	*/
  int			reqs;		/* Amount of op requests 			*/
  time_t		reqtime;	/* Time when to ask ops 			*/
  time_t		resptime;	/* Time when to reset the amount of op reqs 	*/
};
static struct delay_t *find_delay(char *, char *);
static struct delay_t *add_delay(char *, char *);
static void del_delay(struct delay_t *);
static void check_delay();
static void delay_free_mem();
static int delay_expmem();
static int check_delay_status(char *, char *);

/*
 * Who structure and functions (who.c)
 */

struct who_t {
  struct who_t	*next;		/* Next who record 				*/
  char		*chan;		/* Name of the channel bot asked for invite 	*/
  char		*nick;		/* Nick of the bot 				*/
  char		*handle;	/* Handle of the bot 				*/
  char		*uhost;		/* Userhost of the bot 				*/
  time_t	time;		/* Time when to delete the who record 		*/
};
static struct who_t *find_who(char *);
static struct who_t *add_who(char *, char *, char *, char *);
static void del_who(struct who_t *);
static void check_who();
static void who_free_mem();
static int who_expmem();

/*
 * Flood structure and functions (flood.c)
 */

struct flood_t {
  struct flood_t	*next;		/* Next flood record 				*/
  char 			*chan;		/* Name of the channel 				*/
  int			amount;		/* Amount of requests	 			*/
  time_t		time;		/* Time when the counter should be decreased	*/
  time_t		ignore;		/* Time when the ignore should be removed	*/
};
static struct flood_t *find_flood(char *);
static struct flood_t *add_flood(char *);
static void del_flood(struct flood_t *);
static void check_flood();
static void flood_free_mem();
static int flood_expmem();
static int check_flood_status(char *);

/*
 * Request structure and functions (request.c)
 */

struct request_t {
  struct request_t	*next;		/* Next request record 	*/
  char			*handle;	/* Handle of the bot 	*/
  char			*chan;		/* Name of the channel	*/
};
static struct request_t *find_request(char *, char *);
static struct request_t *add_request(char *, char *);
static void del_request(struct request_t *);
static void request_free_mem();
static int request_expmem();
static int check_request_status(char *, char *);

/*
 * Functions in misc.c
 */

static int matchattr(struct userrec *, char *, char *);
static int isop(char *, struct chanset_t *);
static int handisop(char *, struct chanset_t *);
static int lowbots(struct chanset_t *);

/*
 * Functions in botcmds.c
 */

static int bnop_reqop(char *, char *);
static void bnop_askbot(char *, char *);
static void bnop_invite(struct chanset_t *, char *, char *, char *);
static void bnop_letmein(struct chanset_t *, char *);

/*
 * Macros
 */

#ifndef strncpyz
#  define strncpyz(target, source, len) do {      \
          strncpy((target), (source), (len) - 1); \
          (target)[(len) - 1] = 0;                \
          } while (0)
#endif  /* strncpyz */

/* If the compat functions aren't defined
   we just assume that the system has them. */

#ifndef egg_strcasecmp
#  define egg_strcasecmp strcasecmp
#endif /* egg_strcasecmp */

#ifndef egg_snprintf
#  define egg_snprintf snprintf
#endif /* egg_snprintf */

#endif  /* MAKING_BOTNETOP */

#endif  /* _EGG_MOD_BOTNETOP_BOTNETOP_H */
