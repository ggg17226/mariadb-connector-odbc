/************************************************************************************
   Copyright (C) 2013 SkySQL AB
   
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
#include <ma_odbc.h>

void ma_debug_print(my_bool ident, char *format, ...)
{
  FILE *fp= fopen("C:\\MAODBC.LOG", "a");
  if (fp)
  {
    va_list va;
    va_start(va, format);
    if (ident)
      fprintf(fp, "\t");
    vfprintf(fp, format, va);
    fprintf(fp, "\n");
    va_end(va);
    fclose(fp);
  }
}

void ma_debug_printw(wchar_t *format, ...)
{
  FILE *fp= fopen("C:\\MAODBC.LOG", "a");
  if (fp)
  {
    va_list va;
    va_start(va, format);
    fwprintf(fp, format, va);
    fwprintf(fp, L"\n");
    va_end(va);
    fclose(fp);
  }
}

void ma_debug_printv(char *format, va_list args)
{
  FILE *fp= fopen("C:\\MAODBC.LOG", "a");
  if (fp)
  {
    vfprintf(fp, format, args);
    fclose(fp);
  }
}