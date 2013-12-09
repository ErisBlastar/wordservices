// ===========================================================================
//	FindWindow.h
// 	by Peter Barrett
//	Copyright 1996 - 1998, Be Inc. All rights reserved.
// ===========================================================================

#pragma once
#include <Rect.h>
#include <TextView.h>
#include <View.h>
#include <Window.h>

// ============================================================================
//	Floating find window, just one of them.....

class FindPanel : public BView {
public:
					FindPanel(BRect rect);
	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage *msg);
	virtual	void	Draw(BRect updateRect);
	
			void	DoFind();
			void	WindowGuess(BWindow* window);

protected:
		BButton*	mFindButton;
		BTextView*	mBTextView;
		BWindow*	mWindowGuess;
};

class FindWindow : public BWindow {
public:
					FindWindow();
	virtual			~FindWindow();

			void	FindAgain();
			void	WindowGuess(BWindow* window);
	
	static	FindWindow*	mFindWindow;

protected:
			FindPanel*	mFindPanel;
public:
	static	BRect		mLastPosition;
};

// ============================================================================

