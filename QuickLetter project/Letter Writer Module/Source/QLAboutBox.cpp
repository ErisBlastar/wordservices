#include "QLAboutBox.h"
#include "WorkingApplication.h"
#include "QLLtrMain.h"
#include "BrowserLauncher.h"
#include "BackgroundView.h"
#include "PictureView.h"

QLAboutBox::QLAboutBox( bool splash )
	: BWindow( BRect( 100, 100, 500, 300 ),
		"About QuickLetter Letter Writer",
		B_MODAL_WINDOW,
		B_NOT_RESIZABLE ),
		mSplash( splash )

{
	BackgroundView *dlgView = new BackgroundView( Bounds(),
													"DialogView",
													0,
													0 );
	
	AddChild( dlgView );

	BPicture *pic1 = ((WorkingApplication*)be_app)->GetPicture( 0 );
	
	PictureView *v = new PictureView( pic1, BRect( 20, 30, 93, 103 ), "Logo" );
												
	dlgView->AddChild( v );

	BStringView *sv = new BStringView( BRect( 110, 30, 390, 54 ), "a",
										"QuickLetter Letter Writer" );
										
	BFont font( be_bold_font );
	font.SetSize( 18 );
	sv->SetFont( &font );

	dlgView->AddChild( sv );
	sv = new BStringView( BRect( 110, 56, 390, 70 ), "b",
				"©1997 Working Software, Inc." );
	dlgView->AddChild( sv );
	sv = new BStringView( BRect( 110, 72, 390, 86 ), "c",
				"http://www.working.com" );
	dlgView->AddChild( sv );

	sv = new BStringView( BRect( 110, 104, 390, 118 ), "e",
				"Version 1.0d6" );
	dlgView->AddChild( sv );
	
	BButton *ok = new BButton( BRect( 300, 170, 340, 190 ),
								"OK",
								"OK",
								new BMessage( B_QUIT_REQUESTED ) );
								
	dlgView->AddChild( ok );
	
	BMessage *urlMsg = new BMessage( BrowserLauncher::OpenURL );
	urlMsg->AddString( "URL", "http://www.working.com/" );
								
	BButton *url = new BButton( BRect( 180, 170, 280, 190 ),
								"Go To Web Page",
								"Go To Web Page",
								urlMsg );
								
	dlgView->AddChild( url );
								
	return;
}

QLAboutBox::~QLAboutBox()
{
	return;
}

bool QLAboutBox::QuitRequested()
{
	//((QLLtrapp*)be_app)->AboutBoxQuitting();
	
	return true;
}

void QLAboutBox::MessageReceived( BMessage *msg )
{
	switch ( msg->what ){
		case msgSnoozeClose:
			UpdateIfNeeded();
			snooze( 1000000.0 );
			
			BMessage *quitMsg = new BMessage;
			quitMsg->what = B_QUIT_REQUESTED;
			PostMessage( quitMsg );
			break;

		case BrowserLauncher::OpenURL:
			BrowserLauncher::GoToWebPage( msg );
			break;
	}
	
	return;
}

void QLAboutBox::WindowActivated( bool active )
{
	if ( mSplash )
		PostMessage( msgSnoozeClose );
	
	BWindow::WindowActivated( active );
	
	return;
}