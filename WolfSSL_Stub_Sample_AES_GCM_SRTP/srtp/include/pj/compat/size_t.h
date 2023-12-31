/* $Id: size_t.h 3553 2011-05-05 06:14:19Z nanang $ */
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
#ifndef __PJ_COMPAT_SIZE_T_H__
#define __PJ_COMPAT_SIZE_T_H__

/**
 * @file size_t.h
 * @brief Provides size_t type.
 */
#if PJ_HAS_STDDEF_H
# include <stddef.h>
#else
//#ifndef POC_CDE_BREW
# include <_kn_plt_cstd_lib_wrap.h>
/* Merging from 7.10 branch to 7.10 64-bit branch */
/* KSantosh - KN_SIZE_T is mapped to __KN_SIZE_T which is mapped to size_t. Below size_t is mappe backed to KN_SIZE_T. 
     KN_SIZE_T is replaed by KN_SIZE_T itself. Hence resulting in compilation error in */
//#define size_t	KN_SIZE_T
//#endif
//#ifdef POC_CDE_BREW
//#define size_t	unsigned int
//#endif
#endif

#endif	/* __PJ_COMPAT_SIZE_T_H__ */

