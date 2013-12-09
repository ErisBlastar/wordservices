/* AdamAddOnView.h
 *
 *	Addons that can generate views for inclusion
 *	in the adam email system must generate their views
 *	as subclasses of AdamAddOnView.  This enables
 *	adam to ask special questions of the view that 
 *	are not expected of normal BView objects.
 *
 *	As a developer, you are simply implementing a BView
 *	that happens to answer a few extra questions.  Those
 *	extra questions are shown below and should be fairly
 *	easy to implement.  Sample code with this SDK should
 *	show you an example of this.
 */

#pragma once

#define ADAM_ADDON_DATATYPE_UNKNOWN	0	// signifies error condition.
#define ADAM_ADDON_DATATYPE_FILENAME	1	// add on wishes to pass back a filename
#define ADAM_ADDON_DATATYPE_BUFFER		2	// addon wishes to pass back a buffer.

#define ADAM_ADDON_MSG_STATE_UNKNOWN	0	// before state is set, it is unknown
#define ADAM_ADDON_MSG_STATE_CREATING	1	// we are creating a new message
#define ADAM_ADDON_MSG_STATE_READING	2	// reading an existing message
#define ADAM_ADDON_MSG_STATE_REPLYING	3	// replying to an existing message
#define ADAM_ADDON_MSG_STATE_FWDING	4	// forwarding an existing message

class AddOn;

class AdamAddOnView : public BView
{
public:
	AdamAddOnView(	const char *filename,
					BRect frame, const char *name,
					uint32 resizeMask, uint32 flags)
					:BView(frame,name,resizeMask,flags)
	{
		strcpy(mFileName, filename);
		mMsgState = ADAM_ADDON_MSG_STATE_UNKNOWN;
		mAddOnOwner = NULL;
	};

	//	addon should reload any configuration information that has changed.
	virtual	void	ReloadConfiguration(void) {};

	//
	// If the view is already in place, adam may wish to add more to that view.
	//	for example, if you are writing an attachment view, then you will want to
	//	receive multiple files to display in your object.
	//		CanAddToView:
	//			mime - the mime type of the data adam wants you to take.
	//			parentMime - the mime type of the parent of the data you're being asked to take.
	//					this can be really useful in some situations to have floating about.
	//					OFTENTIMES THIS IS NULL, so check it before you use it.
	//			fIsAttached - does the MIME claim this is to be attached.
	//			pFlags - fill out with your desired options.  see AdamAddOn.h for possibilities.
	//		AddToView:
	//			mime, parentMime, fIsAAttached (same as above).
	//			pData - pointer to the data of the message.
	//			dataLen - length of the data.
	//			partIDStr - the part ID that adam wants YOU to maintain and return when asked.
	//						this is adams internal information about which sub-bodypart of the MIME
	//						you have.  many times, this will be "" (when you're creating a new message).
	//						you pass this back as part of the 'extract' message outlined in the notes
	//						for writing attachment add-ons.
	//
	virtual	bool	CanAddToView(const char *mime, const char *parentMime, bool fIsAttached, int32 *pFlags) = 0;
	virtual	bool	AddToView(const char *mime, const char *parentMime, bool fIsAttached, void *pData, int32 dataLen, char *partIDStr) = 0;
	
	//	these functions allow the add-on to define how to reply to this type of object.
	//	if this object type can reply to itself, then GetReplyData builds a data structure that
	//	represents the reply.  This data structure will be passed to another instance of your add-on object that
	//	is created when the reply occurs.  This passing occurs in SetDataFromReply.
	//
	//	This means that you can build ANY data structure you'd like to create a "reply".
	//
	virtual	bool	CanReply(void) = 0;						// can you reply to this type of object?
	virtual	bool	GetReplyData(char ** /*pData*/, int32 * /*pDataLen*/) {return(false);};	// if yes, build data suitable for reply area.
	virtual	bool	SetDataFromReply(char * /*pReplyData*/, int32 /*replyDataLen*/) {return(false);};	// given data, build area as reply.

	//
	//	these functions inform the view what state the message is in.
	//	these are mostly used to determine whether the user can edit the
	//	view or not.
	//
	virtual int32	MessageState(void) { return (mMsgState);};
	virtual	void	SetMessageState(int32 state) {mMsgState = state;};
	
	//
	//	used to access an existing message to build a MIME message 
	//	for delivery.  Since this may be an enclosure type view, 
	//	we have to assume there are several parts to be handled.
	//
	virtual	int32		CountParts(void) = 0;		// how many parts are there?
	virtual	const char *MimeType(int32 idx) = 0;	// what is the mime type of this part?
	virtual	int32		DataType(int32 idx) = 0;	// is it a file or buffer?  see above types.
	virtual	const char *FileName(int32 idx) = 0;				// return the file name.
	virtual	const char *Data(int32 idx, int32 *pDataLen, bool fSelectedOnly = FALSE) = 0;	// return the buffer.
	virtual	void		ReplaceFileName(int32 idx, const char *fname, bool fSelectedOnly = FALSE) = 0;					// replace as file name.
	virtual	void		ReplaceData(int32 idx, const char *pData, int32 dataLen, bool fSelectedOnly = FALSE) = 0;		// replace as buffer.
	
	//
	//	these functions are for Adam to keep track of the addon that 'owns' this view.
	//	you shouldn't need (or use) these at all.
	//
	virtual AddOn *	AddOnOwner(void) { return (mAddOnOwner);};
	virtual void	SetAddOnOwner(AddOn *addon) {mAddOnOwner = addon;};
	
	//	these functions are used for printing.
	//	for PrintHeight, use the given print job to compute how much height you need.
	//	for PrintToRect, use the given rect to print with.  the rect should be setup for you,
	//			and you just need to modify the pRect->top to signify how much you've used.
	//			return TRUE when you're done printing, return FALSE if you have more printing to do.
	virtual	int32	PrintHeight(BPrintJob */*pjob*/) { return(0); };
	virtual	bool	PrintToRect(BPrintJob */*pjob*/, BRect */*pRect*/, BPoint */*pPagePos*/) { return(true); };
	
protected:
	char	mFileName[MAXPATHLEN+1];
	int32	mMsgState;			// is one of the possible states.
	AddOn	*mAddOnOwner;		// used by adam to track which addon owns this view.
};
