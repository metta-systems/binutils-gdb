/* Module support.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   Contributed by Cygnus Support.

This file is part of GDB, the GNU debugger.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "sim-main.h"
#include "sim-io.h"
#include "sim-options.h"

/* List of all modules.  */
static MODULE_INSTALL_FN * const modules[] = {
  standard_install,
  trace_install,
  profile_install,
  sim_core_install,
#if WITH_SCACHE
  scache_install,
#endif
#ifdef SIM_HAVE_MODEL /* FIXME: temporary */
  model_install,
#endif
  /* Configured in [simulator specific] additional modules.  */
#ifdef MODULE_LIST
  MODULE_LIST
#endif
  0
};

/* Functions called from sim_open.  */

/* Initialize common parts before argument processing.  */

SIM_RC
sim_pre_argv_init (SIM_DESC sd, const char *myname)
{
  STATE_MY_NAME (sd) = myname + strlen (myname);
  while (STATE_MY_NAME (sd) > myname && STATE_MY_NAME (sd)[-1] != '/')
    --STATE_MY_NAME (sd);

  /* Install all configured in modules.  */
  if (sim_module_install (sd) != SIM_RC_OK)
    return SIM_RC_FAIL;

  return SIM_RC_OK;
}

/* Initialize common parts after argument processing.  */

SIM_RC
sim_post_argv_init (SIM_DESC sd)
{
  int i;

  if (sim_module_init (sd) != SIM_RC_OK)
    return SIM_RC_FAIL;

  /* Set the cpu->state backlinks for each cpu.  */
  for (i = 0; i < MAX_NR_PROCESSORS; ++i)
    CPU_STATE (STATE_CPU (sd, i)) = sd;

  return SIM_RC_OK;
}

/* Install all modules.  */

SIM_RC
sim_module_install (SIM_DESC sd)
{
  MODULE_INSTALL_FN * const *modp;

  for (modp = modules; *modp != NULL; ++modp)
    {
      if ((*modp) (sd) != SIM_RC_OK)
	return SIM_RC_FAIL;
    }
  return SIM_RC_OK;
}

/* Called after all modules have been installed and after argv
   has been processed.  */

SIM_RC
sim_module_init (SIM_DESC sd)
{
  MODULE_INIT_LIST *modp;

  for (modp = STATE_INIT_LIST (sd); modp != NULL; modp = modp->next)
    {
      if ((*modp->fn) (sd) != SIM_RC_OK)
	return SIM_RC_FAIL;
    }
  return SIM_RC_OK;
}

/* Uninstall installed modules, called by sim_close.  */

void
sim_module_uninstall (SIM_DESC sd)
{
  MODULE_UNINSTALL_LIST *modp;

  /* Uninstall the modules.  */
  for (modp = STATE_UNINSTALL_LIST (sd); modp != NULL; modp = modp->next)
    (*modp->fn) (sd);
}

/* Add FN to the init handler list.  */

void
sim_module_add_init_fn (SIM_DESC sd, MODULE_INIT_FN fn)
{
  MODULE_INIT_LIST *l =
    (MODULE_INIT_LIST *) xmalloc (sizeof (MODULE_INIT_LIST));

  l->fn = fn;
  l->next = STATE_INIT_LIST (sd);
  STATE_INIT_LIST (sd) = l;
}

/* Add FN to the uninstall handler list.  */

void
sim_module_add_uninstall_fn (SIM_DESC sd, MODULE_UNINSTALL_FN fn)
{
  MODULE_UNINSTALL_LIST *l =
    (MODULE_UNINSTALL_LIST *) xmalloc (sizeof (MODULE_UNINSTALL_LIST));

  l->fn = fn;
  l->next = STATE_UNINSTALL_LIST (sd);
  STATE_UNINSTALL_LIST (sd) = l;
}
