class IconMenuItem: public BMenuItem
{
	public:
		IconMenuItem( const char *label,
						BMessage *message,
						char shortcut = 0,
						uint32 modifiers = 0,
						BNode *nodePtr = NULL );
		IconMenuItem( const char *label,
						BMessage *message,
						char shortcut = 0,
						uint32 modifiers = 0,
						const BBitmap *iconBitMap = NULL );
		IconMenuItem( const char *label,
						BMessage *message,
						char shortcut = 0,
						uint32 modifiers = 0,
						const char *iconBytes = NULL );
		virtual ~IconMenuItem();
	
	protected:
		BBitmap	*mIconBitMap;
		
		virtual	void DrawContent();
		virtual void Highlight( bool isHi );
		void MakeBitmap( const char *bytes, size_t len );
};
