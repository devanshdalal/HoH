#pragma once


#include "labs/shell.h"
#include "labs/preempt.h"
#include "devices/lapic.h"
#include "util/fiber.h"

void shell_step_fiber_schedular(shellstate_t& shellstate, addr_t& main_stack , preempt_t& preempt , addr_t stackptrs[], size_t stackptrs_size, addr_t arrays, size_t arrays_size , dev_lapic_t& lapic );


