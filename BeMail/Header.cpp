//--------------------------------------------------------------------
//	
//	Header.cpp
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
#include <time.h>

#include <StringView.h>

#include "Mail.h"
#include "Header.h"
#include "Utilities.h"


//====================================================================

THeaderView::THeaderView(BRect rect, BRect wind_rect, bool incoming,
						 BFile *file, bool resending)
			:BBox(rect, "m_header", B_FOLLOW_NONE, B_WILL_DRAW)
{
	char		string[20];
	float		offset;
	BFont		font = *be_plain_font;
	BMenuField	*field;
	BRect		r;
	rgb_color	c;

	fIncoming = incoming;
	fResending = resending;
	fFile = file;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);
	font.SetSize(FONT_SIZE);
	SetFont(&font);
	offset = font.StringWidth("Enclosures: ") + 12;

	if ((fIncoming) && (!resending)) {
		r.Set(offset - font.StringWidth(FROM_TEXT) - 11, TO_FIELD_V,
			  wind_rect.Width() - SEPERATOR_MARGIN,
			  TO_FIELD_V + TO_FIELD_HEIGHT);
			sprintf(string, FROM_TEXT);
	}
	else {
		r.Set(offset - 11, TO_FIELD_V,
			  wind_rect.Width() - SEPERATOR_MARGIN,
			  TO_FIELD_V + TO_FIELD_HEIGHT);
		string[0] = 0;
	}
	fTo = new TTextControl(r, string, new BMessage(TO_FIELD),
											fIncoming, fFile, resending);
	AddChild(fTo);
	if ((!fIncoming) || (resending)) {
		r.right = r.left + 3;
		r.left = r.right - be_plain_font->StringWidth(TO_TEXT) - 20;
		r.top -= 1;
		fToMenu = new BPopUpMenu(TO_TEXT);
		fToMenu->SetRadioMode(false);
		field = new BMenuField(r, "", "", fToMenu, B_FOLLOW_LEFT | B_FOLLOW_TOP,
													B_WILL_DRAW);
		field->SetDivider(0.0);
		field->SetEnabled(true);
		AddChild(field);
	}

	r.Set(offset - font.StringWidth(SUBJECT_TEXT) - 11, SUBJECT_FIELD_V,
		  wind_rect.Width() - SEPERATOR_MARGIN,
		  SUBJECT_FIELD_V + TO_FIELD_HEIGHT);
	fSubject = new TTextControl(r, SUBJECT_TEXT, new BMessage(SUBJECT_FIELD),
											fIncoming, fFile, false);
	AddChild(fSubject);

	if (!fIncoming) {
		r.Set(offset - 11, CC_FIELD_V,
			  CC_FIELD_H + CC_FIELD_WIDTH,
			  CC_FIELD_V + CC_FIELD_HEIGHT);
		fCc = new TTextControl(r, "", new BMessage(CC_FIELD),
											fIncoming, fFile, false);
		AddChild(fCc);
		r.right = r.left + 3;
		r.left = r.right - be_plain_font->StringWidth(CC_TEXT) - 20;
		r.top -= 1;
		fCcMenu = new BPopUpMenu(CC_TEXT);
		fCcMenu->SetRadioMode(false);
		field = new BMenuField(r, "", "", fCcMenu, B_FOLLOW_LEFT | B_FOLLOW_TOP,
													B_WILL_DRAW);

		field->SetDivider(0.0);
		field->SetEnabled(true);
		AddChild(field);

		r.Set(BCC_FIELD_H + be_plain_font->StringWidth(BCC_TEXT), BCC_FIELD_V,
			  BCC_FIELD_H + BCC_FIELD_WIDTH + 1,
			  BCC_FIELD_V + BCC_FIELD_HEIGHT);
		fBcc = new TTextControl(r, "", new BMessage(BCC_FIELD),
											fIncoming, fFile, false);
		AddChild(fBcc);
		r.right = r.left + 3;
		r.left = r.right - be_plain_font->StringWidth(BCC_TEXT) - 20;
		r.top -= 1;
		fBccMenu = new BPopUpMenu(BCC_TEXT);
		fBccMenu->SetRadioMode(false);
		field = new BMenuField(r, "", "", fBccMenu, B_FOLLOW_LEFT | B_FOLLOW_TOP,
													B_WILL_DRAW);

		field->SetDivider(0.0);
		field->SetEnabled(true);
		AddChild(field);
	}
	else {
		const char	*kDateLabel = "Date:";
		char		dateStr[129] = "Unknown";
		char		theString[257] = "";
		int32		headerLen;
		attr_info	aInfo;

		headerLen = header_len(fFile);
		char *header = (char *)malloc(headerLen + 1);
		fFile->Seek(0, SEEK_SET);
		fFile->Read(header, headerLen);
		header[headerLen] = '\0';

		const char *dateHeader = strstr(header, "Date: ");
		if (dateHeader != NULL) {
			dateHeader += strlen("Date: ");

			int32 i = 0;
			while ((dateHeader[i] >= 32) || (dateHeader[i] == '\t'))
				i++;
			i = (i > 128) ? 128 : i;				

			memcpy(dateStr, dateHeader, i);
			dateStr[i] = '\0';
		}

		free(header);
		sprintf(theString, "%s  %s", kDateLabel, dateStr);

		r.left = offset - font.StringWidth(kDateLabel) - 10;
		r.OffsetBy(0.0, fSubject->Frame().Height() + 1);
		BStringView *stringView = new BStringView(r, "", theString);
		AddChild(stringView);	
		stringView->SetFont(&font);	
		stringView->SetHighColor(127, 127, 127);
	}
}

//--------------------------------------------------------------------

void THeaderView::MessageReceived(BMessage *msg)
{
	BMessage	*message;
	BTextView	*text_view;

	switch (msg->what) {
		case B_SIMPLE_DATA:
			text_view = (BTextView *)fTo->ChildAt(0);
			if (text_view->IsFocus())
				fTo->MessageReceived(msg);
			else if (!fIncoming) {
				text_view = (BTextView *)fCc->ChildAt(0);
				if (text_view->IsFocus())
					fCc->MessageReceived(msg);
				else {
					text_view = (BTextView *)fBcc->ChildAt(0);
					if (text_view->IsFocus())
						fBcc->MessageReceived(msg);
					else {
						message = new BMessage(msg);
						message->what = REFS_RECEIVED;
						Window()->PostMessage(message, Window());
						delete message;
					}
				}
			}
			break;
	}
}

//--------------------------------------------------------------------

void THeaderView::BuildMenus(void)
{
	char			*predicate;
	char			*name;
	char			*offset;
	char			*text;
	char			*text1;
	int32			count = 0;
	int32			groups = 0;
	int32			index;
	int32			index1;
	BEntry			entry;
	BFile			file;
	BMessage		*msg;
	BMenu			*sub1;
	BMenu			*sub2;
	BMenu			*sub3;
	BMenuItem		*item;
	BQuery			query;
	BQuery			query1;
	BVolume			vol;
	BVolumeRoster	volume;
	attr_info		info;

	while (item = fToMenu->ItemAt(0)) {
		fToMenu->RemoveItem(item);
		delete item;
	}
	if (!fResending) {
		while (item = fCcMenu->ItemAt(0)) {
			fCcMenu->RemoveItem(item);
			delete item;
		}
		while (item = fBccMenu->ItemAt(0)) {
			fBccMenu->RemoveItem(item);
			delete item;
		}
	}

	volume.GetBootVolume(&vol);
	query.SetVolume(&vol);
	query.SetPredicate("META:group=*");
	query.Fetch();

	while (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDONLY);
		if ((file.InitCheck() == B_NO_ERROR) &&
			(file.GetAttrInfo("META:group", &info) == B_NO_ERROR) &&
			(info.size > 1)) {
			text = (char *)malloc(info.size);
			text1 = text;
			file.ReadAttr("META:group", B_STRING_TYPE, 0, text, info.size);
			while (1) {
				if (offset = strstr(text, ","))
					*offset = 0;
				if (!fToMenu->FindItem(text)) {
					index = 0;
					while (item = fToMenu->ItemAt(index)) {
						if (strcmp(text, item->Label()) < 0)
							break;
						index++;
					}
					sub1 = new BMenu(text);
					sub1->SetFont(be_plain_font);
					msg = new BMessage(M_TO_MENU);
					msg->AddString("group", text);
					fToMenu->AddItem(new BMenuItem(sub1, msg), index);

					if (!fResending) {
						sub2 = new BMenu(text);
						sub2->SetFont(be_plain_font);
						msg = new BMessage(M_CC_MENU);
						msg->AddString("group", text);
						fCcMenu->AddItem(new BMenuItem(sub2, msg), index);
	
						sub3 = new BMenu(text);
						sub3->SetFont(be_plain_font);
						msg = new BMessage(M_BCC_MENU);
						msg->AddString("group", text);
						fBccMenu->AddItem(new BMenuItem(sub3, msg), index);
					}
					query1.Clear();
					query1.SetVolume(&vol);
					predicate = (char *)malloc(strlen("META:group='**'") + strlen(text) + 1);
					sprintf(predicate, "META:group='*%s*'", text);
					query1.SetPredicate(predicate);
					query1.Fetch();
					while (query1.GetNextEntry(&entry) == B_NO_ERROR) {
						file.SetTo(&entry, O_RDONLY);
						if ((file.InitCheck() == B_NO_ERROR) &&
							(file.GetAttrInfo("META:email", &info) == B_NO_ERROR) &&
							(info.size > 1)) {
							text = (char *)malloc(info.size);
							file.ReadAttr("META:email", B_STRING_TYPE, 0, text, info.size);
				
							file.GetAttrInfo("META:name", &info);
							name = (char *)malloc(info.size + strlen(text) + 4);
							file.ReadAttr("META:name", B_STRING_TYPE, 0, name, info.size);
							strcat(name, " (");
							strcat(name, text);
							strcat(name, ")");
				
							index1 = 0;
							while (item = sub1->ItemAt(index1)) {
								if (strcmp(name, item->Label()) < 0)
									break;
								index1++;
							}
				
							msg = new BMessage(M_TO_MENU);
							msg->AddString("to", text);
							sub1->AddItem(new BMenuItem(name, msg), index1);
				
							if (!fResending) {
								msg = new BMessage(M_CC_MENU);
								msg->AddString("cc", text);
								sub2->AddItem(new BMenuItem(name, msg), index1);
					
								msg = new BMessage(M_BCC_MENU);
								msg->AddString("bcc", text);
								sub3->AddItem(new BMenuItem(name, msg), index1);
							}
							free(text);
							free(name);
						}
					}
					groups++;
				}
				if (offset) {
					text = offset + 1;
					while (*text == ' ')
						text++;
				}
				else
					break;
			}
			free(text1);
		}
	}

	if (groups) {
		fToMenu->AddSeparatorItem();
		if (!fResending) {
			fCcMenu->AddSeparatorItem();
			fBccMenu->AddSeparatorItem();
		}
		groups++;
	}

	query.Clear();
	query.SetVolume(&vol);
	query.SetPredicate("META:email=*");
	query.Fetch();

	while (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDONLY);
		if ((file.InitCheck() == B_NO_ERROR) &&
			(file.GetAttrInfo("META:email", &info) == B_NO_ERROR) &&
			(info.size > 1)) {
			text = (char *)malloc(info.size);
			file.ReadAttr("META:email", B_STRING_TYPE, 0, text, info.size);

			file.GetAttrInfo("META:name", &info);
			name = (char *)malloc(info.size + strlen(text) + 4);
			file.ReadAttr("META:name", B_STRING_TYPE, 0, name, info.size);
			strcat(name, " (");
			strcat(name, text);
			strcat(name, ")");

			index = groups;
			while (item = fToMenu->ItemAt(index)) {
				if (strcmp(name, item->Label()) < 0)
					break;
				index++;
			}

			msg = new BMessage(M_TO_MENU);
			msg->AddString("to", text);
			fToMenu->AddItem(new BMenuItem(name, msg), index);

			if (!fResending) {
				msg = new BMessage(M_CC_MENU);
				msg->AddString("cc", text);
				fCcMenu->AddItem(new BMenuItem(name, msg), index);
	
				msg = new BMessage(M_BCC_MENU);
				msg->AddString("bcc", text);
				fBccMenu->AddItem(new BMenuItem(name, msg), index);
			}

			count++;
			free(text);
			free(name);
		}
	}

	if (!count) {
		fToMenu->AddItem(item = new BMenuItem("none", new BMessage(M_TO_MENU)));
		item->SetEnabled(false);
		if (!fResending) {
			fCcMenu->AddItem(item = new BMenuItem("none", new BMessage(M_CC_MENU)));
			item->SetEnabled(false);
			fBccMenu->AddItem(item = new BMenuItem("none", new BMessage(M_BCC_MENU)));
			item->SetEnabled(false);
		}
	}
}

//--------------------------------------------------------------------

void THeaderView::SetAddress(BMessage *msg)
{
	bool			group = false;
	char			*str;
	char			*predicate;
	int32			end;
	int32			start;
	BEntry			entry;
	BFile			file;
	BQuery			query;
	BTextView		*text;
	BVolume			vol;
	BVolumeRoster	volume;
	attr_info		info;

	switch (msg->what) {
		case M_TO_MENU:
			text = (BTextView *)fTo->ChildAt(0);
			if (msg->HasString("group")) {
				msg->FindString("group", &str);
				group = true;
			}
			else
				msg->FindString("to", &str);
			break;
		case M_CC_MENU:
			text = (BTextView *)fCc->ChildAt(0);
			if (msg->HasString("group")) {
				msg->FindString("group", &str);
				group = true;
			}
			else
				msg->FindString("cc", &str);
			break;
		case M_BCC_MENU:
			text = (BTextView *)fBcc->ChildAt(0);
			if (msg->HasString("group")) {
				msg->FindString("group", &str);
				group = true;
			}
			else
				msg->FindString("bcc", &str);
			break;
	}

	text->GetSelection(&start, &end);
	if (start != end)
		text->Delete();

	if (group) {
		volume.GetBootVolume(&vol);
		query.SetVolume(&vol);
		predicate = (char *)malloc(strlen(str) + strlen("META:group='**'") + 1);
		sprintf(predicate, "META:group='*%s*'", str);
		query.SetPredicate(predicate);
		query.Fetch();
		free(predicate);

		while (query.GetNextEntry(&entry) == B_NO_ERROR) {
			file.SetTo(&entry, O_RDONLY);
			if ((file.InitCheck() == B_NO_ERROR) &&
				(file.GetAttrInfo("META:email", &info) == B_NO_ERROR) &&
				(info.size > 1)) {
				str = (char *)malloc(info.size);
				file.ReadAttr("META:email", B_STRING_TYPE, 0, str, info.size);
				if (info.size > 1) {
					if (end = text->TextLength()) {
						text->Select(end, end);
						text->Insert(", ");
					}
					text->Insert(str);
				}
			}
		}
	}
	else {
		if (end = text->TextLength()) {
			text->Select(end, end);
			text->Insert(", ");
		}
		text->Insert(str);
	}

	text->Select(text->TextLength(), text->TextLength());
}


//====================================================================

// Word Services Begin
TTextControl::TTextControl(BRect rect, char *label, BMessage *msg,
						   bool incoming, BFile *file, bool resending)
			 :WSTextControl(rect, "", label, "", msg, B_FOLLOW_NONE)
// Word Services End
{
	strcpy(fLabel, label);
	fCommand = msg->what;
	fFile = file;
	fIncoming = incoming;
	fResending = resending;
}

//--------------------------------------------------------------------

void TTextControl::AttachedToWindow(void)
{
	char		*string;
	BFont		font = *be_plain_font;
	BTextView	*text;
	attr_info	info;

	SetHighColor(0, 0, 0);
	BTextControl::AttachedToWindow();
	font.SetSize(FONT_SIZE);
	SetFont(&font);

	SetDivider(StringWidth(fLabel) + 6);
	text = (BTextView *)ChildAt(0);
	text->SetFont(&font);

	if (fFile) {
		if ((fIncoming) && (!fResending))
			SetEnabled(false);
		switch (fCommand) {
			case SUBJECT_FIELD:
				if ((fFile->GetAttrInfo(B_MAIL_ATTR_SUBJECT, &info) == B_NO_ERROR) &&
					(info.size)) {
					string = (char *)malloc(info.size);
					fFile->ReadAttr(B_MAIL_ATTR_SUBJECT, B_STRING_TYPE, 0, string, info.size);
					SetText(string);
					free(string);
				}
				else
					SetText("");
				break;

			case TO_FIELD:
				if ((fFile->GetAttrInfo(B_MAIL_ATTR_FROM, &info) == B_NO_ERROR) &&
					(info.size)) {
					string = (char *)malloc(info.size);
					fFile->ReadAttr(B_MAIL_ATTR_FROM, B_STRING_TYPE, 0, string, info.size);
					SetText(string);
					free(string);
				}
				else
					SetText("");
				break;
		}
	}
}

//--------------------------------------------------------------------

void TTextControl::MessageReceived(BMessage *msg)
{
	bool		enclosure = false;
	char		type[B_FILE_NAME_LENGTH];
	char		separator[4] = ", ";
	char		*name;
	int32		index = 0;
	int32		len;
	entry_ref	ref;
	BFile		file;
	BMessage	message(REFS_RECEIVED);
	BNodeInfo	*node;
	BTextView	*text_view;
	attr_info	info;

	switch (msg->what) {
		case B_SIMPLE_DATA:
			if (!fIncoming) {
				while (msg->FindRef("refs", index++, &ref) == B_NO_ERROR) {
					file.SetTo(&ref, O_RDONLY);
					if (file.InitCheck() == B_NO_ERROR) {
						node = new BNodeInfo(&file);
						node->GetType(type);
						delete node;
						if ((fCommand != SUBJECT_FIELD) && (!strcmp(type, "application/x-person"))) {
							file.GetAttrInfo("META:email", &info);
							name = (char *)malloc(info.size);
							file.ReadAttr("META:email", B_STRING_TYPE, 0, name, info.size);
							if (strlen(name)) {
								text_view = (BTextView *)ChildAt(0);
								if (len = text_view->TextLength()) {
									text_view->Select(len, len);
									text_view->Insert(separator);
								}
								text_view->Insert(name);
							}
							free(name);
						}
						else {
							enclosure = true;
							message.AddRef("refs", &ref);
						}
					}
				}
				if (enclosure)
					Window()->PostMessage(&message, Window());
			}
			break;

		case M_SELECT:
			text_view = (BTextView *)ChildAt(0);
			text_view->Select(0, text_view->TextLength());
			break;

		default:
			BTextControl::MessageReceived(msg);
	}
}
