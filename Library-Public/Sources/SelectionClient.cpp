// Word Services client session handler that knows about checking a 
// BTextView selection

#include <Messenger.h>

#include "WSClientSession.h"
#include "SelectionClient.h"
#include "SelectionHandler.h"

SelectionClient::SelectionClient( WSTextView *aTextView, BMessenger *serverMsgr )
	: mHandler( NULL ),
	mServerMessenger( serverMsgr )
{
	mHandler = new SelectionHandler( aTextView );
	
	return;
}

SelectionClient::SelectionClient( WSTextControl *aTextControl, BMessenger *serverMsgr )
	: mHandler( NULL ),
	mServerMessenger( serverMsgr )
{
	mHandler = new SelectionHandler( aTextControl );
	
	return;
}

SelectionClient::~SelectionClient()
{
	delete mHandler;
	delete mServerMessenger;
}

BHandler *SelectionClient::GetHandler() const
{
	return mHandler;
}

BMessenger *SelectionClient::GetServerMessenger()
{
	return mServerMessenger;
}

