class SizeControl;

class SizeControlScroll: public BScrollBar
{
	public:
		SizeControlScroll(	BRect frame,
								const char *name,
								SizeControl *owner );
		virtual ~SizeControlScroll();
		virtual	void ValueChanged(float newValue);
	protected:
		SizeControl	*mOwner;
};
		
		