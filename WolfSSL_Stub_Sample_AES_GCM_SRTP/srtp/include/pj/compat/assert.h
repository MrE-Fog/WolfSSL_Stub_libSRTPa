/* $Id: assert.h 3553 2011-05-05 06:14:19Z nanang $ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#ifndef __PJ_COMPAT_ASSERT_H__
#define __PJ_COMPAT_ASSERT_H__

/**
 * @file assert.h
 * @brief Provides assert() macro.
 */

#if defined(PJ_HAS_ASSERT_H) && PJ_HAS_ASSERT_H != 0
#  include <assert.h>

#elif defined(PJ_LINUX_KERNEL) && PJ_LINUX_KERNEL != 0
#  define assert(expr) do { \
			if (!(expr)) \
			  printk("!!ASSERTION FAILED: [%s:%d] \"" #expr "\"\n",\
				 __FILE__, __LINE__); \
		       } while (0)
#elif (defined(PJ_RTKE) && PJ_RTKE != 0) || (defined(PJ_THREADX) && PJ_THREADX!= 0)
#	include <_kn_plt_cstd_lib_wrap.h>
#	define assert	KN_ASSERT
#elif defined(POC_CDE_BREW)
#	define assert	KN_ASSERT
#else
#  warning "assert() is not implemented"
#  define assert(expr)
#endif

#endif	/* __PJ_COMPAT_ASSERT_H__ */

