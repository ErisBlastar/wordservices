//--------------------------------------------------------------------
//	
//	Prefs.h
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once
#ifndef PREFS_H
#define PREFS_H

#include <Box.h>
#include <Button.h>
#include <Font.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Window.h>

#define	PREF_WIDTH			300
#define PREF_HEIGHT			200

#define BUTTON_WIDTH		 70
#define BUTTON_HEIGHT		 20

#define FONT_X1				 20
#define FONT_Y1				 15
#define FONT_X2				(PREF_WIDTH - FONT_X1)
#define FONT_Y2				(FONT_Y1 + 20)
#define FONT_TEXT			"Font"

#define SIZE_X1				FONT_X1
#define SIZE_Y1				(FONT_Y2 + 10)
#define SIZE_X2				(PREF_WIDTH - SIZE_X1)
#define SIZE_Y2				(SIZE_Y1 + 20)
#define SIZE_TEXT			"Size"

#define LEVEL_X1			FONT_X1
#define LEVEL_Y1			(SIZE_Y2 + 10)
#define LEVEL_X2			(PREF_WIDTH - LEVEL_X1)
#define LEVEL_Y2			(LEVEL_Y1 + 20)
#define LEVEL_TEXT			"User Level"

#define WRAP_X1				FONT_X1
#define WRAP_Y1				(LEVEL_Y2 + 10)
#define WRAP_X2				(PREF_WIDTH - WRAP_X1)
#define WRAP_Y2				(WRAP_Y1 + 20)
#define WRAP_TEXT			"Text Wrapping"

#define SIG_X1				FONT_X1
#define SIG_Y1				(WRAP_Y2 + 10)
#define SIG_X2				(PREF_WIDTH - SIG_X1)
#define SIG_Y2				(SIG_Y1 + 20)
#define SIGNATURE_TEXT		"Auto Signature"

#define OK_BUTTON_X1		(PREF_WIDTH - BUTTON_WIDTH - 6)
#define OK_BUTTON_Y1		(PREF_HEIGHT - (BUTTON_HEIGHT + 10))
#define OK_BUTTON_X2		(OK_BUTTON_X1 + BUTTON_WIDTH)
#define OK_BUTTON_Y2		(OK_BUTTON_Y1 + BUTTON_HEIGHT)
#define OK_BUTTON_TEXT		"OK"

#define CANCEL_BUTTON_X1	(OK_BUTTON_X1 - (BUTTON_WIDTH + 10))
#define CANCEL_BUTTON_Y1	OK_BUTTON_Y1
#define CANCEL_BUTTON_X2	(CANCEL_BUTTON_X1 + BUTTON_WIDTH)
#define CANCEL_BUTTON_Y2	OK_BUTTON_Y2
#define CANCEL_BUTTON_TEXT	"Cancel"

#define REVERT_BUTTON_X1	(CANCEL_BUTTON_X1 - (BUTTON_WIDTH + 10))
#define REVERT_BUTTON_Y1	CANCEL_BUTTON_Y1
#define REVERT_BUTTON_X2	(REVERT_BUTTON_X1 + BUTTON_WIDTH)
#define REVERT_BUTTON_Y2	CANCEL_BUTTON_Y2
#define REVERT_BUTTON_TEXT	"Revert"

#define SIG_NONE			"None"

enum	P_MESSAGES			{P_OK = 128, P_CANCEL, P_REVERT, P_FONT,
							 P_SIZE, P_LEVEL, P_WRAP, P_SIG};


//====================================================================

class TPrefsWindow : public BWindow {

private:

bool			fWrap;
bool			*fNewWrap;
char			*fSignature;
char			**fNewSignature;
int32			fLevel;
int32			*fNewLevel;
BButton			*fOK;
BButton			*fCancel;
BButton			*fRevert;
BFont			fFont;
BFont			*fNewFont;
BPopUpMenu		*fFontMenu;
BPopUpMenu		*fSizeMenu;
BPopUpMenu		*fLevelMenu;
BPopUpMenu		*fWrapMenu;
BPopUpMenu		*fSignatureMenu;

public:

				TPrefsWindow(BRect, BFont*, int32*, bool*, char**);
				~TPrefsWindow(void);
virtual void	MessageReceived(BMessage*);
BPopUpMenu*		BuildFontMenu(BFont*);
BPopUpMenu*		BuildLevelMenu(int32);
BPopUpMenu*		BuildSignatureMenu(char*);
BPopUpMenu*		BuildSizeMenu(BFont*);
BPopUpMenu*		BuildWrapMenu(bool);
};

#endif
