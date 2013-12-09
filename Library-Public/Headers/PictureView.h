class PictureView: public BView
{
	public:
		PictureView( BPicture *pic,
						BRect frame,
						char *name,
						uint32 resizeMask = B_NOT_RESIZABLE,
						uint32 flags = 0 );
		virtual ~PictureView();
		virtual void AttachedToWindow();
		virtual void Draw( BRect updateRect );
	protected:
		BPicture	*mPic;
};
