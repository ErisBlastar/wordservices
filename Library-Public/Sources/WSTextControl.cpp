#include <TextControl.h>
#include <string.h>

#include "WSTextControl.h"

WSTextControl::WSTextControl(BRect frame,
						const char *name,
						const char *label, 
						const char *initial_text, 
						BMessage *message,
						uint32 rmask,
						uint32 flags )
	: BTextControl( frame, name, label, initial_text, message, rmask, flags ),
	mHasBackgroundHilite( false )
{
	return;
}

WSTextControl::~WSTextControl()
{
	return;
}

BHandler *WSTextControl::ResolveSpecifier( BMessage *msg,
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
				
	return BTextControl::ResolveSpecifier( msg, index, specifier, what, property );
}

void WSTextControl::MessageReceived( BMessage *msg )
{
	bool	handled = false;
	
	switch ( msg->what ){
		case B_GET_PROPERTY:
			handled = HandleGet( msg );
			break;
#if 0
		case B_SET_PROPERTY:
			handled = HandleSet( msg );
			break;
#endif
	}
	
	if ( !handled )
		BTextControl::MessageReceived( msg );
		
}

bool WSTextControl::HandleGet( BMessage *msg )
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

bool WSTextControl::HandleSize( BMessage *msg )
{
	BMessage reply( B_REPLY );
	reply.AddInt32( "error", B_NO_ERROR );
	reply.AddInt32( "result", 	TextView()->TextLength() );
	msg->SendReply( &reply );
	return true;
}

void WSTextControl::BackgroundHilite( bool isHi, int32 start, int32 finish )
{				
	mHasBackgroundHilite = isHi;
		
	TextView()->Select( start, finish );
	
	return;
}

void WSTextControl::Draw( BRect updateRect )
{
	BTextControl::Draw( updateRect );
	
	int32 start;
	int32 finish;
	
	TextView()->GetSelection( &start, &finish );
	
	if ( mHasBackgroundHilite )
		TextView()->Highlight( start, finish );
	
	return;
}

void WSTextControl::WindowActivated( bool active )
{
	// Turn off highlight so window activation will turn it back on
	
	if ( active ){
		if ( mHasBackgroundHilite ){
			mHasBackgroundHilite = false;
			
			//int32 start;
			//int32 finish;
			
			////GetSelection( &start, &finish );
			//Highlight( start, finish );	
		}
	}
	
	BTextControl::WindowActivated( active );
	
	return;
}

