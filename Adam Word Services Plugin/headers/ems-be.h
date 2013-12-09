#ifndef __EMS_BE__
#define __EMS_BE__

 
 // Be definitions
#ifndef FAR
#define FAR
#endif

#ifndef LPSTR
typedef char * LPSTR;
#endif

#ifndef HWND
typedef BWindow *HWND;
#endif

#ifndef HICON
typedef char *HICON;
#endif

#ifndef WINAPI
#define	WINAPI
#endif

#ifndef	UINT
#define UINT	unsigned int
#endif

#pragma export on
#include "ems-win.h"
#pragma export reset

#endif /* __EMS_BE__ */
