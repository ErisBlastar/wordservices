//--------------------------------------------------------------------
//	
//	Utilities.h
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once

#ifndef UTILITIES_H
#define UTILITIES_H
#include <SupportDefs.h>


//====================================================================

#ifdef __cplusplus
extern "C" {
#endif

int32	cistrncmp(char*, char*, int32);
char*	cistrstr(char*, char*);
char*	jis_to_sjis(char*, int32*);
void	extract(char**, char*);
void	get_recipients(char**, char*, int32, bool);
void	verify_recipients(char**);
int32	linelen(char*, int32, bool);
bool	get_parameter(char*, char*, char*);
char*	find_boundary(char*, char*, int32);

#ifdef __cplusplus
}
#endif

#endif
