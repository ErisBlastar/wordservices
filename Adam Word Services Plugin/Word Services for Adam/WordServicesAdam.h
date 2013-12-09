// WordServicesAdam.cpp
// Word Services client add-on for Adam from http://www.adamation.com/
// 27 Aug 97 Mike Crawford crawford@scruznet.com
// See the Word Services home page at http://www.wordservices.org/

int DoWordServices(BFile &inFile, BFile &outFile, emsProgress );

class WSAdamDialog: public BWindow
{
	public:
		WSAdamDialog( BFile &inFile, BFile &outFile);
		virtual ~WSAdamDialog();
		
		virtual void MessageReceived( BMessage *msg );
		virtual bool QuitRequested();
	protected:
		BFile	&mInFile;
		BFile	&mOutFile;
		BTextView	*mTextView;
		
		void BuildMenuBar();
		void BuildTextView(void);
		status_t LoadText();
		status_t SaveText();
		void StartBatchCheck( BMessage *msg );
		void HandleCheckTermination();
};