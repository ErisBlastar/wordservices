// ===========================================================================
//	FindWindow.cpp
// 	by Peter Barrett
//	Copyright 1996 - 1998, Be Inc. All rights reserved.
// ===========================================================================

#include <string.h>
#include "FindWindow.h"
#include "Mail.h"

// ============================================================================
//	FindWindow is modeless...

#define FINDBUTTON 'find'

FindPanel::FindPanel(BRect rect) : BView(rect, "FindPanel", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW)
{
	BRect r = rect;
	r.bottom -= 30;
	r.InsetBy(8,8);
	BRect text = r;
	text.OffsetTo(B_ORIGIN);
	text.InsetBy(2,2);
	mBTextView = new BTextView(r,"BTextView",text,B_FOLLOW_ALL,B_WILL_DRAW);
	AddChild(mBTextView);
	
	r = rect;
	r.left = r.right - 100;
	r.top = r.bottom - 36;
	r.InsetBy(10,8);
	mFindButton = new BButton(r,"FINDBUTTON","Find", new BMessage(FINDBUTTON),B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	AddChild(mFindButton);
	
	mWindowGuess = NULL;
}

void FindPanel::AttachedToWindow()
{
	BView::AttachedToWindow();
	SetViewColor(216,216,216);
	Window()->SetDefaultButton(mFindButton);
	mFindButton->SetTarget(this);
	mBTextView->MakeFocus(true);
}

void FindPanel::Draw(BRect updateRect)
{
	BRect	r;

	r = mBTextView->Frame();
	r.InsetBy(-1,-1);
	StrokeRect(r);
	r.InsetBy(-1,-1);
	SetHighColor(128,128,255);
	MovePenTo(r.left,r.bottom);
	StrokeLine(BPoint(r.left,r.top));
	StrokeLine(BPoint(r.right,r.top));
	SetHighColor(240,240,255);
	StrokeLine(BPoint(r.right,r.bottom));
	StrokeLine(BPoint(r.left,r.bottom));
	SetHighColor(0,0,0);
}

//	Send copy to html view?

void FindPanel::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case FINDBUTTON:
			DoFind();
			break;
		default:
			BView::MessageReceived(msg);
	}
}

//	Send the find message to the nearest html window

void FindPanel::DoFind()
{
	mBTextView->SelectAll();
	char *text = (char *)mBTextView->Text();
	if (text == NULL || text[0] == 0) return;

	BWindow *window = mWindowGuess;
	if (window == NULL) {
		long i=0;
		while (window = be_app->WindowAt(i++)) {	// Send the text to a waiting window
			if (window != Window()) {
				BView *view = window->FindView("m_content");
				if (view)
					break;	// Found a window
			}
		}
	}
	
	if (window == NULL)
		return;
		
//	Found a window, send a find message
	
	BView *view = window->FindView("m_content");
	if (view) {
		window->Lock();
		BMessage msg(M_FIND);
		msg.AddString("findthis",text);
		window->PostMessage(&msg,view);
		window->Unlock();
	}
}

void FindPanel::WindowGuess(BWindow* window)
{
	mWindowGuess = window;
}

// ============================================================================

FindWindow* FindWindow::mFindWindow = NULL;
BRect FindWindow::mLastPosition(BRect(100,300,380,380));

FindWindow::FindWindow() : BWindow(FindWindow::mLastPosition, "Find", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	BRect r = FindWindow::mLastPosition;
	r.OffsetTo(0,0);
	mFindPanel = new FindPanel(r);
	AddChild(mFindPanel);
	mFindWindow = this;
}

FindWindow::~FindWindow()
{
	FindWindow::mLastPosition = Frame();
	mFindWindow = NULL;
}

void FindWindow::FindAgain()
{
	Lock();
	mFindPanel->DoFind();
	Unlock();
}

void FindWindow::WindowGuess(BWindow* window)
{
	mFindPanel->WindowGuess(window);
}
