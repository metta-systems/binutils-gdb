# This shell script emits a C file. -*- C -*-
# Generate the main loop of the simulator.
# Syntax: genmloop.sh mono|multi cpu mainloop.in
# FIXME: "multi" support is wip.

type=$1
cpu=$2
file=$3

cat <<EOF
/* This file is is generated by the genmloop script.  DO NOT EDIT! */

/* Main loop for CGEN-based simulators.
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

/* We want the simcache version of SEM_ARG.  */
#define SCACHE_P

#include "sim-main.h"
#include "bfd.h"
#include "cgen-mem.h"
#include "cgen-sem.h"
#include "cgen-scache.h"
#include "cpu-opc.h"
#include "cpu-sim.h"

/* Tell sim_main_loop to use the cache if it's active.
   Collecting profile data and tracing slow us down so we don't do them in
   "fast mode".
   There are 2 possibilities on 2 axes:
   - use or don't use the cache
   - run normally (full featured) or run fast
   Supporting all four possibilities in one executable is a bit much but
   supporting normal/fast seems reasonable.
   If the cache is configured in it is always used.
   ??? Need to see whether it speeds up profiling significantly or not.
   Speeding up tracing doesn't seem worth it.
   ??? Sometimes supporting more than one set of semantic functions will make
   the simulator too large - this should be configurable.
*/

#if WITH_SCACHE
#define RUN_FAST_P(cpu) (STATE_RUN_FAST_P (CPU_STATE (cpu)))
#else
#define RUN_FAST_P(cpu) 0
#endif

#ifndef SIM_PRE_EXEC_HOOK
#define SIM_PRE_EXEC_HOOK(state)
#endif

#ifndef SIM_POST_EXEC_HOOK
#define SIM_POST_EXEC_HOOK(state)
#endif

EOF

${SHELL} $file support

cat <<EOF

static volatile int keep_running;

int
engine_stop (SIM_DESC sd)
{
  keep_running = 0;
  return 1;
}

void
engine_run (SIM_DESC sd, int step, int siggnal)
{
  current_state = sd;
#if WITH_SCACHE
  if (USING_SCACHE_P (sd))
    scache_flush (sd);
#endif
  engine_resume (sd, step, siggnal);
}

void
engine_resume (SIM_DESC sd, int step, int siggnal)
{
#ifdef __STDC__
  /* These are volatile to survive setjmp/longjmp.
     This will slow down the simulation a teensy bit, but we want to
     measure simulator speed even in fast mode.  */
  volatile unsigned long insn_count;
  volatile SIM_ELAPSED_TIME start_time;
#else
  /* ??? Not sure what to do for K&R C.  */
  static unsigned long insn_count;
  static SIM_ELAPSED_TIME start_time;
#endif
  SIM_DESC current_state = sd;
  sim_cpu *current_cpu = STATE_CPU (sd, 0);

  keep_running = 1;
  start_time = sim_elapsed_time_get ();
  insn_count = 0;

  if (setjmp (STATE_HALT_JMP_BUF (sd)))
    {
      TRACE_INSN_FINI (current_cpu);
      PROFILE_EXEC_TIME (CPU_PROFILE_DATA (current_cpu))
	+= sim_elapsed_time_since (start_time);
      PROFILE_TOTAL_INSN_COUNT (CPU_PROFILE_DATA (current_cpu))
	+= insn_count;
      return;
    }

EOF

# Any initialization code before looping starts.
${SHELL} $file init

cat <<EOF

  /* ??? Restart support to be added in time.  */

  if (step
      || !RUN_FAST_P (current_cpu))
    {
      do
	{
#define FAST 0 /* ??? Hopefully this name won't collide with anything.  */
	  /* FIXME: Later check every insn for events and such.  */

	  SIM_PRE_EXEC_HOOK (current_cpu);

EOF

# Copy of main loop that uses the various compiled in features.
# FIXME: May want more than one copy of this.
${SHELL} $file normal

cat <<EOF

	  SIM_POST_EXEC_HOOK (current_cpu);

	  ++insn_count;
	  if (step)
	    engine_halt (current_cpu, EXEC_STATE_STOPPED, SIM_SIGTRAP);
	}
      while (keep_running);
#undef FAST
    }
  else
    {
      do
	{
#define FAST 1

EOF

# Copy of main loop that is run purely for fast execution.
${SHELL} $file fast

cat <<EOF

#undef FAST
	  ++insn_count;
        }
      while (keep_running);
    }
}
EOF
