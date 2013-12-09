class WSTextControl: public BTextControl
{
	public:
		WSTextControl(BRect frame,
						const char *name,
						const char *label, 
						const char *initial_text, 
						BMessage *message,
						uint32 rmask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
						uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
		virtual ~WSTextControl();

		virtual BHandler *ResolveSpecifier( BMessage *msg,
												int32 index,
												BMessage *specifier,
												int32 what,
												const char *property );
		
		virtual void MessageReceived( BMessage *msg );
		virtual void BackgroundHilite( bool isHi, int32 start, int32 finish );
		virtual void WindowActivated( bool active );									
		virtual void Draw( BRect updateRect );
	protected:
		bool	mHasBackgroundHilite;

		bool HandleGet( BMessage *msg );
		bool HandleSize( BMessage *msg );
};
