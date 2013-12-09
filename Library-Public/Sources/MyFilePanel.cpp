#include <Entry.h>
#include <Path.h>
#include <FilePanel.h>

#include "MyFilePanel.h"

SaveFilePanel::SaveFilePanel( BLooper *recipient )
{
	mSavePanel = new BFilePanel(B_SAVE_PANEL,			// mode
						new BMessenger(recipient),  // target
						NULL,                  		// start_dir
						0,						// flavors								//flavors
						false,                 				// multiselect
						new BMessage(msgSaveFile),			// message (what) 
						NULL,					//ref filter*
						false,					// modal
						true); 				//hide when done               
}

SaveFilePanel::~SaveFilePanel()
{
  delete mSavePanel; 
}


////////

OpenFilePanel::OpenFilePanel( BLooper *receiver )
{
  mOpenPanel = new BFilePanel(B_OPEN_PANEL,			// mode
								new BMessenger(receiver),	// target
								NULL,						// start_dir
								0,								//flavors
								false,							// multiselect
								new BMessage(B_REFS_RECEIVED),			// message (what) 
								NULL,						// Ref filter
								false,					// modal
								true);
}

OpenFilePanel::~OpenFilePanel()
{
  delete mOpenPanel; 
}