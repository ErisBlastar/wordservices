#include "SizeDialog.h"
#include "SizeControlScroll.h"
#include "SizeControl.h"

SizeDialog::SizeDialog( char *exampleText, BLooper *target, const BFont &font )
	: BWindow( BRect( 100, 100, 500, 300 ),
				"Set Size",
				B_MODAL_WINDOW,
				B_NOT_RESIZABLE ),
	mTarget( target ),
	mSizeControl( NULL )
{
	BView	*wholeView;
	
	wholeView = new BView( Bounds(), "Whole", B_FOLLOW_ALL, 0 );
	
	AddChild( wholeView );

	//FooView *foo = new FooView( BPoint( 10, 190 ) );
	//wholeView->AddChild( foo );
	
	mSizeControl = new SizeControl( "SizeControl",
											exampleText,
											BPoint( 10, 10 ),
											font );
	wholeView->AddChild( mSizeControl );

	
	BButton *ok = new BButton( BRect( 300, 170, 340, 190 ),
								"OK",
								"OK",
								new BMessage( msgOK ) );
								
	wholeView->AddChild( ok );
	
	BButton *cancel = new BButton( BRect( 240, 170, 280, 190 ),
								"Cancel",
								"Cancel",
								new BMessage( B_QUIT_REQUESTED ) );
								
	wholeView->AddChild( cancel );
	
	Show();
								
	return;
}

SizeDialog::~SizeDialog()
{
	return;
}

void SizeDialog::MessageReceived( BMessage *msg )
{
	switch ( msg->what ){
		case msgOK:
			BMessage sizeMsg( msgSetSize );
			sizeMsg.AddInt32( "Size", mSizeControl->Value() );
			
			mTarget->PostMessage( &sizeMsg );
			
			PostMessage( B_QUIT_REQUESTED );
			break;
	}
	
	return;
}
