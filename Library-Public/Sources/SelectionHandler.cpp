#include <Handler.h>
#include <TextView.h>
#include <TextControl.h>
#include <string.h>

#include "SelectionHandler.h"
#include "WSTextView.h"
#include "WSTextControl.h"

SelectionHandler::SelectionHandler( WSTextControl *aTextControl )
	: mTextView( NULL ),
	mTextControl( aTextControl ),
	mCommonTextView( NULL )
{
	mCommonTextView = mTextControl->TextView();
	
	Construct( mTextControl->Looper() );
	
	return;
}

SelectionHandler::SelectionHandler( WSTextView *aTextView )
	: mTextView( aTextView ),
	mTextControl( NULL ),
	mCommonTextView( aTextView )
{
	Construct( mTextView->Looper() );
	
	return;
}

void SelectionHandler::Construct( BLooper *looper )
{	
	int32 start;
	int32 finish;
	int32 len;

	// Associate ourselves with the same looper that the text view is
	// attached to.
	
	looper->AddHandler( this );
	
	// If a selection exists, spellcheck only the selection.  If there
	// is no selection, spellcheck the entire document.
	
	mCommonTextView->GetSelection( &start, &finish );
	
	if ( start != finish ){
		mSelStartFromTop = start;
	
		len = mCommonTextView->TextLength();
	
		mSelEndFromBottom = len - finish;
	}else{
		mSelStartFromTop = 0;		// Starts at the beginning
		mSelEndFromBottom = 0;		// ends at the end
	}
		
	return;
}

SelectionHandler::~SelectionHandler()
{
	// Remove ourselves from our looper
	
	Looper()->RemoveHandler( this );

	return;
}

void SelectionHandler::MessageReceived( BMessage *msg )
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
		BHandler::MessageReceived( msg );
		
}

BHandler *SelectionHandler::ResolveSpecifier( BMessage *msg,
										int32 index,
										BMessage *specifier,
										int32 what,
										const char *property )
{
	if ( strcmp( property, "Size" ) == 0 && msg->what == B_GET_PROPERTY )
		return this;
	
	if ( strcmp( property, "Text" ) == 0 )
		return this;
	
	if ( strcmp( property, "BackgroundHilite" ) == 0 )
		return this;
				
	return BHandler::ResolveSpecifier( msg, index, specifier, what, property );
}

bool SelectionHandler::HandleGet( BMessage *msg )
{
	BMessage spec;
	int32	index;
	int32	what;
	char	*prop;
	bool	handled;
	
	msg->GetCurrentSpecifier( &index, &spec, &what, &prop );
	
	if ( strcmp( prop, "Size" ) == 0 && what == B_DIRECT_SPECIFIER ){
		HandleGetSize( msg );
		return true;
	}

	if ( strcmp( prop, "Text" ) == 0 ){
		handled = HandleGetText( msg, &spec, what );
		return handled;
	}
	
	return false;
}					

bool SelectionHandler::HandleSet( BMessage *msg )
{
	BMessage spec;
	int32	index;
	int32	what;
	char	*prop;
	bool	handled;

	msg->GetCurrentSpecifier( &index, &spec, &what, &prop );

	if ( strcmp( prop, "Text" ) == 0 ){
		handled = HandleSetText( msg, &spec, what );
		return handled;
	}
	
	if ( strcmp( prop, "BackgroundHilite" ) == 0 ){
		handled = HandleSetBackgroundHilite( msg, &spec, what );
		return handled;
	}
	
	return false;
}					


void SelectionHandler::HandleGetSize( BMessage *msg )
{
	int32	selLen;
	int32	textLen;
	
	BMessage reply( B_REPLY );
	reply.AddInt32( "error", B_NO_ERROR );

	// We calculate this every time in case the length has changed during
	// a session.
	
	textLen = mCommonTextView->TextLength();
	
	selLen = textLen - mSelEndFromBottom - mSelStartFromTop;
	
	reply.AddInt32( "result", 	selLen );
		
	msg->SendReply( &reply );

	return;
}

bool SelectionHandler::HandleGetText( BMessage *msg, BMessage *spec, int32 what )
{	
	switch( what ){
		case B_REVERSE_RANGE_SPECIFIER:
			//GetReverseRange( msg, spec );
			//return true;
			return false;		// STUB don't need get yet
			break;
		case B_RANGE_SPECIFIER:
			GetRange( msg, spec );
			return true;
			break;
	}
	
	return false;
}

bool SelectionHandler::HandleSetText( BMessage *msg, BMessage *spec, int32 what )
{	
	switch( what ){
		case B_REVERSE_RANGE_SPECIFIER:
			SetReverseRange( msg, spec );
			return true;
			break;
		case B_RANGE_SPECIFIER:
			//SetRange( msg, spec );
			return false;
			break;
	}
	
	return false;
}

bool SelectionHandler::HandleSetBackgroundHilite( BMessage *msg, BMessage *spec, int32 what )
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

#if 0
void SelectionHandler::GetReverseRange( BMessage *msg, BMessage *spec )
{
	status_t err = B_NO_ERROR;
	BMessage	reply( B_REPLY );
	int32		textIndex;
	int32		range;


	err = spec->FindInt32( "index", &textIndex );
	if ( err ){
		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return true;
	}
	
	err = spec->FindInt32( "range", &range );
	if ( err ){
		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return true;
	}

	// STUB get the text
	
	reply.AddInt32( "error", err );
	msg->SendReply( &reply );
	
	return;
}
#endif

void SelectionHandler::GetRange( BMessage *msg, BMessage *spec )
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

	char *buf = new char[ range + 1 ];
	if ( !buf ){	
		reply.AddInt32( "error", B_NO_MEMORY );
		msg->SendReply( &reply );
		return;
	}

	int32 realStart = mSelStartFromTop + textIndex;
	
	mCommonTextView->GetText( realStart, range, buf );
	
	reply.AddString( "result", buf );
	
	delete buf;
	
	reply.AddInt32( "error", B_NO_ERROR );
	msg->SendReply( &reply );
	
	return;
}

void SelectionHandler::SetReverseRange( BMessage *msg, BMessage *spec )
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

	int32 realStart = mCommonTextView->TextLength()
						- mSelEndFromBottom
						- textIndex;

	char *theData;
	
	err = msg->FindString( "data", &theData );
	if ( err == B_NAME_NOT_FOUND ){
		// This is OK.  Delete the text.
		mCommonTextView->Delete( realStart, realStart + range );
		reply.AddInt32( "error", B_NO_ERROR );
		msg->SendReply( &reply );
		return;
	}else if ( err != B_NO_ERROR ){
		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return;
	}
	
	// Got the data, insert it
	
	mCommonTextView->Insert( realStart, theData, range );		

	reply.AddInt32( "error", B_NO_ERROR );
	msg->SendReply( &reply );
	
	return;
}

void SelectionHandler::SetReverseRangeHilite( BMessage *msg, BMessage *spec )
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

	int32 realStart = mCommonTextView->TextLength()
						- mSelEndFromBottom
						- textIndex;

	bool theData;
	
	err = msg->FindBool( "data", &theData );
	if ( err != B_NO_ERROR ){
		reply.AddInt32( "error", err );
		msg->SendReply( &reply );
		return;
	}
	
	if ( mTextView ){ 
		mTextView->BackgroundHilite( theData, realStart, realStart + range );
	}else if ( mTextControl ){
		mTextControl->BackgroundHilite( theData, realStart, realStart + range );	
	}else{
		reply.AddInt32( "error", B_NAME_NOT_FOUND );
		msg->SendReply( &reply );
		return;
	}
	
	reply.AddInt32( "error", B_NO_ERROR );
	msg->SendReply( &reply );
	
	return;
}
