/*
 *  ircd-hybrid: an advanced, lightweight Internet Relay Chat Daemon (ircd)
 *
 *  Copyright (c) 1997-2017 ircd-hybrid development team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 *  USA
 */

/*! \file m_motd.c
 * \brief Includes required functions for processing the MOTD command.
 * \version $Id$
 */

#include "stdinc.h"
#include "list.h"
#include "client.h"
#include "motd.h"
#include "ircd.h"
#include "send.h"
#include "numeric.h"
#include "server.h"
#include "parse.h"
#include "modules.h"
#include "conf.h"


/*! \brief Sends the "message of the day" and notifies irc-operators
 *         about the MOTD request
 *
 * \param source_p Pointer to client to report to
 */
static void
do_motd(struct Client *source_p)
{
  sendto_realops_flags(UMODE_SPY, L_ALL, SEND_NOTICE,
                       "MOTD requested by %s (%s@%s) [%s]",
                       source_p->name, source_p->username,
                       source_p->host, source_p->servptr->name);
  motd_send(source_p);
}

/*! \brief MOTD command handler
 *
 * \param source_p Pointer to allocated Client struct from which the message
 *                 originally comes from.  This can be a local or remote client.
 * \param parc     Integer holding the number of supplied arguments.
 * \param parv     Argument vector where parv[0] .. parv[parc-1] are non-NULL
 *                 pointers.
 * \note Valid arguments for this command are:
 *      - parv[0] = command
 *      - parv[1] = nickname/servername
 */
static int
m_motd(struct Client *source_p, int parc, char *parv[])
{
  static uintmax_t last_used = 0;

  if ((last_used + ConfigGeneral.pace_wait) > CurrentTime)
  {
    sendto_one_numeric(source_p, &me, RPL_LOAD2HI, "MOTD");
    return 0;
  }

  last_used = CurrentTime;

  if (!ConfigServerHide.disable_remote_commands)
    if (server_hunt(source_p, ":%s MOTD :%s", 1, parc, parv)->ret != HUNTED_ISME)
      return 0;

  do_motd(source_p);
  return 0;
}

/*! \brief MOTD command handler
 *
 * \param source_p Pointer to allocated Client struct from which the message
 *                 originally comes from.  This can be a local or remote client.
 * \param parc     Integer holding the number of supplied arguments.
 * \param parv     Argument vector where parv[0] .. parv[parc-1] are non-NULL
 *                 pointers.
 * \note Valid arguments for this command are:
 *      - parv[0] = command
 *      - parv[1] = nickname/servername
 */
static int
ms_motd(struct Client *source_p, int parc, char *parv[])
{
  if (server_hunt(source_p, ":%s MOTD :%s", 1, parc, parv)->ret != HUNTED_ISME)
    return 0;

  do_motd(source_p);
  return 0;
}

static struct Message motd_msgtab =
{
  .cmd = "MOTD",
  .args_max = MAXPARA,
  .handlers[UNREGISTERED_HANDLER] = m_unregistered,
  .handlers[CLIENT_HANDLER] = m_motd,
  .handlers[SERVER_HANDLER] = ms_motd,
  .handlers[ENCAP_HANDLER] = m_ignore,
  .handlers[OPER_HANDLER] = ms_motd
};

static void
module_init(void)
{
  mod_add_cmd(&motd_msgtab);
}

static void
module_exit(void)
{
  mod_del_cmd(&motd_msgtab);
}

struct module module_entry =
{
  .version = "$Revision$",
  .modinit = module_init,
  .modexit = module_exit,
};
