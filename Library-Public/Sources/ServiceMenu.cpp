#include <Menu.h>
#include <MenuItem.h>
#include <VolumeRoster.h>
#include <Query.h>
#include <stdio.h>

#include "ServiceMenu.h"

#if WS_WEB_PAGE_MENU == 1
#include "BrowserLauncher.h"
#endif

#if WS_SERVER_ICON_MENUS == 1
#include "IconMenuItem.h"
#endif

ServiceMenu::ServiceMenu( BHandler *target )
	: BMenu( "Services", B_ITEMS_IN_COLUMN ),
	mTarget( target )
{
	BVolumeRoster	vRos;
	BVolume			vol;
	BQuery			query;
	BEntry			entry;
	status_t		err;
	
#if WS_WEB_PAGE_MENU == 1
	AddWSWebPageMenuItem();
#endif

	while ( ( err = vRos.GetNextVolume( &vol ) ) == B_NO_ERROR ){
		query.Clear();
		query.SetVolume( &vol );
		
#if 0
		query.PushAttr( "WordServicesServer" );
		query.PushInt32( 1 );
		query.PushOp( B_GE );
#endif
		query.SetPredicate( "WordServicesServer = 1" );
				
		query.Fetch();
		
		while ( ( err = query.GetNextEntry( &entry ) ) == B_NO_ERROR ){
			AddEntry( entry );
		}
	}
	
	return;
}

ServiceMenu::~ServiceMenu()
{
	return;
}

void ServiceMenu::AddEntry( BEntry &entry )
{
	entry_ref	ref;
	BNode		node( &entry );
	int32		theLong;
	int32		i;
	char		menuString[ 256 ];
	char		attrName[ 256 ];
	BMessage	*msg;
	
	entry.GetRef( &ref );
	
	node.ReadAttr("Menu String Count", 
         			B_INT32_TYPE, 
         			0, 
         			&theLong, 
         			sizeof( theLong ) );
    
    for ( i = 0; i < theLong; i++ ){
	
		msg = new BMessage( msgStartBatchCheck );
	
		status_t	stat;
		
		stat = msg->AddRef( "Server", &ref );
		
		stat = msg->AddInt32( "Index", i );

		sprintf( attrName, "Menu String %d", i );
		
		node.ReadAttr( attrName,
						B_STRING_TYPE,
						0,
						menuString,
						256 );

#if WS_SERVER_ICON_MENUS == 1
		BMenuItem *item = new IconMenuItem( menuString, msg, 0, 0, &node );
#else						
		BMenuItem *item = new BMenuItem( menuString, msg );
#endif
		
		item->SetTarget( mTarget );

		AddItem( item );
	}
	return;
}

#if WS_WEB_PAGE_MENU == 1
void ServiceMenu::AddWSWebPageMenuItem()
{
	BMessage *msg = new BMessage( BrowserLauncher::OpenURL );
	msg->AddString( "URL", "http://www.wordservices.org/be.html" );

#if WS_SERVER_ICON_MENUS == 1
		// use an IconMenuItem so the labels line up
		BMenuItem *item = new IconMenuItem( "Word Services Web Page", msg, 0, 0, (char*)NULL );
#else						
		BMenuItem *item = new BMenuItem( "Word Services Web Page", msg );
#endif
	
	item->SetTarget( mTarget );
	
	AddItem( item );
	
	AddSeparatorItem();

	return;
}
#endif
