//--------------------------------------------------------------------
//	
//	Signature.h
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once
#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <Alert.h>
#include <Beep.h>
#include <Box.h>
#include <FindDirectory.h>
#include <Font.h>
#include <fs_index.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <TextControl.h>
#include <Window.h>

#define	SIG_WIDTH			457
#define SIG_HEIGHT			200

#define NAME_TEXT			"Name:"
#define NAME_FIELD_H		 29
#define NAME_FIELD_V		  8
#define NAME_FIELD_WIDTH	282
#define NAME_FIELD_HEIGHT	 16

#define SIG_TEXT			"Signature:"
#define SIG_TEXT_H			 12
#define SIG_TEXT_V			 45
#define SIG_FIELD_H			 59
#define SIG_FIELD_V			 36

#define INDEX_SIGNATURE		"_signature"

class	TMenu;
class	TNameControl;
class	TScrollView;
class	TSignatureView;
class	TSigTextView;

//====================================================================

class TSignatureWindow : public BWindow {

private:

BMenuItem		*fCut;
BMenuItem		*fCopy;
BMenuItem		*fDelete;
BMenuItem		*fPaste;
BMenuItem		*fSave;
BMenuItem		*fUndo;
BEntry			fEntry;
BFile			*fFile;
TMenu			*fSignature;
TSignatureView	*fSigView;

public:

				TSignatureWindow(BRect, BFont*);
				~TSignatureWindow(void);
virtual void	FrameResized(float, float);
virtual void	MenusBeginning(void);
virtual void	MessageReceived(BMessage*);
virtual bool	QuitRequested(void);
virtual void	Show(void);
bool			Clear(void);
bool			IsDirty(void);
void			Save(void);
};

//--------------------------------------------------------------------

class TSignatureView : public BBox {

private:

bool			fFocus;
float			fOffset;

public:

TNameControl	*fName;
TSigTextView	*fTextView;

				TSignatureView(BRect, BFont*); 
virtual	void	Draw(BRect);
void			Focus(bool);
void			FrameResized(float, float);
};

//====================================================================


class TNameControl : public BTextControl {

private:

char			fLabel[100];

public:

				TNameControl(BRect, char*, BMessage*);
virtual void	AttachedToWindow(void);
virtual void	MessageReceived(BMessage*);
};

//====================================================================

class TSigTextView : public BTextView {

private:

BFont			fFont;
TSignatureView	*fParent;

public:

bool			fDirty;

				TSigTextView(BRect, BRect, TSignatureView*, BFont*); 
virtual	void	AttachedToWindow(void);
virtual void	DeleteText(int32, int32);
virtual void	KeyDown(const char*, int32);
virtual void	InsertText(const char*, int32, int32, const text_run_array*);
virtual void	MessageReceived(BMessage*);
virtual void	MakeFocus(bool);
};
#endif
