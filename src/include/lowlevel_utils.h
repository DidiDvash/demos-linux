/*
 * This file is part of the linuxapi package.
 * Copyright (C) 2011-2014 Mark Veltzer <mark.veltzer@gmail.com>
 *
 * linuxapi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * linuxapi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with linuxapi. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __lowlevel_utils_h
#define __lowlevel_utils_h

#include <firstinclude.h>
#include <unistd.h> // for getpagesize(2)
#include <err_utils.h> // for CHECK_ASSERT()
#include <cpufreq.h>	// for cpufreq_get_freq_kernel(2)

/*
 * This is a collection of helper function to help with working with low level stuff.
 * Stacks, registers, barriers, assembly snipplets and more.
 */

/* THIS IS A C FILE, NO C++ here */

/*
 * a small function to tell you if the local variables addresses
 * on the stack are growing or diminishing with order of declaration
 */
bool stack_vars_direction_up() __attribute__((noinline));
bool stack_vars_direction_up() {
	int a;
	int u;
	unsigned long pa=(unsigned long)&a;
	unsigned long pu=(unsigned long)&u;
	if(pa==pu+sizeof(int)) {
		return false;
	}
	if(pa==pu-sizeof(int)) {
		return true;
	}
	CHECK_ERROR("strange stack you have here");
}

/*
 * a small function to tell you in which direction your stack grows
 * with regard to function call.
 */
unsigned long stack_function_direction_up_inner() __attribute__((noinline));
unsigned long stack_function_direction_up_inner() {
	int a;
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wreturn-local-addr"
	return (unsigned long)&a;
	#pragma GCC diagnostic pop
}

bool stack_function_direction_up() __attribute__((noinline));
bool stack_function_direction_up() {
	int a;
	unsigned long pa=(unsigned long)&a;
	unsigned long adr=stack_function_direction_up_inner();
	if(adr>pa) {
		return true;
	}
	if(adr<pa) {
		return false;
	}
	CHECK_ERROR("strange stack you have here");
}

/*
 * align an address down to page boundry
 */
static inline unsigned long page_align(unsigned long adr) {
	return adr & ~((unsigned long)getpagesize()-1);
}

/*
 * align an address to stack (either up or down)
 */
static inline unsigned long stack_align(unsigned long adr) {
	if(stack_function_direction_up()) {
		return adr & ~((unsigned long)getpagesize()-1);
	} else {
		unsigned long mod=adr%getpagesize();
		if(mod==0) {
			return adr;
		} else {
			return adr-mod+getpagesize();
		}
	}
}
/*
 * align a pointer to the stack (either up or down)
 */
static inline void* stack_align_pointer(void* p) {
	return (void*)stack_align((unsigned long)p);
}

/*
 * get the RDTSC register (counter)
 * This functions seems to be wrong since rdtsc should be a 64 bit wide register, even on 32 bit systems.
 */
static inline unsigned int getrdtsc() {
	unsigned int val;
	asm ("rdtsc" : "=val" (val));
	return val;
}

/*
 * get the stack pointer register
 * on a 32 bit Intel machine this already works. I need to make it work on x64.
 */
static inline unsigned long getstackadr() {
#if __x86_64__
	return 0;
#else
	unsigned long val;
	asm ("movl %%esp, %0" : "=r" (val));
	return val;
#endif
}

/*
 * get the stack pointer as address
 */
static inline void* getstackpointer() {
	return (void*)getstackadr();
}

/*
 * get the current frame pointer
 * This currently works only on i386
 */
static inline unsigned long getframepointer() {
#if __x86_64__
	return 0;
#else
	unsigned long val;
	asm ("movl %%ebp, %0" : "=r" (val));
	return val;
#endif
}

/*
 * Functions which handle the TSC
 *
 * References:
 * http://en.wikipedia.org/wiki/Time_Stamp_Counter
 * http://stackoverflow.com/questions/3388134/rdtsc-accuracy-across-cpu-cores
 */

typedef unsigned long long ticks_t;

static inline ticks_t getticks(void) {
	unsigned int a, d;
	// the volatile here is necessary otherwise the compiler does not know that the value
	// of this register changes and caches it
	// the difference between 'rdtsc' and 'rdtscp' instruction is that the latter will not
	// get executed out of order and therefore measurements using it will be more
	// accurate
	// asm volatile ("rdtsc":"=a" (a), "=d" (d));
	asm volatile ("rdtscp" : "=a" (a), "=d" (d));
	return(((ticks_t)a) | (((ticks_t)d) << 32));
}

static inline unsigned int get_mic_diff(ticks_t t1, ticks_t t2) {
	CHECK_ASSERT(t2 >= t1);
	// this is in clicks
	unsigned long long diff=t2 - t1;
	// the frquency returned is in tousands of clicks per second so we multiply
	// unsigned long long freq=cpufreq_get_freq_kernel(0) * 1000;

	// we take the maxiumum frequency for newer Intel machines supporting the
	// 'constant_tsc' feature (see /proc/cpuinfo...).
	unsigned long min, max;
	cpufreq_get_hardware_limits(0, &min, &max);
	unsigned long long freq=max*1000;
	// cpufreq_get_freq_kernel can return 0
	// this is probably a problem with your cpu governor setup
	// this happens on certain ubuntu systems
	CHECK_ASSERT(freq!=0);
	// how many clicks per micro
	unsigned long long mpart=freq / 1000000;
	CHECK_ASSERT(mpart!=0);
	// how many micros have passed
	unsigned long long mdiff=diff / mpart;
	return(mdiff);
}

#endif	/* !__lowlevel_utils_h */
