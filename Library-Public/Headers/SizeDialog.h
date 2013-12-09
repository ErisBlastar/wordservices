class SizeControl;

class SizeDialog: public BWindow
{
	public:
		SizeDialog( char *exampleText, BLooper *target, const BFont &font );
		virtual ~SizeDialog();
		virtual void MessageReceived( BMessage *msg );

		enum{
			msgSetSize = 'StSz'
		};
		
	protected:
		BLooper	*mTarget;
		char	*mExampleText;
		SizeControl	*mSizeControl;

		enum{
			msgOK = 'OK  '
		};
};
