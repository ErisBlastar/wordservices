#include <Message.h>
#include <Roster.h>
#include "BrowserLauncher.h"

status_t BrowserLauncher::GoToWebPage( BMessage *msg )
{
	char *url;
	status_t	stat;
	
	msg->FindString( "URL", 0, &url );
	
	stat = GoToWebPage( url );
	
	return stat;
}

status_t BrowserLauncher::GoToWebPage( char *url )
{
	status_t	stat;
	char		*argv[ 2 ];
	
	argv[ 0 ] = url;
	
	stat = be_roster->Launch( "application/x-vnd.Be-NPOS",
						1,
						argv  );
	
	return stat;
}
