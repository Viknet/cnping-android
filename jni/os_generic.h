#ifndef _OS_GENERIC_H
#define _OS_GENERIC_H

extern "C" {
   //Things that shouldn't be macro'd
   double OGGetAbsoluteTime();
   void OGSleep( int is );
   void OGUSleep( int ius );
   double OGGetFileTime( const char * file );

   //Threads and Mutices
   typedef void* og_thread_t;
   typedef void* og_mutex_t;
   typedef void* og_sema_t;

   og_thread_t OGCreateThread( void * (routine)( void * ), void * parameter );
   void * OGJoinThread( og_thread_t ot );
};
#endif
//Date Stamp: 2012-02-15

/*
   NOTE: Portions (namely the top section) are part of headers from other
   sources.
   Copyright (c) 2011-2012 <>< Charles Lohr
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of this file.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/
