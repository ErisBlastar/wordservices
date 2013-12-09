class BackgroundView: public BView{
	public:
		BackgroundView( BRect frame,
							const char *name,
							uint32 resizeMask,
							uint32 flags  );
		BackgroundView( rgb_color theColor, 
							BRect frame,
							const char *name,
							uint32 resizeMask,
							uint32 flags  );
		virtual ~BackgroundView();
};
 