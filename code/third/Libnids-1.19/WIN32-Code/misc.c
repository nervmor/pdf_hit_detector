/* -/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
 . Copyright (c) 2001 Michael Davis <mike@datanerds.net>
 . All rights reserved.
 .
 . Redistribution and use in source and binary forms, with or without
 . modification, are permitted provided that the following conditions
 . are met:
 .
 . 1. Redistributions of source code must retain the above copyright
 .    notice, this list of conditions and the following disclaimer.
 .
 . 2. Redistributions in binary form must reproduce the above copyright
 .    notice, this list of conditions and the following disclaimer in the
 .    documentation and/or other materials provided with the distribution.
 .
 . 3. The name of author may not be used to endorse or promote products
 .    derived from this software without specific prior written permission.
 .
 . THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 . INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 . AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 . THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 . EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 . PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 . OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 . WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 . OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 . ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 . -\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\ */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock.h>
#include <sys/timeb.h>
#include <time.h>

int gettimeofday(struct timeval *, struct timezone *);
void *GetAdapterFromList(void *,int);

/****************************************************************************
 *
 * Function: gettimeofday(struct timeval *, struct timezone *)
 *
 * Purpose:  Get current time of day.
 *
 * Arguments: tv => Place to store the curent time of day.
 *            tz => Ignored.
 *
 * Returns: 0 => Success.
 *
 ****************************************************************************/

int gettimeofday(struct timeval *tv, struct timezone *tz) {
    struct _timeb tb;

	if(tv==NULL) return -1;
	_ftime(&tb);
	tv->tv_sec = tb.time;
	tv->tv_usec = ((int) tb.millitm) * 1000;
	return 0;
}

/****************************************************************************
 *
 * Function: GetAdapterFromList(void *,int)
 *
 * Purpose:  Get a specific adapter from the list of adapters on the system.
 *
 * Arguments: device => Device to look for.
 *            index => Adapter number.
 *
 * Returns: Adapter if device was valid.
 *
 * Comments: Shamelessly ripped from WinDump.
 *
 ****************************************************************************/

void *GetAdapterFromList(void *device,int index) {
        DWORD dwVersion;
        DWORD dwWindowsMajorVersion;
        char *Adapter95;
        WCHAR *Adapter;
        int i;

        dwVersion = GetVersion();
        dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
                /* Windows 95. */
        if (dwVersion >= 0x80000000 && dwWindowsMajorVersion >= 4)
        {
                Adapter95 = device;
                for(i = 0;i < index - 1; i++){
                        while(*Adapter95++ != 0);
                        if(*Adapter95 == 0)
							return NULL;
                }
                return  Adapter95;
        }
        else{
				/* NT. */
                Adapter=(WCHAR *)device;
                for(i = 0;i < index - 1; i++){
                        while(*Adapter++ != 0);
                        if(*Adapter == 0)return NULL;
                }
                return  Adapter;
        }

}
