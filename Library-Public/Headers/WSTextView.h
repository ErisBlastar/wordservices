// Somewhat scriptable text view

class WSTextView: public BTextView
{
	public:
		WSTextView( BRect frame, char *name, BRect textRect, int32 resizeMask, int32 flags );
		virtual ~WSTextView();
		
		virtual BHandler *ResolveSpecifier( BMessage *msg,
												int32 index,
												BMessage *specifier,
												int32 what,
												const char *property );
		virtual void MessageReceived( BMessage *msg );	
		virtual void WindowActivated( bool active );									
		virtual void BackgroundHilite( bool isHi, int32 start, int32 finish );
		virtual void Draw( BRect updateRect );
	protected:
		bool	mHasBackgroundHilite;
		
		void Construct( BLooper looper );
		
		bool HandleGet( BMessage *msg );
		bool HandleSet( BMessage *msg );
		bool HandleSize( BMessage *msg );
		bool HandleSetBackgroundHilite( BMessage *msg, BMessage *spec, int32 what );
		void SetReverseRangeHilite( BMessage *msg, BMessage *spec );
};