//--------------------------------------------------------------------
//	
//	Content.h
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once
#ifndef CONTENT_H
#define CONTENT_H

#include <Box.h>
#include <Entry.h>
#include <File.h>
#include <FilePanel.h>
#include <FindDirectory.h>
#include <Font.h>
#include <fs_attr.h>
#include <List.h>
#include <Looper.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Point.h>
#include <PopUpMenu.h>
#include <Rect.h>
#include <ScrollView.h>
#include <TextView.h>
#include <Window.h>

// Word Services Begin
#include "WSTextView.h"
// Word Services End

#define MESSAGE_TEXT		"Message:"
#define MESSAGE_TEXT_H		 16
#define MESSAGE_TEXT_V		 5
#define MESSAGE_FIELD_H		 59
#define MESSAGE_FIELD_V		 11

#define CONTENT_TYPE		"content-type: "
#define CONTENT_ENCODING	"content-transfer-encoding: "
#define CONTENT_DISPOSITION	"Content-Disposition: "
#define MIME_TEXT			"text/"
#define MIME_MULTIPART		"multipart/"

class	TMailWindow;
class	TScrollView;
class	TTextView;

typedef struct {
	bool		header;
	bool		raw;
	bool		quote;
	bool		incoming;
	bool		close;
	bool		mime;
	TTextView	*view;
	BFile		*file;
	BList		*enclosures;
	sem_id		*stop_sem;
} reader;

enum ENCLOSURE_TYPE		{TYPE_ENCLOSURE = 100, TYPE_BE_ENCLOSURE,
						 TYPE_HTML, TYPE_FTP, TYPE_MAILTO};

typedef struct {
	int32		type;
	char		*name;
	char		*content_type;
	char		*encoding;
	int32		text_start;
	int32		text_end;
	off_t		file_offset;
	off_t		file_length;
	bool		saved;
	bool		have_ref;
	entry_ref	ref;
} hyper_text;

bool	get_semaphore(BWindow*, sem_id*);
bool	insert(reader*, char*, int32, bool);
bool	parse_header(char*, char*, off_t, char*, reader*, off_t*);
bool	strip_it(char*, int32, reader*);

class	TSavePanel;


//====================================================================

class TContentView : public BBox {

private:

bool			fFocus;
bool			fIncoming;
float			fOffset;
BFile			*fFile;

public:

TTextView		*fTextView;

				TContentView(BRect, bool, BFile*, BFont*); 
virtual	void	Draw(BRect);
virtual void	MessageReceived(BMessage*);
void			FindString(const char*);
void			Focus(bool);
void			FrameResized(float, float);
};

//====================================================================

// Word Services Begin
//class TTextView : public BTextView {
class TTextView : public WSTextView {
// Word Services End

private:

bool			fIncoming;
bool			fRaw;
char			*fYankBuffer;
int32			fLastPosition;
BFile			*fFile;
BFont			fFont;
TContentView	*fParent;
sem_id			fStopSem;
thread_id		fThread;
BList			*fEnclosures;
BPopUpMenu		*fMenu;
TSavePanel		*fPanel;

public:

bool			fHeader;
bool			fReady;

				TTextView(BRect, BRect, bool, BFile*, TContentView*, BFont*);
				~TTextView(void);
virtual	void	AttachedToWindow(void);
virtual void	KeyDown(const char*, int32);
virtual void	MakeFocus(bool);
virtual void	MessageReceived(BMessage*);
virtual void	MouseDown(BPoint);
virtual void	MouseMoved(BPoint, uint32, const BMessage*);
void			ClearList(void);
void			LoadMessage(BFile*, bool, bool, const char*);
void			Open(hyper_text*);
static status_t	Reader(reader*);
status_t		Save(BMessage*);
void			SaveBeFile(BFile*, char*, ssize_t);
void			StopLoad(void);
void			AddAsContent(BMailMessage*, bool);
};


//====================================================================

class TSavePanel : public BFilePanel {

private:

hyper_text		*fEnclosure;
TTextView		*fView;

public:
				TSavePanel(hyper_text*, TTextView*);
virtual void	SendMessage(const BMessenger*, BMessage*);
void			SetEnclosure(hyper_text*);
};
#endif
