#include "SizeControlScroll.h"
#include "SizeControl.h"

SizeControl::SizeControl( char *name, char *exampleText, BPoint where, const BFont &font )
	: BControl( BRect( where.x, where.y, 390, 170 ),
				name,
				name,
				new BMessage( msgSizeChanged ),
				B_FOLLOW_NONE,
				B_WILL_DRAW ),
		mExampleText( exampleText ),
		mSlider( NULL ),
		mFont( font )
{
	BRect	frame;
	
	SetValue( mFont.Size() );

	frame.left = Bounds().left + 50;
	frame.top = Bounds().bottom - 20;
	frame.right = Bounds().right - 10;
	frame.bottom = Bounds().bottom - 20 + B_H_SCROLL_BAR_HEIGHT;
	
	mSlider = new SizeControlScroll( frame,
								"Slider",
								this );
	AddChild( mSlider );
	
	mSlider->SetValue( mFont.Size() );
	
	mStringBounds = BRect( Bounds().left + 10,
							Bounds().top,
							Bounds().right,
							Bounds().bottom - 21 );
							
	
	return;
}

SizeControl::~SizeControl()
{
	delete [] mExampleText;
	
	return;
}

void SizeControl::Draw(BRect updateRect)
{
	mFont.SetSize( Value() );
	
	SetFont( &mFont, B_FONT_SIZE | B_FONT_FAMILY_AND_STYLE );
	
	DrawString( mExampleText, BPoint( mStringBounds.left, mStringBounds.bottom - 20 ) );
	
	return;
}

void SizeControl::ValueChanged( float newValue )
{
	SetValue( newValue );
	Invalidate( mStringBounds );
	
	return;
}