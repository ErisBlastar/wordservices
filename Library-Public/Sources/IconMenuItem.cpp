#include <MenuItem.h>
#include <fs_attr.h>
#include <Bitmap.h>

#include "IconMenuItem.h"

IconMenuItem::IconMenuItem( const char *label,
						BMessage *message,
						char shortcut,
						uint32 modifiers,
						BNode *nodePtr )
	: BMenuItem( label, message, shortcut, modifiers ),
	mIconBitMap( NULL )
{
	status_t stat;
	attr_info attrInf;
	
	stat = nodePtr->GetAttrInfo( "BEOS:M:STD_ICON", &attrInf );
	
	if ( stat )
		return;
	
	if ( attrInf.type != 'MICN' )
		return;
	
	char *buf = new char[ attrInf.size ];
	if ( !buf )
		return;
		
	size_t size;
	
	size = nodePtr->ReadAttr( "BEOS:M:STD_ICON",
								'MICN',
								0,
								buf,
								attrInf.size );
	
	if ( size != attrInf.size ){
		delete buf;
		return;
	}
	
	MakeBitmap( buf, attrInf.size );
	
	return;
}

IconMenuItem::IconMenuItem( const char *label,
						BMessage *message,
						char shortcut,
						uint32 modifiers,
						const BBitmap *iconBitMap )
	: BMenuItem( label, message, shortcut, modifiers ),
	mIconBitMap( NULL )
{
	mIconBitMap = new BBitmap( iconBitMap->Bounds(), B_COLOR_8_BIT );
	if ( !mIconBitMap )
		return;
	
	mIconBitMap->SetBits( iconBitMap->Bits(),
							iconBitMap->BitsLength(),
							0,
							iconBitMap->ColorSpace() );
	return;
}

IconMenuItem::IconMenuItem( const char *label,
						BMessage *message,
						char shortcut,
						uint32 modifiers,
						const char *iconBytes )
	: BMenuItem( label, message, shortcut, modifiers ),
	mIconBitMap( NULL )
{
	if ( iconBytes )
		MakeBitmap( iconBytes, 256 );
	
	return;
}

IconMenuItem::~IconMenuItem()
{
	delete mIconBitMap;

	return;
}

void IconMenuItem::MakeBitmap( const char *bytes, size_t )
{
	mIconBitMap = new BBitmap( BRect( 0, 0, 15, 15 ), B_COLOR_8_BIT );
	if ( !mIconBitMap )
		return;
		
	mIconBitMap->SetBits( bytes, 256, 0, B_COLOR_8_BIT );
	
	return;
}

void IconMenuItem::Highlight( bool isHi )
{
	BMenuItem::Highlight(isHi);
	
	// Antialiased text looks bad when highlighted.  Don't know what to
	// do about it.
	//DrawContent();

	return;
}

void IconMenuItem::DrawContent()
{
	Menu()->SetDrawingMode( B_OP_OVER );
	
	BPoint where = ContentLocation();
	
	where.y -= 1;
	
	if ( mIconBitMap )
		Menu()->DrawBitmap( mIconBitMap, where );
	
	BPoint strLoc;
	
	strLoc.x = where.x + 20;
	
	float width;
	float height;
	
	GetContentSize( &width, &height );
	
	strLoc.y = where.y + height - 3;
			
	Menu()->DrawString( Label(), strLoc );
	
	return;
}
