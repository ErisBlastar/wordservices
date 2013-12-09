class QLAboutBox: public BWindow
{
	public:
		QLAboutBox( bool splash );
		virtual ~QLAboutBox();
		void MessageReceived( BMessage *msg );
		virtual bool QuitRequested( void );
		virtual void WindowActivated( bool active );
		
		enum {
			msgSnoozeClose = 2000
		};
	protected:
		bool mSplash;
		
};