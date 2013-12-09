#include "SizeControlScroll.h"
#include "SizeControl.h"

SizeControlScroll::SizeControlScroll(	BRect frame,
								const char *name,
								SizeControl *owner )
	: BScrollBar( frame, name, NULL, 1, 256, B_HORIZONTAL ),
		mOwner( owner )
{
	return;
}

SizeControlScroll::~SizeControlScroll()
{
	return;
}

void SizeControlScroll::ValueChanged( float newValue )
{
	BScrollBar::ValueChanged( newValue );
	
	if ( mOwner->Value() == newValue )
		return;

	mOwner->ValueChanged( newValue );
	
	return;
}

