/************************************************************************************
   Copyright (C) 2014 SkySQL AB
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc., 
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/


/* MariaDB ODBC driver platform dependent declarations */

/* NOTE If you change something in this program, please consider if other platform's declaration
        require similar change */


/* Only one "platform" header is supposed to be included */
#ifndef _ma_platform_x_h_
# define _ma_platform_x_h_

/* Need size_t */
#include <stdio.h>

#define CRITICAL_SECTION pthread_mutex_t
typedef char * CODEPAGE;
#define CP_UTF8                   65001

/* Mimicking of VS' _snprintf */
int _snprintf(char *buffer, size_t count, const char *format, ...);

#endif /*_ma_platform_x_h_ */
