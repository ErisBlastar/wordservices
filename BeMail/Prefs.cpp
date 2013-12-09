//--------------------------------------------------------------------
//	
//	Prefs.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Mail.h"
#include "Prefs.h"

extern BPoint	prefs_window;


//====================================================================

TPrefsWindow::TPrefsWindow(BRect rect, BFont *font, int32 *level,
							bool *wrap, char **sig)
			  :BWindow(rect, "BeMail Preferences", B_TITLED_WINDOW,
									B_NOT_CLOSABLE |
			  						B_NOT_RESIZABLE |
			  						B_NOT_ZOOMABLE)
{
	BBox		*box;
	BFont		menu_font;
	BMenuField	*menu;
	BRect		r;

	fNewFont = font;
	fFont = *fNewFont;
	fNewLevel = level;
	fLevel = *fNewLevel;
	fNewWrap = wrap;
	fWrap = *fNewWrap;
	fNewSignature = sig;
	fSignature = (char *)malloc(strlen(*fNewSignature) + 1);
	strcpy(fSignature, *fNewSignature);

	r = Bounds();
	r.InsetBy(-1, -1);
	box = new BBox(r);
	AddChild(box);

	r.Set(OK_BUTTON_X1, OK_BUTTON_Y1, OK_BUTTON_X2, OK_BUTTON_Y2);
	fOK = new BButton(r, "ok", OK_BUTTON_TEXT, new BMessage(P_OK));
	fOK->MakeDefault(true);
	fOK->SetEnabled(false);
	box->AddChild(fOK);

	r.Set(CANCEL_BUTTON_X1, CANCEL_BUTTON_Y1, CANCEL_BUTTON_X2, CANCEL_BUTTON_Y2);
	fCancel = new BButton(r, "cancel", CANCEL_BUTTON_TEXT, new BMessage(P_CANCEL));
	box->AddChild(fCancel);

	r.Set(REVERT_BUTTON_X1, REVERT_BUTTON_Y1, REVERT_BUTTON_X2, REVERT_BUTTON_Y2);
	fRevert = new BButton(r, "revert", REVERT_BUTTON_TEXT, new BMessage(P_REVERT));
	fRevert->SetEnabled(false);
	box->AddChild(fRevert);

	r.Set(FONT_X1, FONT_Y1, FONT_X2, FONT_Y2);
	fFontMenu = BuildFontMenu(font);
	menu = new BMenuField(r, "font", FONT_TEXT, fFontMenu,
				B_FOLLOW_ALL,
				B_WILL_DRAW |
				B_NAVIGABLE |
				B_NAVIGABLE_JUMP);
	menu->GetFont(&menu_font);
	menu->SetDivider(menu_font.StringWidth(WRAP_TEXT) + 7);
	menu->SetAlignment(B_ALIGN_RIGHT);
	box->AddChild(menu);

	r.Set(SIZE_X1, SIZE_Y1, SIZE_X2, SIZE_Y2);
	fSizeMenu = BuildSizeMenu(font);
	menu = new BMenuField(r, "size", SIZE_TEXT, fSizeMenu,
				B_FOLLOW_ALL,
				B_WILL_DRAW |
				B_NAVIGABLE |
				B_NAVIGABLE_JUMP);
	menu->SetDivider(menu_font.StringWidth(WRAP_TEXT) + 7);
	menu->SetAlignment(B_ALIGN_RIGHT);
	box->AddChild(menu);

	r.Set(LEVEL_X1, LEVEL_Y1, LEVEL_X2, LEVEL_Y2);
	fLevelMenu = BuildLevelMenu(*level);
	menu = new BMenuField(r, "level", LEVEL_TEXT, fLevelMenu,
				B_FOLLOW_ALL,
				B_WILL_DRAW |
				B_NAVIGABLE |
				B_NAVIGABLE_JUMP);
	menu->SetDivider(menu_font.StringWidth(WRAP_TEXT) + 7);
	menu->SetAlignment(B_ALIGN_RIGHT);
	box->AddChild(menu);

	r.Set(WRAP_X1, WRAP_Y1, WRAP_X2, WRAP_Y2);
	fWrapMenu = BuildWrapMenu(*wrap);
	menu = new BMenuField(r, "wrap", WRAP_TEXT, fWrapMenu,
				B_FOLLOW_ALL,
				B_WILL_DRAW |
				B_NAVIGABLE |
				B_NAVIGABLE_JUMP);
	menu->SetDivider(menu_font.StringWidth(WRAP_TEXT) + 7);
	menu->SetAlignment(B_ALIGN_RIGHT);
	box->AddChild(menu);

	r.Set(SIG_X1, SIG_Y1, SIG_X2, SIG_Y2);
	fSignatureMenu = BuildSignatureMenu(*sig);
	menu = new BMenuField(r, "sig", SIGNATURE_TEXT, fSignatureMenu,
				B_FOLLOW_ALL,
				B_WILL_DRAW |
				B_NAVIGABLE |
				B_NAVIGABLE_JUMP);
	menu->SetDivider(menu_font.StringWidth(WRAP_TEXT) + 7);
	menu->SetAlignment(B_ALIGN_RIGHT);
	box->AddChild(menu);
	Show();
}

//--------------------------------------------------------------------

TPrefsWindow::~TPrefsWindow(void)
{
	BMessage	msg(WINDOW_CLOSED);

	prefs_window = Frame().LeftTop();

	msg.AddInt32("kind", PREFS_WINDOW);
	be_app->PostMessage(&msg);
}

//--------------------------------------------------------------------

void TPrefsWindow::MessageReceived(BMessage *msg)
{
	bool		changed;
	bool		revert = true;
	char		*family;
	char		*signature;
	char		*style;
	char		label[256];
	int32		new_size;
	int32		old_size;
	font_family	new_family;
	font_family	old_family;
	font_style	new_style;
	font_style	old_style;
	BMenuItem	*item;
	BMessage	message;

	switch (msg->what) {
		case P_OK:
			Quit();
			break;

		case P_CANCEL:
			revert = false;
		case P_REVERT:
			fFont.GetFamilyAndStyle(&old_family, &old_style);
			fNewFont->GetFamilyAndStyle(&new_family, &new_style);
			old_size = fFont.Size();
			new_size = fNewFont->Size();
			if ((strcmp(old_family, new_family)) || (strcmp(old_style, new_style)) ||
				(old_size != new_size)) {
				fNewFont->SetFamilyAndStyle(old_family, old_style);
				if (revert) {
					sprintf(label, "%s %s", old_family, old_style);
					item = fFontMenu->FindItem(label);
					item->SetMarked(true);
				}
			
				fNewFont->SetSize(old_size);
				if (revert) {
					sprintf(label, "%d", old_size);
					item = fSizeMenu->FindItem(label);
					item->SetMarked(true);
				}
				message.what = M_FONT;
				be_app->PostMessage(&message);
			}
			*fNewLevel = fLevel;
			*fNewWrap = fWrap;
			if (strcmp(fSignature, *fNewSignature)) {
				free(*fNewSignature);
				*fNewSignature = (char *)malloc(strlen(fSignature) + 1);
				strcpy(*fNewSignature, fSignature);
			}

			if (revert) {
				if (fLevel == L_EXPERT)
					strcpy(label, "Expert");
				else
					strcpy(label, "Beginner");
				item = fLevelMenu->FindItem(label);
				item->SetMarked(true);
	
				if (fWrap)
					strcpy(label, "On");
				else
					strcpy(label, "Off");
				item = fWrapMenu->FindItem(label);
				item->SetMarked(true);

				item = fSignatureMenu->FindItem(fSignature);
				item->SetMarked(true);
			}
			else
				Quit();
			break;

		case P_FONT:
			msg->FindString("font", &family);
			msg->FindString("style", &style);
			fNewFont->GetFamilyAndStyle(&old_family, &old_style);
			if ((strcmp(old_family, family)) || (strcmp(old_style, style))) {
				fNewFont->SetFamilyAndStyle(family, style);
				message.what = M_FONT;
				be_app->PostMessage(&message);
			}
			break;
		case P_SIZE:
			old_size = fNewFont->Size();
			msg->FindInt32("size", &new_size);
			if (old_size != new_size) {
				fNewFont->SetSize(new_size);
				message.what = M_FONT;
				be_app->PostMessage(&message);
			}
			break;
		case P_LEVEL:
			msg->FindInt32("level", fNewLevel);
			break;
		case P_WRAP:
			msg->FindBool("wrap", fNewWrap);
			break;
		case P_SIG:
			free(*fNewSignature);
			if (msg->FindString("signature", &signature) == B_NO_ERROR) {
				*fNewSignature = (char *)malloc(strlen(signature) + 1);
				strcpy(*fNewSignature, signature);
			}
			else {
				*fNewSignature = (char *)malloc(strlen(SIG_NONE) + 1);
				strcpy(*fNewSignature, SIG_NONE);
			}
			break;

		default:
			BWindow::MessageReceived(msg);
	}
	fFont.GetFamilyAndStyle(&old_family, &old_style);
	fNewFont->GetFamilyAndStyle(&new_family, &new_style);
	old_size = fFont.Size();
	new_size = fNewFont->Size();
	changed =	((old_size != new_size) ||
				(fLevel != *fNewLevel) ||
				(fWrap != *fNewWrap) ||
				(strcmp(old_family, new_family)) ||
				(strcmp(old_style, new_style)) ||
				(strcmp(fSignature, *fNewSignature)));
	fOK->SetEnabled(changed);
	fRevert->SetEnabled(changed);
}

//--------------------------------------------------------------------

BPopUpMenu *TPrefsWindow::BuildFontMenu(BFont *font)
{
	char		label[512];
	int32		family_count;
	int32		family_loop;
	int32		style_count;
	int32		style_loop;
	BMenuItem	*item;
	BMessage	*msg;
	BPopUpMenu	*menu;
	font_family	def_family;
	font_family	f_family;
	font_style	def_style;
	font_style	f_style;

	menu = new BPopUpMenu("");
	font->GetFamilyAndStyle(&def_family, &def_style);
	family_count = count_font_families();
	for (family_loop = 0; family_loop < family_count; family_loop++) {
		get_font_family(family_loop, &f_family);
		style_count = count_font_styles(f_family);
		for (style_loop = 0; style_loop < style_count; style_loop++) {
			get_font_style(f_family, style_loop, &f_style);
			msg = new BMessage(P_FONT);
			msg->AddString("font", f_family);
			msg->AddString("style", f_style);
			sprintf(label, "%s %s", f_family, f_style);
			menu->AddItem(item = new BMenuItem(label, msg));
			if ((!strcmp(def_family, f_family)) && (!strcmp(def_style, f_style)))
				item->SetMarked(true);
			item->SetTarget(this);
		}
	}
	return menu;
}

//--------------------------------------------------------------------

BPopUpMenu *TPrefsWindow::BuildLevelMenu(int32 level)
{
	BMenuItem	*item;
	BMessage	*msg;
	BPopUpMenu	*menu;

	menu = new BPopUpMenu("");
	msg = new BMessage(P_LEVEL);
	msg->AddInt32("level", L_BEGINNER);
	menu->AddItem(item = new BMenuItem("Beginner", msg));
	if (level == L_BEGINNER)
		item->SetMarked(true);

	msg = new BMessage(P_LEVEL);
	msg->AddInt32("level", L_EXPERT);
	menu->AddItem(item = new BMenuItem("Expert", msg));
	if (level == L_EXPERT)
		item->SetMarked(true);

	return menu;
}

//--------------------------------------------------------------------

BPopUpMenu *TPrefsWindow::BuildSignatureMenu(char *sig)
{
	char			name[B_FILE_NAME_LENGTH];
	int32			index = 0;
	BEntry			entry;
	BFile			file;
	BMenuItem		*item;
	BMessage		*msg;
	BPopUpMenu		*menu;
	BQuery			query;
	BVolume			vol;
	BVolumeRoster	volume;

	menu = new BPopUpMenu("");
	msg = new BMessage(P_SIG);
	menu->AddItem(item = new BMenuItem(SIG_NONE, msg));
	if (!strcmp(sig, SIG_NONE))
		item->SetMarked(true);
	menu->AddSeparatorItem();

	volume.GetBootVolume(&vol);
	query.SetVolume(&vol);
	query.SetPredicate("_signature = *");
	query.Fetch();

	while (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDONLY);
		if (file.InitCheck() == B_NO_ERROR) {
			msg = new BMessage(P_SIG);
			file.ReadAttr("_signature", B_STRING_TYPE, 0, name, sizeof(name));
			msg->AddString("signature", name);
			menu->AddItem(item = new BMenuItem(name, msg));
			if (!strcmp(sig, name))
				item->SetMarked(true);
		}
	}
	return menu;
}


//--------------------------------------------------------------------

BPopUpMenu *TPrefsWindow::BuildSizeMenu(BFont *font)
{
	char		label[16];
	int32		loop;
	int32		sizes[] = {9, 10, 12, 14, 18, 24};
	float		size;
	BMenuItem	*item;
	BMessage	*msg;
	BPopUpMenu	*menu;

	menu = new BPopUpMenu("");
	size = font->Size();
	for (loop = 0; loop < sizeof(sizes) / sizeof(int32); loop++) {
		msg = new BMessage(P_SIZE);
		msg->AddInt32("size", sizes[loop]);
		sprintf(label, "%d", sizes[loop]);
		menu->AddItem(item = new BMenuItem(label, msg));
		if (sizes[loop] == (int32)size)
			item->SetMarked(true);
	}
	return menu;
}

//--------------------------------------------------------------------

BPopUpMenu *TPrefsWindow::BuildWrapMenu(bool wrap)
{
	BMenuItem	*item;
	BMessage	*msg;
	BPopUpMenu	*menu;

	menu = new BPopUpMenu("");
	msg = new BMessage(P_WRAP);
	msg->AddBool("wrap", true);
	menu->AddItem(item = new BMenuItem("On", msg));
	if (wrap)
		item->SetMarked(true);

	msg = new BMessage(P_WRAP);
	msg->AddInt32("wrap", false);
	menu->AddItem(item = new BMenuItem("Off", msg));
	if (!wrap)
		item->SetMarked(true);

	return menu;
}
