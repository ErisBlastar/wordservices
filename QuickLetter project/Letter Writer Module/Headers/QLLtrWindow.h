/*
	QuickLetter for BeOS
	Letter Writing Module
	Copyright © 1997 Working Software, Inc.

*/


#pragma once

#include "MyFilePanel.h"

class SelectionClient;
class WSTextView;

class QLLtrWindow:public BWindow
{

public:

	QLLtrWindow();
	virtual	~QLLtrWindow();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage *message);
	virtual void WindowActivated( bool active );
	
protected:
	enum{
		msgNewWindow = 'NwWn',
		msgSetFont = 'StFt',
		msgAbout = 'Abot',
		msgSizeDialog = 'SzDg'
	};

	BFile				*mReadFile;
	BFile				*mSaveFile;
	OpenFilePanel		*mOpenPanel;
	SaveFilePanel		*mSavePanel;

	BView				*mWholeWindowView;
	WSTextView			*mTextView;

	float				mOld_width;
	float				mOld_height;

	SelectionClient		*mClientSession;
	
	void FrameResized(float new_width, float new_height);
	void BuildFileMenu( BMenuBar *menuBar );
	void BuildFontMenu( BMenuBar *menuBar );
	void HandleSetFont( BMessage *message );
	void BuildDialog(void);
	void BuildMenuBar();
	void BuildTextFields();
	void BuildWholeWindowView(void);
	void BuildSizeMenu( BMenuBar *menuBar );
	void HandleSetSize( BMessage *msg );
	void HandleSizeDialog();
	void StartBatchCheck( BMessage *msg );
	void HandleCheckTermination();
	
};


enum {
	V_MENU_BAR = 15,
	SOME_EVENT
};


enum {
	ADD_BUTTON = 5
};
