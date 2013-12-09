#include "PictureView.h"

PictureView::PictureView( BPicture *pic,
							BRect frame,
							char *name,
							uint32 resizeMask,
							uint32 flags )
	: BView( frame, name, resizeMask, flags | B_WILL_DRAW ),
		mPic( pic )
{
	return;
}

PictureView::~PictureView()
{
	delete mPic;
}

void PictureView::AttachedToWindow()
{
	if ( Parent() ){
		SetViewColor( Parent()->ViewColor() );
	}
	
	BView::AttachedToWindow();
}

void PictureView::Draw( BRect updateRect )
{
	BView::Draw( updateRect );
	
	DrawPicture( mPic, BPoint( 0, 0 ) );
	
	return;
}
