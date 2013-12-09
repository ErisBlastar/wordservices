/*
	QuickLetter for BeOS
	Letter Writing Module
	Copyright © 1997 Working Software, Inc.

*/


#pragma once

#include "QLLtrWindow.h"

class QLAboutBox;

class QLLtrapp: public WorkingApplication{

	public:

		QLLtrapp();

		virtual ~QLLtrapp ();

		virtual void MessageReceived( BMessage *msg );
		virtual	void ArgvReceived(int32 argc, char **argv);
		//virtual	void DispatchMessage(BMessage *message, BHandler *handler);

		virtual bool QuitRequested();
		virtual void AboutRequested();
		virtual void AboutRequested( bool splash );
		
	private:

		QLLtrWindow	*Window1;

};
