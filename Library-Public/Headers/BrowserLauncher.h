class BrowserLauncher{
	public:
		enum{
			OpenURL = 'oURL'
		};
		static status_t GoToWebPage( BMessage *msg );
		static status_t GoToWebPage( char *url );
	protected:
};