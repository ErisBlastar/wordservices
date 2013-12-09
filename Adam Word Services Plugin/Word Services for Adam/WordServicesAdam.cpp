// WordServicesAdam.cpp
// Word Services client add-on for Adam from http://www.adamation.com/
// 27 Aug 97 Mike Crawford crawford@scruznet.com
// See the Word Services home page at http://www.wordservices.org/

#include "ems-be.h" // The EMS API
#include "WordServicesAdam.h"
#include "ServiceMenu.h"
#include "WSTextView.h"
#include "WordServices.h"
#include "BrowserLauncher.h"



int DoWordServices(BFile &inFile, BFile &outFile, emsProgress )
{
	WSAdamDialog *dlg = new WSAdamDialog( inFile, outFile );
	if ( !dlg ){
		return B_NO_MEMORY;
	}
	
	dlg->Show();
	
	status_t	exitVal;
	
	wait_for_thread( dlg->Thread(), &exitVal );
	
#ifdef DEBUG_LOG
	delete gLog;
#endif
	
	return B_NO_ERROR;
}

WSAdamDialog::~WSAdamDialog()
{
	return;
}

WSAdamDialog::WSAdamDialog( BFile &in, BFile &out )
	: BWindow( BRect( 100, 100, 500, 400 ),
				"Word Services",
				B_DOCUMENT_WINDOW,
				0 ),
	mInFile( in ),
	mOutFile( out ),
	mTextView( NULL )
{
	BuildMenuBar();
	BuildTextView();
	
	LoadText();
	
	return;
}

void WSAdamDialog::MessageReceived( BMessage *msg )
{
	switch (msg->what) 
	{
		case ServiceMenu::msgStartBatchCheck:
			StartBatchCheck( msg );
			break;

		case kWSTerminateBatchCheck:
			HandleCheckTermination();
			break;
			
		case BrowserLauncher::OpenURL:
			BrowserLauncher::GoToWebPage( msg );
			break;
	}
	
	return;
}

bool WSAdamDialog::QuitRequested()
{
	SaveText();
	
	return true;
}

status_t WSAdamDialog::LoadText()
{
	off_t	size;
	size_t	smallerSize;
	
	mInFile.GetSize( &size );
	smallerSize = size;				// convert 64 bit value to 32
	
	char *buf = new char[ size ];
	
	if ( !buf )
		return B_NO_MEMORY;
	
	ssize_t numRead;
	
	numRead = mInFile.Read( buf, size );
	if ( numRead != size )
		return -1;
	
	mTextView->SetText( buf, size );
	
	delete buf;
	
	return B_NO_ERROR;
}

status_t WSAdamDialog::SaveText()
{
	const char *buf = mTextView->Text();
	size_t len = mTextView->TextLength();
	
	mOutFile.Write( buf, len );
	
	return B_NO_ERROR;
}

void WSAdamDialog::BuildMenuBar()
{
	BMenuBar			*menuBar;
#define V_MENU_BAR 18	
	// build menubar
	BRect	barRect;
	barRect = Bounds();
	barRect.bottom = barRect.top + V_MENU_BAR;
	menuBar = new BMenuBar(barRect, "menu bar");
	
	ServiceMenu *servMenu = new ServiceMenu( this );
	
	menuBar->AddItem( servMenu );

	AddChild(menuBar);

}

void WSAdamDialog::BuildTextView(void)
{
	BRect frame = Bounds();
	frame.top += 20;
	frame.right -=  B_V_SCROLL_BAR_WIDTH;
	frame.bottom -= B_H_SCROLL_BAR_HEIGHT;
	
	BRect	textFrame = frame;
	
	// Offset to 0, 0
	textFrame.bottom -= textFrame.top;
	textFrame.top = 0;
	textFrame.right -= textFrame.left;
	textFrame.left = 0;
		
	mTextView = new WSTextView( frame, "TextView", textFrame, B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	mTextView->SetStylable( true );

	// Allow the text view to get messages sent by the Word Services server
	//AddHandler( mTextView );

	BScrollView *scrollv = new BScrollView( "Scroll", mTextView,
											B_FOLLOW_ALL_SIDES, B_WILL_DRAW,
											true, true, B_FANCY_BORDER );
											
	Lock();
	AddChild( scrollv );
	Unlock();
	
	BScrollBar *sbarv = scrollv->ScrollBar(B_VERTICAL);
	BScrollBar *sbarh = scrollv->ScrollBar(B_HORIZONTAL);
	sbarv->SetSteps(15,150);
	sbarv->SetProportion(1);
	sbarh->SetSteps(15,150);
	sbarh->SetProportion(.5);
	
	return;
}

void WSAdamDialog::StartBatchCheck( BMessage *msg )
{
	// We create a message to send to the speller.  This message
	// contains one or more BMessengers.  Each BMessenger specifies
	// one text block IN THE CLIENT that is to be spellchecked.  The
	// speller will use the messengers to send messages back to the text
	// block to get and set its data.
	
	BMessage batch( kWSBatchCheckMe );
	
	BMessenger mesr( mTextView );
	
	status_t stat;
	
	stat = batch.AddMessenger( "Target", mesr );
	
	// A Word Services server can have more than one service, for example
	// encryption and decryption.  The service index is stored in the menu
	// item message.
	
	int32	index;
	stat = msg->FindInt32( "Index", &index );
	
	stat = batch.AddInt32( "Index", index );
	
	entry_ref	server_ref;
	
	stat = msg->FindRef( "Server", &server_ref );
	
	app_info	info;
	
	be_roster->GetAppInfo( &server_ref, &info );
	
	if ( info.thread == -1 ){
		team_id	team;
		be_roster->Launch( &server_ref, (BMessage*)NULL, &team );
		be_roster->GetRunningAppInfo( team, &info );
		// Give the server time to get started
		snooze( 100000 );
	}
	
	BMessenger	sender( NULL, info.team );
	
	sender.SendMessage( &batch, this );
	
	return;
}

void WSAdamDialog::HandleCheckTermination()
{
	PostMessage( B_QUIT_REQUESTED );
		
	return;
}

