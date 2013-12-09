class SelectionHandler;
class WSTextView;
class WSTextControl;

class SelectionClient: public WSClientSession
{
	public:
		SelectionClient( WSTextView *aTextView, BMessenger *serverMsgr  );
		SelectionClient( WSTextControl *aTextControl, BMessenger *serverMsgr  );
		virtual ~SelectionClient();
		BHandler *GetHandler() const;
		BMessenger *GetServerMessenger();
		
	protected:
		SelectionHandler	*mHandler;
		BMessenger			*mServerMessenger;
		
};