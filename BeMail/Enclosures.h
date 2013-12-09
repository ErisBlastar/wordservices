//--------------------------------------------------------------------
//	
//	Enclosures.h
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once
#ifndef ENCLOSURES_H
#define ENCLOSURES_H

#include <Bitmap.h>
#include <Box.h>
#include <File.h>
#include <ListView.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Point.h>
#include <Rect.h>
#include <ScrollView.h>
#include <View.h>
#include <Volume.h>

#define ENCLOSURES_HEIGHT	 94

#define ENCLOSE_TEXT		"Enclosures:"
#define ENCLOSE_TEXT_H		  7
#define ENCLOSE_TEXT_V		 5
#define ENCLOSE_FIELD_H		 59
#define ENCLOSE_FIELD_V		 11
#define ENCLOSE_FIELD_HEIGHT 61

class	TListView;
class	TMailWindow;
class	TScrollView;


//====================================================================

class TEnclosuresView : public BBox {

private:

bool			fFocus;
float			fOffset;
TMailWindow		*fWindow;

public:

TListView		*fList;

				TEnclosuresView(BRect, BRect); 
virtual	void	Draw(BRect);
virtual void	MessageReceived(BMessage*);
void			Focus(bool);
};


//====================================================================

class TListView : public BListView {

private:

TEnclosuresView	*fParent;

public:

				TListView(BRect, TEnclosuresView*);
virtual	void	AttachedToWindow(void);
virtual void	MakeFocus(bool);
};


//====================================================================

class TListItem : public BListItem {

private:

entry_ref*		fRef;

public:

				TListItem(entry_ref*);
virtual void	DrawItem(BView*, BRect, bool);
virtual	void	Update(BView*, const BFont*);
	entry_ref*	Ref(void);
};
#endif
