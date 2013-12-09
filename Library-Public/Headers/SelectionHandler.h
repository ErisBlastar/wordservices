// A BHandler that can receive messages with specifiers aimed at just
// a portion of a text view.  The text can change it's length while being
// operated on but the handler will keep the right concept of where the
// selection is at.
class WSTextView;
class WSTextControl;

class SelectionHandler: public BHandler
{
	public:
		SelectionHandler( WSTextView *aTextView );
		SelectionHandler( WSTextControl *aTextControl );
		virtual ~SelectionHandler();
		virtual void MessageReceived( BMessage *msg );
		virtual BHandler *ResolveSpecifier( BMessage *msg,
										int32 index,
										BMessage *specifier,
										int32 what,
										const char *property );
	protected:
		WSTextView		*mTextView;
		WSTextControl	*mTextControl;
		
		BTextView		*mCommonTextView;
		
		int32		mSelStartFromTop;
		int32		mSelEndFromBottom;

		void Construct( BLooper *looper );

		bool HandleGet( BMessage *msg );
		bool HandleSet( BMessage *msg );
		void HandleGetSize( BMessage *msg );
		bool HandleGetText( BMessage *msg, BMessage *spec, int32 what );
		void GetRange( BMessage *msg, BMessage *spec );
		//void GetReverseRange( BMessage *msg, BMessage *spec );
		bool HandleSetText( BMessage *msg, BMessage *spec, int32 what );
		void SetReverseRange( BMessage *msg, BMessage *spec );
		bool HandleSetBackgroundHilite( BMessage *msg, BMessage *spec, int32 what );
		void SetReverseRangeHilite( BMessage *msg, BMessage *spec );
};