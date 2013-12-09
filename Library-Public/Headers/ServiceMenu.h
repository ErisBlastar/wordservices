// 1.0d2 MDC Go to a web page set up to explain Word Services and
// allow users to find W.S. applications.  This is optional, if you
// don't want to use it, but note that the www.wordservices.org web
// pages are vendor-neutral
	
#define WS_WEB_PAGE_MENU 1

// 1.0d6 Draw server application icons in the menus

#define WS_SERVER_ICON_MENUS 1

class ServiceMenu: public BMenu
{
	public:
		ServiceMenu( BHandler *target );
		virtual ~ServiceMenu();
		enum {
			msgStartBatchCheck = 'WsBC'
		};

	protected:
		BHandler	*mTarget;

		void AddEntry( BEntry &entry );
#if WS_WEB_PAGE_MENU == 1
		void AddWSWebPageMenuItem();
#endif
};