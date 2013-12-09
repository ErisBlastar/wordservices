/*

	QuickLetter for BeOS
	Letter Writing Module
	Copyright © 1997 Working Software, Inc.

*/


#include "WorkingApplication.h"
#include "QLLtrMain.h"

#include "QLLtrWindow.h"
#include "QLAboutBox.h"


main()
{
QLLtrapp *MyApp;

	MyApp = new QLLtrapp;
	
	MyApp->Run();
	
	delete MyApp;
	
}

QLLtrapp::QLLtrapp (): WorkingApplication( "application/x-vnd.working-QLdj" )
{
	AboutRequested( true );
	
	// create and display the window
	Window1 = new QLLtrWindow();
	Window1->Show();	
}

QLLtrapp::~QLLtrapp () {
}

void QLLtrapp::MessageReceived( BMessage *msg )
{
	switch ( msg->what ){
		case B_ARGV_RECEIVED:
			msg->PrintToStream();
		break;
	}
	
	return;
}

void QLLtrapp::ArgvReceived(int32 argc, char **argv)
{
	return;
}

void QLLtrapp::AboutRequested()
{
	AboutRequested( false );
	
	return;
}

void QLLtrapp::AboutRequested( bool splash )
{
	QLAboutBox *ab = new QLAboutBox( splash );
	ab->Show();
	return;
}

bool QLLtrapp::QuitRequested()
{
	BWindow	*window;
	int32	i = 0;
	
	while ( window = WindowAt( i++ )){
		window->PostMessage( B_QUIT_REQUESTED );
	}
	while ( window = WindowAt( i++ )){
		status_t	stat;
		wait_for_thread( window->Thread(), &stat );
	}
	return true;
}
