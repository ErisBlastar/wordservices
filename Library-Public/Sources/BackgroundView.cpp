#include "BackgroundView.h"

BackgroundView::BackgroundView( BRect frame,
									const char *name,
									uint32 resizeMask,
									uint32 flags  )
	: BView( frame, name, resizeMask, flags | B_WILL_DRAW )
{
	rgb_color theColor = { 220, 220, 220, 0 };

	SetViewColor( theColor );
		
	return;
}

BackgroundView::BackgroundView( rgb_color theColor, 
									BRect frame,
									const char *name,
									uint32 resizeMask,
									uint32 flags  )
	: BView( frame, name, resizeMask, flags )
{
	SetViewColor( theColor );
		
	return;
}

BackgroundView::~BackgroundView()
{
	return;
}
