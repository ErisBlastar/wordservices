#pragma once


class SaveFilePanel{
	public:
		SaveFilePanel( BLooper *recipient );
		~SaveFilePanel();
		void Show(){ mSavePanel->Show(); };
		void Hide(){ mSavePanel->Hide(); };
		enum{
			msgSaveFile = 'SvFl'
		};
	protected:
		BFilePanel	*mSavePanel;
};

class OpenFilePanel{
	public:
		OpenFilePanel( BLooper *recipient );
		~OpenFilePanel();
		void Show(){ mOpenPanel->Show(); };
		void Hide(){ mOpenPanel->Hide(); };
		enum{
			msgOpenFile = 'OpFl'
		};
	protected:
		BFilePanel	*mOpenPanel;
};


