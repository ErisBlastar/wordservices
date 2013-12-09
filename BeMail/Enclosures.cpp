//--------------------------------------------------------------------
//	
//	Enclosures.cpp
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
#include <Debug.h>

#include "Mail.h"
#include "Enclosures.h"


//====================================================================

TEnclosuresView::TEnclosuresView(BRect rect, BRect wind_rect)
				:BBox(rect, "m_enclosures", 
					  B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW)
{
	BFont		v_font = *be_plain_font;
	BRect		r;
	BScrollView	*scroll;
	rgb_color	c;

	fFocus = false;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);

	v_font.SetSize(FONT_SIZE);

	font_height fHeight;
	v_font.GetHeight(&fHeight);

	//fOffset = v_font.StringWidth("Enclosures: ") + 12;
	fOffset = 12;

	r.Set(fOffset, 
		  ENCLOSE_FIELD_V + fHeight.ascent + fHeight.descent,
		  wind_rect.right - wind_rect.left - B_V_SCROLL_BAR_WIDTH - 8,
		  ENCLOSE_FIELD_V + fHeight.ascent + fHeight.descent + ENCLOSE_FIELD_HEIGHT);
	fList = new TListView(r, this);
	fList->SetInvocationMessage(new BMessage(LIST_INVOKED));

	scroll = new BScrollView("", fList, B_FOLLOW_ALL, 0, false, true);
	AddChild(scroll);
	scroll->ScrollBar(B_VERTICAL)->SetRange(0, 0);
}

//--------------------------------------------------------------------

void TEnclosuresView::Draw(BRect where)
{
	float	offset;
	BFont	font = *be_plain_font;

	BBox::Draw(where);
	font.SetSize(FONT_SIZE);
	SetFont(&font);
	SetHighColor(0, 0, 0);
	SetLowColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);

	offset = 12;

	font_height fHeight;
	font.GetHeight(&fHeight);

	MovePenTo(12, ENCLOSE_TEXT_V + fHeight.ascent);
	DrawString(ENCLOSE_TEXT);
	if (!fFocus)
		SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
	StrokeLine(BPoint(offset, ENCLOSE_TEXT_V + fHeight.ascent + 2),
			   BPoint(offset + font.StringWidth(ENCLOSE_TEXT),
									  ENCLOSE_TEXT_V + fHeight.ascent + 2));
}

//--------------------------------------------------------------------

void TEnclosuresView::MessageReceived(BMessage *msg)
{
	bool		bad_type = false;
	TListItem	*data;
	short		loop;
	int32		index = 0;
	BListView	*list;
	BFile		file;
	BMessage	message(B_REFS_RECEIVED);
	BMessenger	*tracker;
	BRect		r;
	entry_ref	ref;
	entry_ref	*item;

	switch (msg->what) {
		case LIST_INVOKED:
			msg->FindPointer("source", &list);
			if (list) {
				data = (TListItem *) (list->ItemAt(msg->FindInt32("index")));
				if (data) {
					tracker = new BMessenger("application/x-vnd.Be-TRAK", -1, NULL);
					if (tracker->IsValid()) {
						message.AddRef("refs", data->Ref());
						tracker->SendMessage(&message);
					}
					delete tracker;
				}
			}
			break;

		case M_REMOVE:
			while ((index = fList->CurrentSelection()) >= 0) {
				data = (TListItem *) fList->ItemAt(index);
				fList->RemoveItem(index);
				free(data->Ref());
				free(data);
			}
			break;

		case M_SELECT:
			fList->Select(0, fList->CountItems() - 1, true);
			break;

		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		case REFS_RECEIVED:
			if (msg->HasRef("refs")) {
				while (msg->FindRef("refs", index++, &ref) == B_NO_ERROR) {
					file.SetTo(&ref, O_RDONLY);
					if ((file.InitCheck() == B_NO_ERROR) && (file.IsFile())) {
						for (loop = 0; loop < fList->CountItems(); loop++) {
							data = (TListItem *) fList->ItemAt(loop);
							if (ref == *(data->Ref())) {
								fList->Select(loop);
								fList->ScrollToSelection();
								goto next;
							}
						}
						item = new entry_ref(ref);
						fList->AddItem(new TListItem(item));
						fList->Select(fList->CountItems() - 1);
						fList->ScrollToSelection();
					}
					else
						bad_type = true;
next:;
				}
				if (bad_type) {
					beep();
					(new BAlert("", "Only files can be added as enclosures.",
								"OK"))->Go();
				}
			}
			break;

		default:
			BView::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

void TEnclosuresView::Focus(bool focus)
{
	BRect	r;

	if (fFocus != focus) {
		r = Frame();
		fFocus = focus;
		Draw(r);
	}
}


//====================================================================

TListView::TListView(BRect rect, TEnclosuresView *view)
		  :BListView(rect, "", B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL)
{
	fParent = view;
}

//--------------------------------------------------------------------

void TListView::AttachedToWindow(void)
{
	int32		offset = 0;
	BFont		font = *be_plain_font;
	entry_ref	ref;

	BListView::AttachedToWindow();
	font.SetSize(FONT_SIZE);
	SetFont(&font);
}

//--------------------------------------------------------------------

void TListView::MakeFocus(bool focus)
{
	BListView::MakeFocus(focus);
	fParent->Focus(focus);
}


//====================================================================

TListItem::TListItem(entry_ref *ref)
{
	fRef = ref;
}

//--------------------------------------------------------------------

void TListItem::Update(BView *owner, const BFont *finfo)
{
	BListItem::Update(owner, finfo);
	if (Height() < 17)
		SetHeight(17);
}

//--------------------------------------------------------------------

void TListItem::DrawItem(BView *owner, BRect r, bool /* complete */)
{
	BBitmap		*bitmap;
	BEntry		entry;
	BFile		file;
	BFont		font = *be_plain_font;
	BNodeInfo	*info;
	BPath		path;
	BRect		sr;
	BRect		dr;

	if (IsSelected()) {
		owner->SetHighColor(180, 180, 180);
		owner->SetLowColor(180, 180, 180);
	}
	else {
		owner->SetHighColor(255, 255, 255);
		owner->SetLowColor(255, 255, 255);
	}
	owner->FillRect(r);
	owner->SetHighColor(0, 0, 0);

	font.SetSize(FONT_SIZE);
	owner->SetFont(&font);
	owner->MovePenTo(r.left + 24, r.bottom - 4);

	entry.SetTo(fRef);
	if (entry.GetPath(&path) == B_NO_ERROR)
		owner->DrawString(path.Path());
	else
		owner->DrawString("<missing enclosure>");

	file.SetTo(fRef, O_RDONLY);
	if (file.InitCheck() == B_NO_ERROR) {
		info = new BNodeInfo(&file);
		sr.Set(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1);
		bitmap = new BBitmap(sr, B_COLOR_8_BIT);
		if (info->GetTrackerIcon(bitmap, B_MINI_ICON) == B_NO_ERROR) {
			dr.Set(r.left + 4, r.top + 1, r.left + 4 + 15, r.top + 1 + 15);
			owner->SetDrawingMode(B_OP_OVER);
			owner->DrawBitmap(bitmap, sr, dr);
			owner->SetDrawingMode(B_OP_COPY);
		}
		delete bitmap;
		delete info;
	}
}

//--------------------------------------------------------------------

entry_ref* TListItem::Ref()
{
	return fRef;
}
