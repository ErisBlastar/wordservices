/* AdamAddOn.h
 *	
 *	these are the basic routines available for building an Adam AddOn.
 *
 *	for Adam to recognize you, you MUST have at least:
 *		export these routines (either by export-all, or export by pragma).
 *		implement: addon_IDVersion, addon_IDName, addon_IDAuthor, addon_IDNotice.
 *
 *		to implement EMS-API add-ons, see the readme in the emsapi directory.
 *		you still need to implement the above for fucntions for the EMS-API.
 */

#pragma once

#define ADAM_ADDON_VERSION				(3)		// current version - should be bumped whenever API changes.

// these are the pFlags as passed BACK from CanCreateBView...
#define ADAM_ADDON_HDR_ONLY				0x01	// only get header lines in MIME of this body part.
#define ADAM_ADDON_REQUIRES_MIME		0x02	// wants MIME as input.
#define ADAM_ADDON_GENERATES_MIME		0x04	// generates MIME as output.

#pragma export on

//		the first block (addon_IDName, addon_IDAuthor and addon_IDNotice are
//			for telling the world who wrote the addon. see the code example for
//			more detail.
//		addon_CanCreateBView:
//			mime - the mime type of the data adam wishes you to consider building a view for.
//			parentMime - the mime type of the parent of this item.  Oftentimes, this is NULL.
//					useful sometimes to know if you're a sub-part of another type, what that parent is.
//			fIsAttached - does the MIME claim this data to be "attached" to the message.
//			pFlags - fill out with what you want.  See flags above.
//		addon_CreateBView:
//			mime, parentMime, fIsAttached - same as above.
//			pRect - the rect that you are going to work with.  notice the casting done.
//					this ugly casting stuff will go away when i change the API to be all
//					class based.  sorry for now...
//			pData - data to build the view from.
//			dataLen - what do you think?
//			partID - this is an internal number that Adam will want back when you wish to send
//					info to adam.  currently, only attachments do this, as noted in the
//					attachments example.
//					you don't fill this out - it is passed to you and you should store it if you
//					for future communication with Adam (many add-ons won't use this).

const int32		addon_IDVersion(void);			// you should just return ADAM_ADDON_VERSION

// IMPORTANT, PLEASE READ: here is the Adam logic for initialization:
//	
//	for each addon:
//		ems_globals = NULL;
//		if (implements ems_api) 
//			ems_plugin_init(..., &ems_globals, ...);
//		addon_globals = addon_Initialize(ems_globals);
//	
//  NOTE: so if you implement ems-api, you get ems_globals in addon_Initialize.
//	
//	Adam therefore stores TWO sets of globals for your addon.
//	they CAN be the same pointer, if you want, but don't you go freeing it twice...
//  when you deinitialize, you will be called like this:
//	
//	for each addon:
//		addon_DeInitialize(addon_globals);
//		ems_plugin_finish(ems_globals);
//
//	if you want to use the SAME globals for both a normal AddOn AND EMS-API, then you can
//	do this: addon_Initialize(void *globals) { return(globals); }	 // ** ONLY FOR EMS-API add-ons.
//	but be darn sure you only free globals in either addon_DeInitialize OR ems_plugin_finish, not both.
//	it of course, doesn't matter which.  but note that ems_plugin_finish is called last... so best to put it there.
const void *	addon_Initialize(void *ems_globals);	// called during load.  called only ONCE.  return your globals.
const void 		addon_DeInitialize(void *globals);		// called when adam is shutting down.  called only ONCE.

const char *	addon_IDName(void *globals);		// Name of the add-on (longer non-menu based name).
const char *	addon_IDAuthor(void *globals);		// Who are you? Your company?
const char *	addon_IDNotice(void *globals);		// Copyright Notice
const char *	addon_MenuName(void *globals);		// name that appears in Adam's menus (ignored for EMS-API AddOns).
bool 			addon_CanConfigure(void *globals);	// can you be launched to configure yourself?
bool 			addon_IsFilter(void *globals);		// are you a filter - are you to be launched with "-f" when new messages arrive?
													// a filter is configured by launching, run by "filter -f".

bool	 		addon_NickNames(void *globals, char *userTyped, char *name, char *email);

const char *	addon_SignatureName(void *globals, int32 idx);
char *			addon_Signature(void *globals, int32 idx);	// you must return a ptr allocated with malloc() so Adam can free it.

bool addon_CanCreateBView(void *globals, const char *mime, const char *parentMime, bool fIsAttached, int32 *pFlags);
BView *addon_CreateBView(void *globals, const char *mime, const char *parentMime, bool fIsAttached, void *pRect, void *pData, int32 dataLen, const char *partIDStr, const char *filename);

#pragma export reset
