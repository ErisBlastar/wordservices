class SizeControlScroll;

class SizeControl: public BControl
{
	friend void SizeControlScroll::ValueChanged( float newValue );
			
	public:
		SizeControl( char *name, char *exampleText, BPoint where, const BFont &font );
		virtual ~SizeControl();
		virtual	void Draw(BRect updateRect);

		enum {
			msgSizeChanged = 'SzCh'
		};
	
	protected:
		float	mValue;
		char *mExampleText;
		BScrollBar	*mSlider;
		BRect		mStringBounds;
		BFont		mFont;

		void ValueChanged( float newValue );
		
};