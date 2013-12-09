//--------------------------------------------------------------------
//	
//	Header.h
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once
#ifndef HEADER_H
#define HEADER_H

#include <Box.h>
#include <fs_attr.h>
#include <Entry.h>
#include <File.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <Point.h>
#include <PopUpMenu.h>
#include <Query.h>
#include <Rect.h>
#include <TextControl.h>
#include <TextView.h>
#include <View.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <Window.h>
// Word Services Begin
#include "WSTextControl.h"
// Word Services End

#define HEADER_HEIGHT		 82
#define MIN_HEADER_HEIGHT	(HEADER_HEIGHT - 8)//26)

#define TO_TEXT				"To:"
#define FROM_TEXT			"From:"
#define TO_FIELD_H			 39
#define FROM_FIELD_H		 31
#define TO_FIELD_V			  8
#define TO_FIELD_WIDTH		270
#define FROM_FIELD_WIDTH	280
#define TO_FIELD_HEIGHT		 16

#define SUBJECT_TEXT		"Subject:"
#define SUBJECT_FIELD_H		 18
#define SUBJECT_FIELD_V		 33
#define SUBJECT_FIELD_WIDTH	270
#define SUBJECT_FIELD_HEIGHT 16

#define CC_TEXT				"Cc:"
#define CC_FIELD_H			 40
#define CC_FIELD_V			 58
#define CC_FIELD_WIDTH		192
#define CC_FIELD_HEIGHT		 16

#define BCC_TEXT			"Bcc:"
#define BCC_FIELD_H			252
#define BCC_FIELD_V			 58
#define BCC_FIELD_WIDTH		197
#define BCC_FIELD_HEIGHT	 16

class	TTextControl;


//====================================================================

class THeaderView : public BBox {

private:

bool			fIncoming;
bool			fResending;
BPopUpMenu		*fBccMenu;
BPopUpMenu		*fCcMenu;
BPopUpMenu		*fToMenu;
BFile			*fFile;

public:

TTextControl	*fBcc;
TTextControl	*fCc;
TTextControl	*fSubject;
TTextControl	*fTo;

				THeaderView(BRect, BRect, bool, BFile*, bool); 
virtual void	MessageReceived(BMessage*);
void			BuildMenus(void);
void			SetAddress(BMessage*);
};


//====================================================================

// Word Services Begin
class TTextControl : public WSTextControl {
// Word Services End

private:

bool			fIncoming;
bool			fResending;
char			fLabel[100];
int32			fCommand;
BFile			*fFile;

public:

				TTextControl(BRect, char*, BMessage*, bool, BFile*, bool);
virtual void	AttachedToWindow(void);
virtual void	MessageReceived(BMessage*);
};
#endif
