#include <TextView.h>
#include <Region.h>
#include <string.h>

#include "WSTextView.h"

#define BACKGROUND_HILITE 0		// Set to 1 to use in the view itself

WSTextView::WSTextView( BRect frame, char *name, BRect textRect, int32 resizeMask, int32 flags )
	: BTextView( frame, name, textRect, resizeMask, flags ),
	mHasBackgroundHilite( false )
{
	return;
}

WSTextView::~WSTextView()
{
	return;
}

void WSTextView::BackgroundHilite( bool isHi, int32 start, int32 finish )
{				
	mHasBackgroundHilite = isHi;
	
	int32 oldStart;
	int32 oldFinish;
	BRegion rgn;
	
	GetSelection( &oldStart, &oldFinish );
	GetTextRegion( oldStart, oldFinish, &rgn );
	Invalidate( rgn.Frame() );
		
	Select( start, finish );
	GetTextRegion( start, finish, &rgn );
	Invalidate( rgn.Frame() );
		
	return;
}

void WSTextView::Draw( BRect updateRect )
{
	BTextView::Draw( updateRect );
	
	int32 start;
	int32 finish;
	
	GetSelection( &start, &finish );
	
	if ( mHasBackgroundHilite )
		Highlight( start, finish );
	
	return;
}
		
void WSTextView::WindowActivated( bool active )
{
	// Turn off highlight so window activation will turn it back on
	
	if ( active ){
		if ( mHasBackgroundHilite ){
			mHasBackgroundHilite = false;
			
			//int32 start;
			//int32 finish;
			
			//GetSelection( &start, &finish );
			//Highlight( start, finish );	
		}
	}
	
	BTextView::WindowActivated( active );
	
	return;
}

BHandler *WSTextView::ResolveSpecifier( BMessage *msg,
										int32 index,
										BMessage *specifier,
										int32 what,
										const char *property )
{
	if ( strcmp( property, "Size" ) == 0 && msg->what == B_GET_PROPERTY )
		return this;

#if BACKGROUND_HILITE == 1	
	if ( strcmp( property, "BackgroundHilite" ) == 0 )
		return this;
#endif
				
	return BTextView::ResolveSpecifier( msg, index, specifier, what, property );
}

void WSTextView::MessageReceived( BMessage *msg )
{
	bool	handled = false;
	
	switch ( msg->what ){
		case B_GET_PROPERTY:
			handled = HandleGet( msg );
			break;

		case B_SET_PROPERTY:
			handled = HandleSet( msg );
			break;
	}
	
	if ( !handled )
		BTextView::MessageReceived( msg );
		
}

bool WSTextView::HandleSet( BMessage *msg )
{
	BMessage	spec;
	int32		index;
	int32		what;
	char		*prop;
	
	msg->GetCurrentSpecifier( &index, &spec, &what, &prop );

	// !!!! IMPORTANT BUG FIX Forgot the == 0 before - screwed everything up!
	
	if ( strcmp( prop, "BackgroundHilite" ) == 0){
		return HandleSetBackgroundHilite( msg, &spec, what );
	}
	
	// There is a bug in BTextView's handling of the B_REVERSE_RANGE form.
	// it inserts the characters rather than replacing them
	
	// The folks at Be say this is a feature, not a bug.  The server must send
	// messages to first delete the text and then insert new text.  I think I will
	// need to get the font 'n style of the text to be able to restore it.
	
#if 0
	if ( strcmp( prop, "Text" ) == 0 && what == B_REVERSE_RANGE_SPECIFIER ){
		status_t err = B_NO_ERROR;
		BMessage	reply( B_REPLY );
		int32		textIndex;
		int32		range;
		
		err = spec.FindInt32( "index", &textIndex );
		if ( err ){
			reply.AddInt32( "error", err );
			msg->SendReply( &reply );
			return true;
		}
		
		err = spec.FindInt32( "range", &range );
		if ( err ){
			reply.AddInt32( "error", err );
			msg->SendReply( &reply );
			return true;
		}
		
		char *theStr;
		err = msg->FindString( "data", &theStr );
		if ( err ){
			reply.AddInt32( "error", err );
			msg->SendReply( &reply );
			return true;
		}
		
		// The index is relative to the end
		
		textIndex = TextLength() - textIndex;
		
		Delete( textIndex - range, textIndex );
		Insert( textIndex - range, theStr, strlen( theStr ) );

		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return true;
	}
#endif
	
	return false;
}

bool WSTextView::HandleGet( BMessage *msg )
{
	BMessage spec;
	int32	index;
	int32	what;
	char	*prop;

	msg->GetCurrentSpecifier( &index, &spec, &what, &prop );
	
	if ( strcmp( prop, "Size" ) == 0 && what == B_DIRECT_SPECIFIER ){
		return HandleSize( msg );
	}
	
	return false;						
}

bool WSTextView::HandleSize( BMessage *msg )
{
	BMessage reply( B_REPLY );
	reply.AddInt32( "error", B_NO_ERROR );
	reply.AddInt32( "result", 	TextLength() );
	msg->SendReply( &reply );
	return true;
}

bool WSTextView::HandleSetBackgroundHilite( BMessage *msg, BMessage *spec, int32 what )
{	
	switch( what ){
		case B_REVERSE_RANGE_SPECIFIER:
			SetReverseRangeHilite( msg, spec );
			return true;
			break;
		case B_RANGE_SPECIFIER:
			//SetRange( msg, spec );
			return false;
			break;
	}
	
	return false;
}

void WSTextView::SetReverseRangeHilite( BMessage *msg, BMessage *spec )
{
	status_t err;
	BMessage	reply( B_REPLY );
	int32		textIndex;
	int32		range;


	err = spec->FindInt32( "index", &textIndex );
	if ( err ){

		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return;
	}
	
	err = spec->FindInt32( "range", &range );
	if ( err ){
		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return;
	}

	int32 realStart = TextLength() - textIndex;

	bool theData;
	
	err = msg->FindBool( "data", &theData );
	if ( err != B_NO_ERROR ){
		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return;
	}
	
	BackgroundHilite( theData, realStart, realStart + range );

	reply.AddInt32( "error", B_NO_ERROR );
	msg->SendReply( &reply );
	
	return;
}

