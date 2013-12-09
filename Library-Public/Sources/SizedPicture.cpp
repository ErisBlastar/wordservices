#include "SizedPicture.h"

SizedPicture::SizedPicture( float width, float height )
	: BPicture(),
		mWidth( width ),
		mHeight( height )
{
	return;
}

SizedPicture::SizedPicture( const BPicture &pic, float width, float height )
	: BPicture( pic ),
		mWidth( width ),
		mHeight( height )
{
	return;
}

SizedPicture::SizedPicture( const SizedPicture &pic )
	: BPicture( pic ),
		mWidth( pic.Width() ),
		mHeight( pic.Height() )
{
	return;
}

SizedPicture::SizedPicture( char *data, size_t len )
	: BPicture( data + 8, len - 8 ),
		mWidth( *(float*)data ),
		mHeight( *(float*)(data + 4 ) )
{
	return;
}

SizedPicture::~SizedPicture()
{
	return;
}
		
float SizedPicture::Height() const
{
	return mHeight;
}

float SizedPicture::Width() const
{
	return mWidth;
}

