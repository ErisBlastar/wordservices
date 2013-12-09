//--------------------------------------------------------------------
//	
//	Mail.h
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once

#ifndef MAIL_H
#define MAIL_H

#include <Alert.h>
#include <Application.h>
#include <Beep.h>
#include <Directory.h>
#include <E-mail.h>
#include <File.h>
#include <FilePanel.h>
#include <FindDirectory.h>
#include <Font.h>
#include <Looper.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <MessageFilter.h>
#include <PopUpMenu.h>
#include <Point.h>
#include <PrintJob.h>
#include <Query.h>
#include <Handler.h>
#include <Rect.h>
#include <Roster.h>
#include <ScrollView.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <Window.h>

#define	TITLE_BAR_HEIGHT	 25
#define	WIND_WIDTH			457
#define WIND_HEIGHT			400

enum	MESSAGES			{REFS_RECEIVED = 64, LIST_INVOKED, WINDOW_CLOSED,
							 CHANGE_FONT};

enum	TEXT				{SUBJECT_FIELD = REFS_RECEIVED + 64, TO_FIELD,
							 ENCLOSE_FIELD, CC_FIELD, BCC_FIELD, NAME_FIELD};

enum	MENUS	/* app */	{M_NEW = SUBJECT_FIELD + 64, M_PREFS, M_EDIT_SIGNATURE,
								M_FONT, M_STYLE, M_SIZE, M_BEGINNER, M_EXPERT,
				/* file */	 M_WRAP_TEXT, M_REPLY, M_REPLY_ALL, M_FORWARD, M_RESEND,
								M_HEADER, M_RAW, M_SEND_NOW, M_SEND_LATER,
								M_SAVE, M_PRINT_SETUP, M_PRINT, M_DELETE,
								M_CLOSE_READ, M_CLOSE_SAVED, M_CLOSE_SAME,
								M_CLOSE_CUSTOM, M_STATUS,
				/* edit */	 M_SELECT, M_QUOTE, M_REMOVE_QUOTE,
								M_SIGNATURE, M_FIND, M_FIND_AGAIN,
				/* encls */	 M_ADD, M_REMOVE,
							 M_TO_MENU, M_CC_MENU, M_BCC_MENU};

enum	USER_LEVEL			 {L_BEGINNER = 0, L_EXPERT};

enum	WINDOW_TYPES		 {MAIL_WINDOW = 0, PREFS_WINDOW, SIG_WINDOW};

#define SEPERATOR_MARGIN	  7

#define	VIEW_COLOR			216
#define FONT_SIZE			 10.0

#define QUOTE				">"

class	TMailWindow;
class	THeaderView;
class	TEnclosuresView;
class	TContentView;
class	TMenu;
class	TPrefsWindow;
class	TSignatureWindow;
// Word Services Begin
class	SelectionClient;
// Word Services End


//====================================================================

class TMailApp : public BApplication {

private:

int32			fWindowCount;
BFile			*fPrefs;
BMenu			*fLevelMenu;
TPrefsWindow	*fPrefsWindow;
TSignatureWindow	*fSigWindow;

public:

BFont			fFont;

				TMailApp(void);
				~TMailApp(void);
virtual void	AboutRequested(void);
virtual void	ArgvReceived(int32, char**);
virtual void	MessageReceived(BMessage*);
virtual bool	QuitRequested(void);
virtual void	ReadyToRun(void);
virtual void	RefsReceived(BMessage*);
TMailWindow*	FindWindow(entry_ref);
void			FontChange(void);
TMailWindow*	NewWindow(entry_ref *rec = NULL, char *to = NULL,
						  bool resend = false);
};

//--------------------------------------------------------------------

class TMailWindow : public BWindow {

private:

bool			fSigAdded;
bool			fDelete;
bool			fIncoming;
bool			fReplying;
bool			fResending;
bool			fSent;
BFile			*fFile;
BFilePanel		*fPanel;
BMenuItem		*fAdd;
BMenuItem		*fCut;
BMenuItem		*fCopy;
BMenuItem		*fHeader;
BMenuItem		*fPaste;
BMenuItem		*fPrint;
BMenuItem		*fPrintSetup;
BMenuItem		*fQuote;
BMenuItem		*fRaw;
BMenuItem		*fRemove;
BMenuItem		*fRemoveQuote;
BMenuItem		*fSendNow;
BMenuItem		*fSendLater;
BMenuItem		*fUndo;
BRect			fZoom;
TContentView	*fContentView;
THeaderView		*fHeaderView;
TEnclosuresView	*fEnclosuresView;
TMenu			*fSignature;

// Word Services Begin
SelectionClient	*mClientSession;

void StartBatchCheck( BMessage *msg );
void HandleCheckTermination();
// Word Services End

public:

entry_ref		*fRef;

				TMailWindow(BRect, char*, entry_ref*, char*, BFont*, bool);
				~TMailWindow(void);
virtual void	FrameResized(float, float);
virtual void	MenusBeginning(void);
virtual void	MessageReceived(BMessage*);
virtual bool	QuitRequested(void);
virtual void	Show(void);
virtual void	Zoom(BPoint, float, float);

// Word Services Begin
virtual void WindowActivated( bool active );
// Word Services End

void			AddSignature(BMailMessage*);
void			Forward(entry_ref*);
void			Print(void);
void			PrintSetup(void);
void			Reply(entry_ref*, TMailWindow*, bool);
status_t		Send(bool);
};

//====================================================================

class TMenu: public BMenu {

private:

char			*fAttribute;
char			*fPredicate;
int32			fMessage;

public:
				TMenu(const char*, const char*, int32);
				~TMenu(void);
virtual void	AttachedToWindow(void);
void			BuildMenu(void);
};

//====================================================================

int32	header_len(BFile*);
#endif
