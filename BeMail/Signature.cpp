//--------------------------------------------------------------------
//	
//	Signature.cpp
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
#include "Signature.h"

extern int32	level;
extern BRect	signature_window;


//====================================================================

TSignatureWindow::TSignatureWindow(BRect rect, BFont *font)
				 :BWindow(rect, "Signatures", B_TITLED_WINDOW, 0)
{
	float		height;
	BMenu		*menu;
	BMenuBar	*menu_bar;
	BMenuItem	*item;
	BRect		r;

	fFile = NULL;
	r.Set(0, 0, 32767, 15);
	menu_bar = new BMenuBar(r, "");
	menu = new BMenu("Signature");
	menu->AddItem(new BMenuItem("New", new BMessage(M_NEW), 'N'));
	fSignature = new TMenu("Open", INDEX_SIGNATURE, M_SIGNATURE);
	menu->AddItem(new BMenuItem(fSignature));
	menu->AddSeparatorItem();
	menu->AddItem(fSave = new BMenuItem("Save", new BMessage(M_SAVE), 'S'));
	menu->AddItem(fDelete = new BMenuItem("Delete", new BMessage(M_DELETE), 'T'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Close", new BMessage(B_CLOSE_REQUESTED), 'W'));
	menu_bar->AddItem(menu);

	menu = new BMenu("Edit");
	menu->AddItem(fUndo = new BMenuItem("Undo", new BMessage(B_UNDO), 'Z'));
	fUndo->SetTarget(NULL, this);
	menu->AddSeparatorItem();
	menu->AddItem(fCut = new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
	fCut->SetTarget(NULL, this);
	menu->AddItem(fCopy = new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
	fCopy->SetTarget(NULL, this);
	menu->AddItem(fPaste = new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
	fPaste->SetTarget(NULL, this);
	menu->AddItem(item = new BMenuItem("Select All",
								new BMessage(M_SELECT), 'A'));
	item->SetTarget(NULL, this);
	menu_bar->AddItem(menu);

	Lock();
	AddChild(menu_bar);
	height = menu_bar->Bounds().bottom + 1;
	Unlock();

	r.left--;
	r.top = height;
	r.right = rect.Width() + 2;
	r.bottom = rect.Height() + 2;
	fSigView = new TSignatureView(r, font);

	Lock();
	AddChild(fSigView);
	Unlock();
}

//--------------------------------------------------------------------

TSignatureWindow::~TSignatureWindow(void)
{
	signature_window = Frame();
}

//--------------------------------------------------------------------

void TSignatureWindow::FrameResized(float width, float height)
{
	fSigView->FrameResized(width, height);
}

//--------------------------------------------------------------------

void TSignatureWindow::MenusBeginning(void)
{
	int32		finish = 0;
	int32		start = 0;
	BTextView	*text_view;

	fDelete->SetEnabled(fFile);
	fSave->SetEnabled(IsDirty());
	fUndo->SetEnabled(false);		// ***TODO***

	text_view = (BTextView *)fSigView->fName->ChildAt(0);
	if (text_view->IsFocus())
		text_view->GetSelection(&start, &finish);
	else
		fSigView->fTextView->GetSelection(&start, &finish);

	fCut->SetEnabled(start != finish);
	fCopy->SetEnabled(start != finish);

	be_clipboard->Lock();
	fPaste->SetEnabled(be_clipboard->Data()->HasData("text/plain", B_MIME_TYPE));
	be_clipboard->Unlock();
}

//--------------------------------------------------------------------

void TSignatureWindow::MessageReceived(BMessage* msg)
{
	char		*sig;
	char		name[B_FILE_NAME_LENGTH];
	BFont		*font;
	BTextView	*text_view;
	entry_ref	ref;
	off_t		size;

	switch(msg->what) {
		case CHANGE_FONT:
			msg->FindPointer("font", &font);
			fSigView->fTextView->SetFontAndColor(font);
			fSigView->fTextView->Invalidate(fSigView->fTextView->Bounds());
			break;

		case M_NEW:
			if (Clear()) {
				fSigView->fName->SetText("");
				fSigView->fTextView->SetText(NULL, (int32)0);
				fSigView->fName->MakeFocus(true);
			}
			break;

		case M_SAVE:
			Save();
			break;

		case M_DELETE:
			if (level == L_BEGINNER) {
				beep();
				if (!(new BAlert("",
						"Are you sure you want to delete this signature?",
						"Cancel", "Delete", NULL, B_WIDTH_AS_USUAL,
						B_WARNING_ALERT))->Go())
					break;
			}
			if (fFile) {
				delete fFile;
				fFile = NULL;
				fEntry.Remove();
				fSigView->fName->SetText("");
				fSigView->fTextView->SetText(NULL, (int32)0);
				fSigView->fName->MakeFocus(true);
			}
			break;

		case M_SIGNATURE:
			if (Clear()) {
				msg->FindRef("ref", &ref);
				fEntry.SetTo(&ref);
				fFile = new BFile(&ref, O_RDWR);
				if (fFile->InitCheck() == B_NO_ERROR) {
					fFile->ReadAttr(INDEX_SIGNATURE, B_STRING_TYPE, 0, name, sizeof(name));
					fSigView->fName->SetText(name);
					fFile->GetSize(&size);
					sig = (char *)malloc(size);
					size = fFile->Read(sig, size);
					fSigView->fTextView->SetText(sig, size);
					fSigView->fName->MakeFocus(true);
					text_view = (BTextView *)fSigView->fName->ChildAt(0);
					text_view->Select(0, text_view->TextLength());
					fSigView->fTextView->fDirty = false;
				}
				else {
					fFile = NULL;
					beep();
					(new BAlert("", "An error occurred trying to open this signature.",
						"Sorry"))->Go();
				}
			}
			break;

		default:
			BWindow::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

bool TSignatureWindow::QuitRequested(void)
{
	BMessage	msg(WINDOW_CLOSED);

	if (Clear()) {
		msg.AddInt32("kind", SIG_WINDOW);
		be_app->PostMessage(&msg);
		return true;
	}
	else
		return false;
}

//--------------------------------------------------------------------

void TSignatureWindow::Show(void)
{
	BTextView	*text_view;

	Lock();
	text_view = (BTextView *)fSigView->fName->ChildAt(0);
	fSigView->fName->MakeFocus(true);
	text_view->Select(0, text_view->TextLength());
	Unlock();
	BWindow::Show();
}

//--------------------------------------------------------------------

bool TSignatureWindow::Clear(void)
{
	int32		result;

	if (IsDirty()) {
		beep();
		result = (new BAlert("", "Save changes to signature?",
				"Don't save", "Cancel", "Save", B_WIDTH_AS_USUAL,
				B_WARNING_ALERT))->Go();
		if (result == 1)
			return false;
		if (result == 2)
			Save();
	}

	delete fFile;
	fFile = NULL;
	fSigView->fTextView->fDirty = false;
	return true;
}

//--------------------------------------------------------------------

bool TSignatureWindow::IsDirty(void)
{
	char		name[B_FILE_NAME_LENGTH];

	if (fFile) {
		fFile->ReadAttr(INDEX_SIGNATURE, B_STRING_TYPE, 0, name, sizeof(name));
		if ((strcmp(name, fSigView->fName->Text())) || (fSigView->fTextView->fDirty))
			return true;
	}
	else {
		if ((strlen(fSigView->fName->Text())) ||
			(fSigView->fTextView->TextLength()))
			return true;
	}
	return false;
}

//--------------------------------------------------------------------

void TSignatureWindow::Save(void)
{
	char			name[B_FILE_NAME_LENGTH];
	int32			index = 0;
	status_t		result;
	BDirectory		dir;
	BEntry			entry;
	BNodeInfo		*node;
	BPath			path;
	BVolume			vol;
	BVolumeRoster	roster;

	roster.GetBootVolume(&vol);
	fs_create_index(vol.Device(), INDEX_SIGNATURE, B_STRING_TYPE, 0);

	if (!fFile) {
		find_directory(B_USER_SETTINGS_DIRECTORY, &path, true);
		dir.SetTo(path.Path());
		if (dir.FindEntry("bemail", &entry) == B_NO_ERROR)
			dir.SetTo(&entry);
		else
			dir.CreateDirectory("bemail", &dir);
		if (dir.InitCheck() != B_NO_ERROR)
			goto err_exit;

		if (dir.FindEntry("signatures", &entry) == B_NO_ERROR)
			dir.SetTo(&entry);
		else
			dir.CreateDirectory("signatures", &dir);
		if (dir.InitCheck() != B_NO_ERROR)
			goto err_exit;

		fFile = new BFile();
		while(1) {
			sprintf(name, "signature_%d", index++);
			if ((result = dir.CreateFile(name, fFile, true)) == B_NO_ERROR)
				break;
			if (result != EEXIST)
				goto err_exit;
		}
		dir.FindEntry(name, &fEntry);
		node = new BNodeInfo(fFile);
		node->SetType("text/plain");
		delete node;
	}

	fSigView->fTextView->fDirty = false;
	fFile->Seek(0, 0);
	fFile->Write(fSigView->fTextView->Text(),
				 fSigView->fTextView->TextLength());
	fFile->SetSize(fFile->Position());
	fFile->WriteAttr(INDEX_SIGNATURE, B_STRING_TYPE, 0, fSigView->fName->Text(),
					 strlen(fSigView->fName->Text()) + 1);
	return;

err_exit:
	beep();
	(new BAlert("", "An error occurred trying to save this signature.",
			"Sorry"))->Go();
}


//====================================================================

TSignatureView::TSignatureView(BRect rect, BFont *font)
			   :BBox(rect, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	BFont		v_font = *be_plain_font;
	BRect		r;
	BRect		text;
	BScrollView	*scroll;
	rgb_color	c;

	fFocus = false;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);

	v_font.SetSize(FONT_SIZE);
	fOffset = v_font.StringWidth(SIG_TEXT) + 12;

	r.Set(fOffset - v_font.StringWidth(NAME_TEXT) - 11, NAME_FIELD_V,
		  rect.Width() - SEPERATOR_MARGIN,
		  NAME_FIELD_V + NAME_FIELD_HEIGHT);
	fName = new TNameControl(r, NAME_TEXT, new BMessage(NAME_FIELD));
	AddChild(fName);

	r = rect;
	r.OffsetTo(0, 0);
	r.left = fOffset;
	r.right -= (B_V_SCROLL_BAR_WIDTH + 11);
	r.top = SIG_FIELD_V;
	r.bottom -= 12;
	text = r;
	text.OffsetTo(0, 0);

	fTextView = new TSigTextView(r, text, this, font);
	scroll = new BScrollView("", fTextView, B_FOLLOW_ALL, 0, false, true);
	AddChild(scroll);
}

//--------------------------------------------------------------------

void TSignatureView::Draw(BRect where)
{
	float	offset;
	BFont	font = *be_plain_font;

	BBox::Draw(where);

	font.SetSize(FONT_SIZE);
	SetFont(&font);
	SetHighColor(0, 0, 0);
	SetLowColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
	offset = fOffset - font.StringWidth(SIG_TEXT) - 8;
	MovePenTo(offset, SIG_TEXT_V);
	DrawString(SIG_TEXT);
	if (!fFocus)
		SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
	StrokeLine(BPoint(offset, SIG_TEXT_V + 2),
			   BPoint(offset + font.StringWidth(SIG_TEXT), SIG_TEXT_V + 2));
}

//--------------------------------------------------------------------

void TSignatureView::Focus(bool focus)
{
	BRect	r;

	if (fFocus != focus) {
		r = Frame();
		fFocus = focus;
		Draw(r);
	}
}

//--------------------------------------------------------------------

void TSignatureView::FrameResized(float /* width */, float /* height */)
{
	BRect	r;

	r = Bounds();
	r.left = fOffset;
	r.right -= (B_V_SCROLL_BAR_WIDTH + 11);
	r.top = SIG_FIELD_V;
	r.bottom -= 12;
	r.OffsetTo(0, 0);
	fTextView->SetTextRect(r);
}


//====================================================================

TNameControl::TNameControl(BRect rect, char *label, BMessage *msg)
			 :BTextControl(rect, "", label, "", msg, B_FOLLOW_NONE)
{
	strcpy(fLabel, label);
}

//--------------------------------------------------------------------

void TNameControl::AttachedToWindow(void)
{
	BFont		font = *be_plain_font;
	BTextView	*text;

	SetHighColor(0, 0, 0);
	BTextControl::AttachedToWindow();
	font.SetSize(FONT_SIZE);
	SetFont(&font);

	SetDivider(StringWidth(fLabel) + 6);
	text = (BTextView *)ChildAt(0);
	text->SetFontAndColor(&font);
	text->SetMaxBytes(B_FILE_NAME_LENGTH - 1);
}

//--------------------------------------------------------------------

void TNameControl::MessageReceived(BMessage *msg)
{
	BTextView	*text_view;

	switch (msg->what) {
		case M_SELECT:
			text_view = (BTextView *)ChildAt(0);
			text_view->Select(0, text_view->TextLength());
			break;

		default:
			BTextControl::MessageReceived(msg);
	}
}


//====================================================================

TSigTextView::TSigTextView(BRect frame, BRect text, TSignatureView *view,
						   BFont *font)
			 :BTextView(frame, "", text, B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE)
{
	fParent = view;
	fFont = *font;
	fDirty = false;
}

//--------------------------------------------------------------------

void TSigTextView::AttachedToWindow(void)
{
	BTextView::AttachedToWindow();
	SetFontAndColor(&fFont);
}

//--------------------------------------------------------------------

void TSigTextView::DeleteText(int32 offset, int32 len)
{
	fDirty = true;
	BTextView::DeleteText(offset, len);
}

//--------------------------------------------------------------------

void TSigTextView::InsertText(const char *text, int32 len, int32 offset,
							  const text_run_array *runs)
{
	fDirty = true;
	BTextView::InsertText(text, len, offset, runs);
}

//--------------------------------------------------------------------

void TSigTextView::KeyDown(const char *key, int32 count)
{
	bool	up = false;
	int32	height;
	BRect	r;

	switch (key[0]) {
		case B_HOME:
			Select(0, 0);
			ScrollToSelection();
			break;

		case B_END:
			Select(TextLength(), TextLength());
			ScrollToSelection();
			break;

		case B_PAGE_UP:
			up = true;
		case B_PAGE_DOWN:
			r = Bounds();
			height = (up ? r.top - r.bottom : r.bottom - r.top) - 25;
			if ((up) && (!r.top))
				break;
			ScrollBy(0, height);
			break;

		default:
			BTextView::KeyDown(key, count);
	}
}

//--------------------------------------------------------------------

void TSigTextView::MessageReceived(BMessage *msg)
{
	char		type[B_FILE_NAME_LENGTH];
	char		*text;
	int32		end;
	int32		start;
	BFile		file;
	BNodeInfo	*node;
	entry_ref	ref;
	off_t		size;

	switch (msg->what) {
		case B_SIMPLE_DATA:
			if (msg->HasRef("refs")) {
				msg->FindRef("refs", &ref);
				file.SetTo(&ref, O_RDONLY);
				if (file.InitCheck() == B_NO_ERROR) {
					node = new BNodeInfo(&file);
					node->GetType(type);
					delete node;
					file.GetSize(&size);
					if ((!strncmp(type, "text/", 5)) && (size)) {
						text = (char *)malloc(size);
						file.Read(text, size);
						Delete();
						GetSelection(&start, &end);
						Insert(text, size);
						Select(start, start + size);
						free(text);
					}
				}
			}
			else
				BTextView::MessageReceived(msg);
			break;

		case M_SELECT:
			if (IsSelectable())
				Select(0, TextLength());
			break;

		default:
			BTextView::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

void TSigTextView::MakeFocus(bool focus)
{
	BTextView::MakeFocus(focus);
	fParent->Focus(focus);
}
