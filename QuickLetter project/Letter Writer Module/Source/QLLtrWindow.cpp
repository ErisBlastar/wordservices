/*
	QuickLetter for BeOS
	Letter Writing Module
	Copyright © 1997 Working Software, Inc.

*/


#include "QLLtrWindow.h"
#include "WorkingApplication.h"
#include "QLLtrMain.h"
#include "SizeDialog.h"
#include "ServiceMenu.h"
#include "WordServices.h"
#include "WSTextView.h"
#include "BrowserLauncher.h"
#include "WSClientSession.h"
#include "SelectionClient.h"

QLLtrWindow::QLLtrWindow()
	:BWindow(BRect( 20, 20, 500, 500 ), "QL", B_TITLED_WINDOW, 0),
	mClientSession( NULL )
{
	BuildWholeWindowView();
	BuildMenuBar();
	BuildDialog();

	mOpenPanel = new OpenFilePanel( be_app );
	mSavePanel = new SaveFilePanel( this );
}

QLLtrWindow::~QLLtrWindow()
{
	delete mClientSession;
}

void QLLtrWindow::BuildWholeWindowView(void)
{
	BRect frame = Bounds();
	mWholeWindowView = new BView( frame, "WindowView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	Lock();
	AddChild(mWholeWindowView);
	Unlock();
}
void QLLtrWindow::BuildDialog(void)
{
	BRect frame = mWholeWindowView->Bounds();
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
	mWholeWindowView->AddChild( scrollv );
	Unlock();
	
	BScrollBar *sbarv = scrollv->ScrollBar(B_VERTICAL);
	BScrollBar *sbarh = scrollv->ScrollBar(B_HORIZONTAL);
	sbarv->SetSteps(15,150);
	sbarv->SetProportion(1);
	sbarh->SetSteps(15,150);
	sbarh->SetProportion(.5);
	
	return;
}

// standard message hook: passes quit request to application.

bool QLLtrWindow::QuitRequested()
{
	//be_app->PostMessage(B_QUIT_REQUESTED);
	
	return(TRUE);
}
void QLLtrWindow::FrameResized(float new_width, float new_height)
{
	BRect rv(mOld_width, Bounds().top, new_width, Bounds().bottom);
	BRect rh(Bounds().left, mOld_height, Bounds().right, new_height);
	
	mOld_width = new_width;
	mOld_height = new_height;
	mWholeWindowView->Invalidate(rh);	// so it will draw
	mWholeWindowView->Invalidate(rv);	// so it will draw
	
}


// standard message hook: catches all custom messages
// 		handles key events passed from view
// 		handles button events directly
//		handles menu events

void QLLtrWindow::MessageReceived(BMessage *message)
{
	switch (message->what) 
	{
		case ServiceMenu::msgStartBatchCheck:
			StartBatchCheck( message );
			break;

		case kWSTerminateBatchCheck:
			HandleCheckTermination();
			break;
			
		case BrowserLauncher::OpenURL:
			BrowserLauncher::GoToWebPage( message );
			break;

		case msgSizeDialog:
			HandleSizeDialog();
			break;
		
		case SizeDialog::msgSetSize:
			HandleSetSize( message );
			break;

		case msgSetFont:
			HandleSetFont( message );
			break;

		case B_KEY_DOWN:
		{
			char entry = (char)(message->FindInt32("char"));
			
			// process filtered data
			//ProcessKey(entry);
		}
		break;
		
		default:
		{
			BWindow::MessageReceived(message);
		}
		break;
	}
}

void QLLtrWindow::StartBatchCheck( BMessage *msg )
{
	// We create a message to send to the speller.  This message
	// contains one or more BMessengers.  Each BMessenger specifies
	// one text block IN THE CLIENT that is to be spellchecked.  The
	// speller will use the messengers to send messages back to the text
	// block to get and set its data.
	
	entry_ref	server_ref;
	status_t stat;
	
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
	
	BMessenger	*serverMessenger = new BMessenger( NULL, info.team );

	mClientSession = new SelectionClient( mTextView, serverMessenger );
	
	BMessage batch( kWSBatchCheckMe );
	
	BMessenger mesr( mClientSession->GetHandler() );
	
	
	stat = batch.AddMessenger( "Target", mesr );
	
	// A Word Services server can have more than one service, for example
	// encryption and decryption.  The service index is stored in the menu
	// item message.
	
	int32	index;
	stat = msg->FindInt32( "Index", &index );
	
	stat = batch.AddInt32( "Index", index );
	
	
	serverMessenger->SendMessage( &batch, this );
	
	return;
}

void QLLtrWindow::WindowActivated( bool active )
{
	if ( active && mClientSession ){
		BMessage abort( kWSBatchCheckAbort );
		mClientSession->GetServerMessenger()->SendMessage( &abort );
		delete mClientSession;
		mClientSession = NULL;
	}
	
	return;
}

void QLLtrWindow::HandleCheckTermination()
{
	delete mClientSession;
	
	mClientSession = NULL;
	
	return;
}

// init function: constructs the menubar

void QLLtrWindow::BuildMenuBar()
{
	BMenuBar			*menuBar;
	
	// build menubar
	BRect	barRect;
	barRect = Bounds();
	barRect.bottom = barRect.top + V_MENU_BAR;
	menuBar = new BMenuBar(barRect, "menu bar");
	
	BuildFileMenu( menuBar );
	
	BuildFontMenu( menuBar );

	BuildSizeMenu( menuBar );
	
	ServiceMenu *servMenu = new ServiceMenu( this );
	
	menuBar->AddItem( servMenu );

	mWholeWindowView->AddChild(menuBar);

}

void QLLtrWindow::BuildSizeMenu( BMenuBar *menuBar )
{
	BMenuItem			*menuItem;
	BMenu				*menu;

	menu = new BMenu("Size", B_ITEMS_IN_COLUMN);

	menuItem = new BMenuItem("Other...", new BMessage(msgSizeDialog));
	menu->AddItem(menuItem);	
	
	menuBar->AddItem( menu );
	
	return;
}

void QLLtrWindow::BuildFileMenu( BMenuBar *menuBar )
{
	BMenuItem			*menuItem;
	BMenu				*menu;

	menu = new BMenu("File", B_ITEMS_IN_COLUMN);

	menuItem = new BMenuItem("About QuickLetter", new BMessage(B_ABOUT_REQUESTED));
	menuItem->SetTarget( be_app );
	menu->AddItem(menuItem);	

	menuItem = new BMenuItem("New", new BMessage(msgNewWindow));
	menu->AddItem(menuItem);	

	BMessage *msg = new BMessage( B_QUIT_REQUESTED );
	menuItem = new BMenuItem("Quit", msg );
	menuItem->SetTarget( be_app );

	menu->AddItem(menuItem);	

	menuBar->AddItem(menu);	

	return;
}

void QLLtrWindow::BuildFontMenu( BMenuBar *menuBar )
{
	BMenuItem	*menuItem;
	BMenu		*menu;
	BMenu		*styleMenu;
	BMenuItem	*familyItem;
	int32		numFamilies;
	int32		i;
	font_family	family;
	font_style	style;
	BFont		*font;
	BMessage	*msg;
	
	menu = new BMenu("Font", B_ITEMS_IN_COLUMN);

	numFamilies = count_font_families();
	
	for ( i = 0; i < numFamilies; i++ ){
		get_font_family( i, &family );
		
		styleMenu = new BMenu( family, B_ITEMS_IN_COLUMN );
		
		int32 numStyles;
		
		numStyles = count_font_styles( family );
		
		for ( int32 j = 0; j < numStyles; j++ ){
		
			get_font_style( family, j, &style );
			
			font = new BFont();
			font->SetFamilyAndStyle( family, style );
			
			msg = new BMessage(msgSetFont);
			msg->AddData( "Font", 'Font', font, sizeof( *font ) );
			
			menuItem = new BMenuItem( style, msg);
		
			styleMenu->AddItem( menuItem );
		}
		familyItem = new BMenuItem( styleMenu );
	
		menu->AddItem(familyItem);	
	}

	menuBar->AddItem( menu );	

	return;
}

void QLLtrWindow::BuildTextFields()
{
ulong resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP;
ulong flags = B_NAVIGABLE | B_PULSE_NEEDED;
BTextView *textArea;
BBox *box;

	textArea = new BTextView(BRect(70,70,200,200), "Text", BRect(70,70,200,200), resizingMode, flags);
	
	AddChild(textArea);
	box = new BBox(BRect (70,70,200,200), NULL, resizingMode, B_WILL_DRAW);
}

void QLLtrWindow::HandleSetFont( BMessage *message )
{
	BTextView	*textView;
	BFont		*font;
	status_t	stat;
	int32		size;
	
	stat = message->FindData( "Font", 'Font', (void**)&font, &size );
	if ( stat )
		return;

	textView = (BTextView*)mWholeWindowView->FindView( "TextView" );
	
	textView->SetFontAndColor( font, B_FONT_FAMILY_AND_STYLE );
	
	return;
}

void QLLtrWindow::HandleSetSize( BMessage *message )
{
	BTextView	*textView;
	BFont		font;
	status_t	stat;
	int32		size;
	
	stat = message->FindInt32( "Size", &size );
	if ( stat )
		return;

	textView = (BTextView*)mWholeWindowView->FindView( "TextView" );
	
	font.SetSize( size );
	
	textView->SetFontAndColor( &font, B_FONT_SIZE );
	
	return;
}

void QLLtrWindow::HandleSizeDialog()
{
	rgb_color	color;
	BFont		font;
	uint32		properties;
	int32		start;
	int32		end;
	char		*exampleText;
	BTextView	*textView;
	int32		length;
	
	textView = (BTextView*)mWholeWindowView->FindView( "TextView" );

	textView->GetFontAndColor( &font, &properties, &color );
	
	textView->GetSelection( &start, &end );
	
	length = end - start;
	if ( length > 255 )
		length = 255;
	
	exampleText = new char[ 256 ];
	
	if ( length ){
		textView->GetText( start, length, exampleText );
	}else{
		strcpy( exampleText, "Working Software, Inc." );
	}
	
	SizeDialog *dlg = new SizeDialog( exampleText, this, font );
	return;
}
