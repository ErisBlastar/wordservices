//--------------------------------------------------------------------
//	
//	Content.cpp
//
//	Written by: Robert Polic
//	
//	Copyright 1997 - 1998 Be, Inc. All Rights Reserved.
//	
//--------------------------------------------------------------------

#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Mail.h"
#include "Content.h"
#include "Utilities.h"

extern	bool header_flag;


//====================================================================

TContentView::TContentView(BRect rect, bool incoming, BFile *file, BFont *font)
			 :BBox(rect, "m_content", B_FOLLOW_ALL, B_WILL_DRAW |
													B_FULL_UPDATE_ON_RESIZE)
{
	BFont		v_font = *be_plain_font;
	BRect		r;
	BRect		text;
	BScrollView	*scroll;
	rgb_color	c;

	fIncoming = incoming;
	fFile = file;
	fFocus = false;

	c.red = c.green = c.blue = VIEW_COLOR;
	SetViewColor(c);

	r = rect;
	r.OffsetTo(0, 0);
	v_font.SetSize(FONT_SIZE);
//	fOffset = v_font.StringWidth("Enclosures: ") + 12;
	fOffset = 12;

	font_height fHeight;
	v_font.GetHeight(&fHeight);

	r.left = fOffset;
	r.right -= (B_V_SCROLL_BAR_WIDTH + 11);
	r.top = MESSAGE_FIELD_V + fHeight.ascent + fHeight.descent;
	r.bottom -= 12;
	text = r;
	text.OffsetTo(0, 0);
	text.InsetBy(1, 1);

	fTextView = new TTextView(r, text, fIncoming, fFile, this, font);
	scroll = new BScrollView("", fTextView, B_FOLLOW_ALL, 0, false, true);
	AddChild(scroll);
}

//--------------------------------------------------------------------

void TContentView::Draw(BRect where)
{
	float	offset;
	BFont	font = *be_plain_font;

	BBox::Draw(where);
	if (fIncoming)
		SetHighColor(128, 128, 128);
	else
		SetHighColor(0, 0, 0);
	SetLowColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);

	font.SetSize(FONT_SIZE);
	SetFont(&font);
	offset = font.StringWidth("Enclosures: ") - font.StringWidth(MESSAGE_TEXT) + 4;

	font_height fHeight;
	font.GetHeight(&fHeight);

	MovePenTo(offset, MESSAGE_TEXT_V + fHeight.ascent);
	DrawString(MESSAGE_TEXT);
	if (!fFocus)
		SetHighColor(VIEW_COLOR, VIEW_COLOR, VIEW_COLOR);
	StrokeLine(BPoint(offset, MESSAGE_TEXT_V + fHeight.ascent + 2),
			   BPoint(offset + font.StringWidth(MESSAGE_TEXT), MESSAGE_TEXT_V + fHeight.ascent + 2));
}

//--------------------------------------------------------------------

void TContentView::MessageReceived(BMessage *msg)
{
	char		*str;
	char		*quote;
	const char	*text;
	char		new_line = '\n';
	int32		finish;
	int32		len;
	int32		loop;
	int32		new_start;
	int32		offset;
	int32		removed = 0;
	int32		start;
	BFile		file;
	BFont		*font;
	BRect		r;
	entry_ref	ref;
	off_t		size;

	switch (msg->what) {
		case CHANGE_FONT:
			msg->FindPointer("font", &font);
			fTextView->SetFontAndColor(0, LONG_MAX, font);
			fTextView->Invalidate(Bounds());
			break;

		case M_QUOTE:
			r = fTextView->Bounds();
			fTextView->GetSelection(&start, &finish);
			quote = (char *)malloc(strlen(QUOTE) + 1);
			strcpy(quote, QUOTE);
			len = strlen(QUOTE);
			fTextView->GoToLine(fTextView->CurrentLine());
			fTextView->GetSelection(&new_start, &new_start);
			fTextView->Select(new_start, finish);
			finish -= new_start;
			str = (char *)malloc(finish + 1);
			fTextView->GetText(new_start, finish, str);
			offset = 0;
			for (loop = 0; loop < finish; loop++) {
				if (str[loop] == '\n') {
					quote = (char *)realloc(quote, len + loop - offset + 1);
					memcpy(&quote[len], &str[offset], loop - offset + 1);
					len += loop - offset + 1;
					offset = loop + 1;
					if (offset < finish) {
						quote = (char *)realloc(quote, len + strlen(QUOTE));
						memcpy(&quote[len], QUOTE, strlen(QUOTE));
						len += strlen(QUOTE);
					}
				}
			}
			if (offset != finish) {
				quote = (char *)realloc(quote, len + (finish - offset));
				memcpy(&quote[len], &str[offset], finish - offset);
				len += finish - offset;
			}
			free(str);

			fTextView->Delete();
			fTextView->Insert(quote, len);
			if (start != new_start) {
				start += strlen(QUOTE);
				len -= (start - new_start);
			}
			fTextView->Select(start, start + len);
			fTextView->ScrollTo(r.LeftTop());
			free(quote);
			break;

		case M_REMOVE_QUOTE:
			r = fTextView->Bounds();
			fTextView->GetSelection(&start, &finish);
			len = start;
			fTextView->GoToLine(fTextView->CurrentLine());
			fTextView->GetSelection(&start, &start);
			fTextView->Select(start, finish);
			new_start = finish;
			finish -= start;
			str = (char *)malloc(finish + 1);
			fTextView->GetText(start, finish, str);
			for (loop = 0; loop < finish; loop++) {
				if (strncmp(&str[loop], QUOTE, strlen(QUOTE)) == 0) {
					finish -= strlen(QUOTE);
					memcpy(&str[loop], &str[loop + strlen(QUOTE)],
									finish - loop);
					removed += strlen(QUOTE);
				}
				while ((loop < finish) && (str[loop] != '\n')) {
					loop++;
				}
				if (loop == finish)
					break;
			}
			if (removed) {
				fTextView->Delete();
				fTextView->Insert(str, finish);
				new_start -= removed;
				fTextView->Select(new_start - finish + (len - start) - 1,
								  new_start);
			}
			else
				fTextView->Select(len, new_start);
			fTextView->ScrollTo(r.LeftTop());
			free(str);
			break;

		case M_SIGNATURE:
			msg->FindRef("ref", &ref);
			file.SetTo(&ref, O_RDWR);
			if (file.InitCheck() == B_NO_ERROR) {
				file.GetSize(&size);
				str = (char *)malloc(size);
				size = file.Read(str, size);
				fTextView->GetSelection(&start, &finish);
				text = fTextView->Text();
				len = fTextView->TextLength();
				if ((len) && (text[len - 1] != '\n')) {
					fTextView->Select(len, len);
					fTextView->Insert(&new_line, 1);
					len++;
				}
				fTextView->Select(len, len);
				fTextView->Insert(str, size);
				fTextView->Select(len, len + size);
				fTextView->ScrollToSelection();
				fTextView->Select(start, finish);
				fTextView->ScrollToSelection();
			}
			else {
				beep();
				(new BAlert("", "An error occurred trying to open this signature.",
					"Sorry"))->Go();
			}
			break;

		case M_FIND:
			FindString(msg->FindString("findthis"));
			break;

		case B_SIMPLE_DATA:
			fTextView->MessageReceived(msg);
			break;

		default:
			BBox::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

void TContentView::FindString(const char *str)
{
	int32	finish;
	int32	pass = 0;
	int32	start = 0;

	if (str == NULL)
		return;
	
//	Start from current selection or from the beginning of the pool
	
	char *text = (char *)fTextView->Text();
	int32 count = fTextView->TextLength();
	fTextView->GetSelection(&start, &finish);
	if (start != finish)
		start = finish;
	if (!count || text == NULL)
		return;
	
//	Do the find

	while (pass < 2) {
		long found = -1;
		char lc = tolower(str[0]);
		char uc = toupper(str[0]);
		for (long i = start; i < count; i++) {
			if (text[i] == lc || text[i] == uc) {
				const char *s = str;
				const char *t = text + i;
				while (*s && (tolower(*s) == tolower(*t))) {
					s++;
					t++;
				}
				if (*s == 0) {
					found = i;
					break;
				}
			}
		}
		
	//	Select the text if it worked
	
		if (found != -1) {
			Window()->Activate();
			fTextView->Select(found, found + strlen(str));
			fTextView->ScrollToSelection();
			fTextView->MakeFocus(true);
			return;
		}
		else if (start) {
			start = 0;
			text = (char *)fTextView->Text();
			count = fTextView->TextLength();
			pass++;
		}
		else {
			beep();
			return;
		}
	}
}

//--------------------------------------------------------------------

void TContentView::Focus(bool focus)
{
	BRect	r;

	if (fFocus != focus) {
		r = Frame();
		fFocus = focus;
		Draw(r);
	}
}

//--------------------------------------------------------------------

void TContentView::FrameResized(float /* width */, float /* height */)
{
	BRect	r;

	BFont v_font = *be_plain_font;
	v_font.SetSize(FONT_SIZE);

	font_height fHeight;
	v_font.GetHeight(&fHeight);

	r = Bounds();
	r.left = fOffset;
	r.right -= (B_V_SCROLL_BAR_WIDTH + 11);
	r.top = MESSAGE_FIELD_V + fHeight.ascent + fHeight.descent;
	r.bottom -= 12;
	r.OffsetTo(0, 0);
	fTextView->SetTextRect(r);
}


//====================================================================

// Word Services Begin
TTextView::TTextView(BRect frame, BRect text, bool incoming, BFile *file,
					  TContentView *view, BFont *font)
		  :WSTextView(frame, "", text, B_FOLLOW_ALL, B_WILL_DRAW |
													B_NAVIGABLE)
// Word Services End
{
	BFont	m_font = *be_plain_font;

	fIncoming = incoming;
	fFile = file;
	fParent = view;
	fFont = *font;
	fReady = false;
	fLastPosition = -1;
	fYankBuffer = NULL;
	fStopSem = create_sem(1, "reader_sem");
	fThread = NULL;
	fHeader = header_flag;
	fRaw = false;
	if (fIncoming)
		SetStylable(true);
	fEnclosures = new BList();
	fPanel = NULL;

	m_font.SetSize(10);
	fMenu = new BPopUpMenu("Enclosure", false, false);
	fMenu->SetFont(&m_font);
	fMenu->AddItem(new BMenuItem("Save"B_UTF8_ELLIPSIS, new BMessage(M_SAVE)));
	fMenu->AddItem(new BMenuItem("Open", new BMessage(M_ADD)));

	SetDoesUndo(true);
}

//--------------------------------------------------------------------

TTextView::~TTextView(void)
{
	ClearList();
	if (fPanel)
		delete fPanel;
	if (fYankBuffer)
		free(fYankBuffer);
	delete_sem(fStopSem);
}

//--------------------------------------------------------------------

void TTextView::AttachedToWindow(void)
{
	BTextView::AttachedToWindow();
	fFont.SetSpacing(B_FIXED_SPACING);
	SetFontAndColor(&fFont);
	if (fFile) {
		LoadMessage(fFile, false, false, NULL);
		if (fIncoming)
			MakeEditable(false);
	}
}

//--------------------------------------------------------------------

void TTextView::KeyDown(const char *key, int32 count)
{
	bool		up = false;
	char		new_line = '\n';
	char		raw;
	int32		end;
	int32		height;
	int32 		start;
	uint32		mods;
	BMessage	*msg;
	BRect		r;
	key_info	keys;

	msg = Window()->CurrentMessage();
	mods = msg->FindInt32("modifiers");

	switch (key[0]) {
		case B_UP_ARROW:
		case B_DOWN_ARROW:
			// if the key is not physically down, exit
			raw = msg->FindInt32("key");
			get_key_info(&keys);
			if (!(keys.key_states[raw>>3] & (1 << ((7 - raw) & 7))))
				break;

			if (IsEditable())
				BTextView::KeyDown(key, count);
			else {
				if ((Bounds().top) && (key[0] == B_UP_ARROW))
					ScrollBy(0, LineHeight() * -1);
				else if ((Bounds().bottom < CountLines() * LineHeight()) &&
						 (key[0] == B_DOWN_ARROW))
					ScrollBy(0, LineHeight());
			}
			break;

		case B_HOME:
			if (IsSelectable()) {
				if (mods & B_CONTROL_KEY)	// ^a - start of line
					GoToLine(CurrentLine());
				else {
					Select(0, 0);
					ScrollToSelection();
				}
			}
			break;

		case 0x02:						// ^b - back 1 char
			if (IsSelectable()) {
				GetSelection(&start, &end);
				start--;
				if (start >= 0) {
					Select(start, start);
					ScrollToSelection();
				}
			}
			break;

		case B_END:
		case B_DELETE:
			if (IsSelectable()) {
				if ((key[0] == B_DELETE) || (mods & B_CONTROL_KEY)) {	// ^d
					if (IsEditable()) {
						GetSelection(&start, &end);
						if (start != end)
							Delete();
						else {
							Select(start, start + 1);
							Delete();
						}
					}
				}
				else
					Select(TextLength(), TextLength());
				ScrollToSelection();
			}
			break;

		case 0x05:						// ^e - end of line
			if ((IsSelectable()) && (mods & B_CONTROL_KEY)) {
				if (CurrentLine() == CountLines() - 1)
					Select(TextLength(), TextLength());
				else {
					GoToLine(CurrentLine() + 1);
					GetSelection(&start, &end);
					Select(start - 1, start - 1);
				}
			}
			break;

		case 0x06:						// ^f - forward 1 char
			if (IsSelectable()) {
				GetSelection(&start, &end);
				if (end > start)
					start = end;
				Select(start + 1, start + 1);
				ScrollToSelection();
			}
			break;

		case 0x0e:						// ^n - next line
			if (IsSelectable()) {
				raw = B_DOWN_ARROW;
				BTextView::KeyDown(&raw, 1);
			}
			break;

		case 0x0f:						// ^o - open line
			if (IsEditable()) {
				GetSelection(&start, &end);
				Delete();
				Insert(&new_line, 1);
				Select(start, start);
				ScrollToSelection();
			}
			break;

		case B_PAGE_UP:
			if (mods & B_CONTROL_KEY) {	// ^k kill text from cursor to e-o-line
				if (IsEditable()) {
					GetSelection(&start, &end);
					if ((start != fLastPosition) && (fYankBuffer)) {
						free(fYankBuffer);
						fYankBuffer = NULL;
					}
					fLastPosition = start;
					if (CurrentLine() < (CountLines() - 1)) {
						GoToLine(CurrentLine() + 1);
						GetSelection(&end, &end);
						end--;
					}
					else
						end = TextLength();
					if (end < start)
						break;
					if (start == end)
						end++;
					Select(start, end);
					if (fYankBuffer) {
						fYankBuffer = (char *)realloc(fYankBuffer,
									 strlen(fYankBuffer) + (end - start) + 1);
						GetText(start, end - start,
							    &fYankBuffer[strlen(fYankBuffer)]);
					}
					else {
						fYankBuffer = (char *)malloc(end - start + 1);
						GetText(start, end - start, fYankBuffer);
					}
					Delete();
					ScrollToSelection();
				}
				break;
			}
			else
				up = true;
				// yes, fall through!

		case B_PAGE_DOWN:
			r = Bounds();
			height = (up ? r.top - r.bottom : r.bottom - r.top) - 25;
			if ((up) && (!r.top))
				break;
			else if ((!up) && (r.bottom >= CountLines() * LineHeight()))
				break;
			ScrollBy(0, height);
			break;

		case 0x10:						// ^p goto previous line
			if (IsSelectable()) {
				raw = B_UP_ARROW;
				BTextView::KeyDown(&raw, 1);
			}
			break;

		case 0x19:						// ^y yank text
			if ((IsEditable()) && (fYankBuffer)) {
				Delete();
				Insert(fYankBuffer);
				ScrollToSelection();
			}
			break;

		default:
			BTextView::KeyDown(key, count);
	}
}

//--------------------------------------------------------------------

void TTextView::MakeFocus(bool focus)
{
	// Word Services Begin
	//BTextView::MakeFocus(focus);
	WSTextView::MakeFocus(focus);	// probably should be inherited::
	// Word Services End
	fParent->Focus(focus);
}

//--------------------------------------------------------------------

void TTextView::MessageReceived(BMessage *msg)
{
	bool		inserted = false;
	bool		enclosure = false;
	char		type[B_FILE_NAME_LENGTH];
	char		*text;
	int32		end;
	int32		index = 0;
	int32		loop;
	int32		offset;
	int32		start;
	BFile		file;
	BMessage	message(REFS_RECEIVED);
	BNodeInfo	*node;
	entry_ref	ref;
	off_t		len = 0;
	off_t		size;

	switch (msg->what) {
		case B_SIMPLE_DATA:
			if (!fIncoming) {
				while (msg->FindRef("refs", index++, &ref) == B_NO_ERROR) {
					file.SetTo(&ref, O_RDONLY);
					if (file.InitCheck() == B_NO_ERROR) {
						node = new BNodeInfo(&file);
						node->GetType(type);
						delete node;
						file.GetSize(&size);
						if ((!strncmp(type, "text/", 5)) && (size)) {
							len += size;
							text = (char *)malloc(size);
							file.Read(text, size);
							if (!inserted) {
								GetSelection(&start, &end);
								Delete();
								inserted = true;
							}
							offset = 0;
							for (loop = 0; loop < size; loop++) {
								if (text[loop] == '\n') {
									Insert(&text[offset], loop - offset + 1);
									offset = loop + 1;
								}
								else if (text[loop] == '\r') {
									text[loop] = '\n';
									Insert(&text[offset], loop - offset + 1);
									if ((loop + 1 < size) && (text[loop + 1] == '\n'))
										loop++;
									offset = loop + 1;
								}
							}
							free(text);
						}
						else {
							enclosure = true;
							message.AddRef("refs", &ref);
						}
					}
				}
				if (inserted)
					Select(start, start + len);
				if (enclosure)
					Window()->PostMessage(&message, Window());
			}
			break;

		case M_HEADER:
			Window()->Unlock();
			StopLoad();
			Window()->Lock();
			msg->FindBool("header", &fHeader);
			SetText(NULL);
			LoadMessage(fFile, false, false, NULL);
			break;

		case M_RAW:
			Window()->Unlock();
			StopLoad();
			Window()->Lock();
			msg->FindBool("raw", &fRaw);
			SetText(NULL);
			LoadMessage(fFile, false, false, NULL);
			break;

		case M_SELECT:
			if (IsSelectable())
				Select(0, TextLength());
			break;

		case M_SAVE:
			Save(msg);
			break;

		default:
			BTextView::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

void TTextView::MouseDown(BPoint where)
{
	int32			items;
	int32			loop;
	int32			start;
	uint32			buttons;
	BMenuItem		*item;
	BPoint			point;
	bigtime_t		click = 0;
	hyper_text		*enclosure;

	if (Window()->CurrentMessage())
		Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
	start = OffsetAt(where);
	items = fEnclosures->CountItems();
	for (loop = 0; loop < items; loop++) {
		enclosure = (hyper_text *)fEnclosures->ItemAt(loop);
		if ((start >= enclosure->text_start) && (start < enclosure->text_end)) {
			Select(enclosure->text_start, enclosure->text_end);
			Window()->UpdateIfNeeded();
			if (buttons != B_SECONDARY_MOUSE_BUTTON) {
				get_click_speed(&click);
				click += system_time();
				point = where;
				while ((buttons) && (abs(point.x - where.x) < 4) &&
					   (abs(point.y - where.y) < 4) && (system_time() < click)) {
					snooze(10000);
					GetMouse(&point, &buttons);
				}
			}
			else
				point = where;
			if (system_time() >= click) {
				if ((enclosure->type != TYPE_ENCLOSURE) &&
					(enclosure->type != TYPE_BE_ENCLOSURE))
					return;
				ConvertToScreen(&point);
				item = fMenu->Go(point, true);
				if (item) {
					if (item->Message()->what == M_SAVE) {
						if (fPanel)
							fPanel->SetEnclosure(enclosure);
						else {
							fPanel = new TSavePanel(enclosure, this);
							fPanel->Window()->Show();
						}
						return;
					}
					point = where;
				}
				else
					return;
			}
			if ((abs(point.x - where.x) < 4) && (abs(point.y - where.y) < 4))
				Open(enclosure);
			return;
		}
	}
	BTextView::MouseDown(where);
}

//--------------------------------------------------------------------

void TTextView::MouseMoved(BPoint where, uint32 code, const BMessage *msg)
{
	int32			items;
	int32			loop;
	int32			start;
	hyper_text		*enclosure;

	start = OffsetAt(where);
	items = fEnclosures->CountItems();
	for (loop = 0; loop < items; loop++) {
		enclosure = (hyper_text *)fEnclosures->ItemAt(loop);
		if ((start >= enclosure->text_start) && (start < enclosure->text_end)) {
			be_app->SetCursor(B_HAND_CURSOR);
			return;
		}
	}
	BTextView::MouseMoved(where, code, msg);
}

//--------------------------------------------------------------------

void TTextView::ClearList(void)
{
	BEntry			entry;
	hyper_text		*enclosure;

	while (enclosure = (hyper_text *)fEnclosures->FirstItem()) {
		fEnclosures->RemoveItem(enclosure);
		if (enclosure->name)
			free(enclosure->name);
		if (enclosure->content_type)
			free(enclosure->content_type);
		if (enclosure->encoding)
			free(enclosure->encoding);
		if ((enclosure->have_ref) && (!enclosure->saved)) {
			entry.SetTo(&enclosure->ref);
			entry.Remove();
		}
		free(enclosure);
	}
}

//--------------------------------------------------------------------

void TTextView::LoadMessage(BFile *file, bool quote_it, bool close,
							const char *text)
{
	reader			*info;
	attr_info		a_info;

	ClearList();
	MakeSelectable(false);
	if (text)
		Insert(text, strlen(text));

	info = (reader *)malloc(sizeof(reader));
	info->header = fHeader;
	info->raw = fRaw;
	info->quote = quote_it;
	info->incoming = fIncoming;
	info->close = close;
	if (file->GetAttrInfo(B_MAIL_ATTR_MIME, &a_info) == B_NO_ERROR)
		info->mime = true;
	else
		info->mime = false;
	info->view = this;
	info->enclosures = fEnclosures;
	info->file = file;
	info->stop_sem = &fStopSem;
	resume_thread(fThread = spawn_thread((status_t (*)(void *)) Reader,
							   "reader", B_DISPLAY_PRIORITY, info));
}

//--------------------------------------------------------------------

void TTextView::Open(hyper_text *enclosure)
{
	char		name[B_FILE_NAME_LENGTH];
	char		name1[B_FILE_NAME_LENGTH];
	int32		index = 0;
	BDirectory	dir;
	BEntry		entry;
	BMessage	save(M_SAVE);
	BMessage	open(B_REFS_RECEIVED);
	BMessenger	*tracker;
	BPath		path;
	entry_ref	ref;
	status_t	result;

	switch (enclosure->type) {
		case TYPE_HTML:
		case TYPE_FTP:
			result = be_roster->Launch("text/html", 1, &enclosure->name);
			if ((result != B_NO_ERROR) && (result != B_ALREADY_RUNNING)) {
				beep();
				(new BAlert("", "There is no installed handler for 'text/html'.",
					"Sorry"))->Go();
			}
			break;

		case TYPE_MAILTO:
			be_roster->Launch(B_MAIL_TYPE, 1, &enclosure->name);
			break;

		case TYPE_ENCLOSURE:
		case TYPE_BE_ENCLOSURE:
			if (!enclosure->have_ref) {
				if (find_directory(B_COMMON_TEMP_DIRECTORY, &path) == B_NO_ERROR) {
					dir.SetTo(path.Path());
					if (dir.InitCheck() == B_NO_ERROR) {
						if (enclosure->name)
							sprintf(name1, "%s", enclosure->name);
						else
							sprintf(name1, "enclosure");
						strcpy(name, name1);
						while (dir.Contains(name)) {
							sprintf(name, "%s_%d", name1, index++);
						}
						entry.SetTo(path.Path());
						entry.GetRef(&ref);
						save.AddRef("directory", &ref);
						save.AddString("name", name);
						save.AddPointer("enclosure", enclosure);
						if (Save(&save) != B_NO_ERROR)
							break;
						enclosure->saved = false;
					}
				}
			}
			tracker = new BMessenger("application/x-vnd.Be-TRAK", -1, NULL);
			if (tracker->IsValid()) {
				open.AddRef("refs", &enclosure->ref);
				tracker->SendMessage(&open);
			}
			delete tracker;
			break;
	}
}

//--------------------------------------------------------------------

status_t TTextView::Reader(reader *info)
{
	char		*msg;
	int32		len;
	off_t		size;

	info->file->GetSize(&size);
	if ((msg = (char *)malloc(size)) == NULL)
		goto done;
	info->file->Seek(0, 0);
	size = info->file->Read(msg, size);

	len = header_len(info->file);
	if ((info->header) && (len)) {
		if (!strip_it(msg, len, info))
			goto done;
	}

	if ((info->raw) || (!info->mime)) {
		if (!strip_it(msg + len, size - len, info))
			goto done;
	}
	else if (!parse_header(msg, msg, size, NULL, info, NULL))
		goto done;

	if (get_semaphore(info->view->Window(), info->stop_sem)) {
		info->view->Select(0, 0);
		info->view->MakeSelectable(true);
		if (!info->incoming)
			info->view->MakeEditable(true);
		info->view->Window()->Unlock();
		release_sem(*(info->stop_sem));
	}

done:;
	if (info->close)
		delete info->file;
	free(info);
	if (msg)
		free(msg);
	return B_NO_ERROR;
}

//--------------------------------------------------------------------

status_t TTextView::Save(BMessage *msg)
{
	bool			is_text = false;
	const char		*name;
	char			*data;
	entry_ref		ref;
	BDirectory		dir;
	BEntry			entry;
	BFile			file;
	BNodeInfo		*info;
	hyper_text		*enclosure;
	ssize_t			size;
	status_t		result = B_NO_ERROR;
	char 			entry_name[B_FILE_NAME_LENGTH];
	msg->FindString("name", &name);
	msg->FindRef("directory", &ref);
	msg->FindPointer("enclosure", &enclosure);
	dir.SetTo(&ref);
	if (dir.InitCheck() == B_NO_ERROR) {
		if (dir.FindEntry(name, &entry) == B_NO_ERROR)
			entry.Remove();
		if ((enclosure->have_ref) && (!enclosure->saved)) {
			entry.SetTo(&enclosure->ref);

			/* Added true arg and entry_name so MoveTo clobbers as before.
			 * This may not be the correct behaviour, but
			 * it's the preserved behaviour.
			 */
			entry.GetName(entry_name);
			if (entry.MoveTo(&dir, entry_name, true) == B_NO_ERROR) {
				entry.Rename(name);
				entry.GetRef(&enclosure->ref);
				enclosure->saved = true;
				return result;
			}
		}
		if ((result = dir.CreateFile(name, &file)) == B_NO_ERROR) {
			if (enclosure->content_type) {
				info = new BNodeInfo(&file);
				info->SetType(enclosure->content_type);
				delete info;
			}
			data = (char *)malloc(enclosure->file_length);
			fFile->Seek(enclosure->file_offset, 0);
			size = fFile->Read(data, enclosure->file_length);
			if (enclosure->type == TYPE_BE_ENCLOSURE)
				SaveBeFile(&file, data, size);
			else {
				if ((enclosure->encoding) && (cistrstr(enclosure->encoding, "base64"))) {
					is_text = ((cistrstr(enclosure->content_type, "text")) &&
							  (!cistrstr(enclosure->content_type, B_MAIL_TYPE)));
					size = decode_base64(data, data, size, is_text);
				}
				file.Write(data, size);
			}
			free(data);
			dir.FindEntry(name, &entry);
			entry.GetRef(&enclosure->ref);
			enclosure->have_ref = true;
			enclosure->saved = true;
		}
		else {
			beep();
			(new BAlert("", "An error occurred trying to save the enclosure.",
				"Sorry"))->Go();
		}
	}
	return result;
}

//--------------------------------------------------------------------

void TTextView::SaveBeFile(BFile *file, char *data, ssize_t size)
{
	char		*boundary;
	char		*encoding;
	char		*name;
	char		*offset;
	char		*start;
	char		*type;
	int32		len;
	int32		index;
	BNodeInfo	*info;
	type_code	code;
	off_t		length;
	ssize_t		total;

	offset = data;
	total = size;
	while ((len = linelen(offset, (data + total) - offset, true)) <= 2) {
		offset += len;
		if (offset >= data + size)
			return;
		total -= len;
		data += len;
	}
	boundary = offset;
	boundary[len - 2] = 0;
	
	while (1) {
		if ((!(offset = find_boundary(offset, boundary, (data + total) - offset))) ||
			(offset[strlen(boundary) + 1] == '-'))
			return;

		while ((len = linelen(offset, (data + total) - offset, true)) > 2) {
			if (!cistrncmp(offset, CONTENT_TYPE, strlen(CONTENT_TYPE))) {
				offset[len - 2] = 0;
				type = offset;
			}
			else if (!cistrncmp(offset, CONTENT_ENCODING, strlen(CONTENT_ENCODING))) {
				offset[len - 2] = 0;
				encoding = offset + strlen(CONTENT_ENCODING);
			}
			offset += len;
			if (*offset == '\r')
				offset++;
			if (offset > data + total)
				return;
		}
		offset += len;

		start = offset;
		while ((offset < (data + total)) && strncmp(boundary, offset, strlen(boundary))) {
			offset += linelen(offset, (data + total) - offset, false);
			if (*offset == '\r')
				offset++;
		}
		len = offset - start;

		len = decode_base64(start, start, len, false);
		if (cistrstr(type, "x-be_attribute")) {
			index = 0;
			while (index < len) {
				name = &start[index];
				index += strlen(name) + 1;
				memcpy(&code, &start[index], sizeof(type_code));
				code = B_BENDIAN_TO_HOST_INT32(code);
				index += sizeof(type_code);
				memcpy(&length, &start[index], sizeof(length));
				length = B_BENDIAN_TO_HOST_INT64(length);
				index += sizeof(length);
				swap_data(code, &start[index], length, B_SWAP_BENDIAN_TO_HOST);
				file->WriteAttr(name, code, 0, &start[index], length);
				index += length;
			}
		}
		else {
			file->Write(start, len);
			info = new BNodeInfo(file);
			type += strlen(CONTENT_TYPE);
			start = type;
			while ((*start) && (*start != ';')) {
				start++;
			}
			*start = 0;
			info->SetType(type);
			delete info;
		}
	}
}

//--------------------------------------------------------------------

void TTextView::StopLoad(void)
{
	int32		result;
	thread_id	thread;
	thread_info	info;

	if ((thread = fThread) && (get_thread_info(fThread, &info) == B_NO_ERROR)) {
		acquire_sem(fStopSem);
		wait_for_thread(thread, &result);
		fThread = NULL;
		release_sem(fStopSem);
	}
}

//--------------------------------------------------------------------

void TTextView::AddAsContent(BMailMessage *mail, bool wrap)
{
	if ((mail == NULL) || (TextLength() < 1))
		return;
	
	const char	*text = Text();
	int32		textLen = TextLength();
	
	if (!wrap)
		mail->AddContent(text, textLen);
	else {
		BWindow	*window = Window();
		BRect	saveTextRect = TextRect();
		
		// do this before we start messing with the fonts
		// the user will never know...
		window->DisableUpdates();
		Hide();
		BScrollBar *vScroller = ScrollBar(B_VERTICAL);
		BScrollBar *hScroller = ScrollBar(B_HORIZONTAL);
		if (vScroller != NULL)
			vScroller->SetTarget((BView *)NULL);
		if (hScroller != NULL)
			hScroller->SetTarget((BView *)NULL);

		// temporarily set the font to be_fixed_font 				
		SetFontAndColor(0, textLen, be_fixed_font);
		
		// calculate a text rect that is 75 columns wide
		BRect newTextRect = saveTextRect;
		newTextRect.right = newTextRect.left + 
							(be_fixed_font->StringWidth("m") * 75);	
		SetTextRect(newTextRect);

		// hard-wrap, based on TextView's soft-wrapping
		int32	numLines = CountLines();
		char	*content = (char *)malloc(textLen + numLines);	// most we'll ever need
		if (content != NULL) {
			int32 contentLen = 0;

			for (int32 i = 0; i < numLines; i++) {
				int32 startOffset = OffsetAt(i);
				int32 endOffset = OffsetAt(i + 1);
				int32 lineLen = endOffset - startOffset;

				memcpy(content + contentLen, text + startOffset, lineLen);
				contentLen += lineLen;

				// add a newline to every line except for the ones
				// that already end in newlines, and the last line 
				if ((text[endOffset - 1] != '\n') && (i < (numLines - 1)))
					content[contentLen++] = '\n';
			}
			content[contentLen] = '\0';

			mail->AddContent(content, contentLen);
			free(content);
		}

		// reset the text rect and font			
		SetTextRect(saveTextRect);	
		SetFontAndColor(0, textLen, &fFont);

		// should be OK to hook these back up now
		if (vScroller != NULL)
			vScroller->SetTarget(this);
		if (hScroller != NULL)
			hScroller->SetTarget(this);				
		Show();
		window->EnableUpdates();
	}
}

//--------------------------------------------------------------------

bool get_semaphore(BWindow *window, sem_id *sem)
{
	if (!window->Lock())
		return false;
	if (acquire_sem_etc(*sem, 1, B_TIMEOUT, 0) != B_NO_ERROR) {
		window->Unlock();
		return false;
	}
	return true;
}

//--------------------------------------------------------------------

bool insert(reader *info, char *line, int32 count, bool hyper)
{
	uint32			mode;
	BFont			font;
	rgb_color		c;
	rgb_color		hyper_color = {0, 0, 255, 0};
	rgb_color		normal_color = {0, 0, 0, 0};
	text_run_array	style;

	info->view->GetFontAndColor(&font, &mode, &c);
	style.count = 1;
	style.runs[0].offset = 0;	
	style.runs[0].font = font;
	if (hyper)
		style.runs[0].color = hyper_color;
	else
		style.runs[0].color = normal_color;

	if (count) {
		if (!get_semaphore(info->view->Window(), info->stop_sem))
			return false;
		info->view->Insert(line, count, &style);
		info->view->Window()->Unlock();
		release_sem(*info->stop_sem);
	}
	return true;
}

//--------------------------------------------------------------------

bool parse_header(char *base, char *data, off_t size, char *boundary,
				  reader *info, off_t *processed)
{
	bool			is_bfile;
	bool			is_text;
	bool			result;
	char			*charset;
	char			*disposition;
	char			*encoding;
	char			*hyper;
	char			*new_boundary;
	char			*offset;
	char			*sjis;
	char			*start;
	char			*str;
	char			*type;
	char			*utf8;
	int32			dst_len;
	int32			index;
	int32			len;
	int32			saved_len;
	off_t			amount;
	hyper_text		*enclosure;

	offset = data;
	while (1) {
		is_bfile = false;
		is_text = true;
		charset = NULL;
		encoding = NULL;
		disposition = NULL;
		new_boundary = NULL;
		type = NULL;

		if (boundary) {
			if ((!(offset = find_boundary(offset, boundary, (data + size) - offset))) ||
				(offset[strlen(boundary) + 1] == '-')) {
				if (processed)
					*processed = offset - data;
				return true;
			}
		}

		while ((len = linelen(offset, (data + size) - offset, true)) > 2) {
			if (!cistrncmp(offset, CONTENT_TYPE, strlen(CONTENT_TYPE))) {
				offset[len - 2] = 0;
				type = offset;
				if (cistrstr(offset, MIME_TEXT)) {
				}
				else {
					is_text = false;
					if (cistrstr(offset, MIME_MULTIPART)) {
						if (cistrstr(offset, "x-bfile")) {
							is_bfile = true;
							start = offset + len;
							while ((start < (data + size)) && strncmp(boundary, start, strlen(boundary))) {
								index = linelen(start, (data + size) - start, true);
								start[index - 2] = 0;
								if ((!cistrncmp(start, CONTENT_TYPE, strlen(CONTENT_TYPE))) &&
									(!cistrstr(start, "x-be_attribute"))) {
									type = start;
									break;
								}
								else
									start[index - 2] = '\r';
								start += index;
								if (*start == '\r')
									start++;
								if (start > data + size)
									break;
							}
						}
						else if (get_parameter(offset, "boundary=", &offset[2])) {
							offset[0] = '-';
							offset[1] = '-';
							new_boundary = offset;
						}
					}
				}
			}
			else if (!cistrncmp(offset, CONTENT_ENCODING, strlen(CONTENT_ENCODING))) {
				offset[len - 2] = 0;
				encoding = offset + strlen(CONTENT_ENCODING);
			}
			else if (!cistrncmp(offset, CONTENT_DISPOSITION, strlen(CONTENT_DISPOSITION))) {
				offset[len - 2] = 0;
				disposition = offset + strlen(CONTENT_DISPOSITION);
			}
			offset += len;
			if (*offset == '\r')
				offset++;
			if (offset > data + size)
				return true;
		}
		offset += len;

		if (new_boundary) {
			if (!parse_header(base, offset, (data + size) - offset, new_boundary, info, &amount))
				return false;
			offset += amount;
		}
		else {
			if (boundary) {
				start = offset;
				while ((offset < (data + size)) && strncmp(boundary, offset, strlen(boundary))) {
					offset += linelen(offset, (data + size) - offset, false);
					if (*offset == '\r')
						offset++;
				}
				len = offset - start;
				offset = start;
			}
			else
				len = (data + size) - offset;
			if (((is_text) && (!type)) || ((is_text) && (type) && (!cistrstr(type, "name=")))) {
				utf8 = NULL;
				saved_len = 0;
				if ((type) && (get_parameter(type, "charset=", type))) {
					charset = type;
					if (!cistrncmp(charset, "iso-2022-jp", 11)) {
						saved_len = len;
						sjis = jis_to_sjis(offset, &len);
						utf8 = (char *)malloc(4 * len);
						dst_len = 4 * len;
						convert_to_utf8(B_SJIS_CONVERSION, sjis, &len, utf8, &dst_len);
						free(sjis);
						len = dst_len;
					}
					else if (!cistrncmp(charset, "iso-8859-1", 10)) {
						saved_len = len;
						utf8 = (char *)malloc(4 * len);
						dst_len = 4 * len;
						convert_to_utf8(B_ISO1_CONVERSION, offset, &len, utf8, &dst_len);
						len = dst_len;
					}
				}
				if ((encoding) && (cistrstr(encoding, "base64"))) {
					saved_len = len;
					len = decode_base64(offset, offset, len, true);
				}
				if (utf8) {
					result = strip_it(utf8, len, info);
					free(utf8);
					if (!result)
						return false;
				}
				else if (!strip_it(offset, len, info))
					return false;
				if (saved_len)
					len = saved_len;
				is_text = false;
			}
			else if (info->incoming) {
				if (type) {
					enclosure = (hyper_text *)malloc(sizeof(hyper_text));
					memset(enclosure, 0, sizeof(hyper_text));
					if (is_bfile)
						enclosure->type = TYPE_BE_ENCLOSURE;
					else
						enclosure->type = TYPE_ENCLOSURE;
					enclosure->content_type = (char *)malloc(strlen(type) + 1);
					if (encoding) {
						enclosure->encoding = (char *)malloc(strlen(encoding) + 1);
						strcpy(enclosure->encoding, encoding);
					}
					str = (char *)malloc(strlen(type));
					hyper = (char *)malloc(strlen(str) + 256);
					if (get_parameter(type, "name=", str)) {
						enclosure->name = (char *)malloc(strlen(str) + 1);
						strcpy(enclosure->name, str);
					}
					else if ((disposition) && (get_parameter(disposition, "name=", str))) {
						enclosure->name = (char *)malloc(strlen(str) + 1);
						strcpy(enclosure->name, str);
					}
					else
						sprintf(str, "???");
					index = 0;
					while ((type[index]) && (type[index] != ';')) {
						index++;
					}
					type[index] = 0;
					sprintf(hyper, "\n<Enclosure: %s (MIME type: %s)>\n",
								str, &type[strlen(CONTENT_TYPE)]);
					strcpy(enclosure->content_type, &type[strlen(CONTENT_TYPE)]);
					info->view->GetSelection(&enclosure->text_start,
											 &enclosure->text_end);
					enclosure->text_start++;
					enclosure->text_end += strlen(hyper) - 1;
					enclosure->file_offset = offset - base;
					enclosure->file_length = len;
					insert(info, hyper, strlen(hyper), true);
					free(hyper);
					free(str);
					info->enclosures->AddItem(enclosure);
				}
			}
			offset += len;
		}
		if (offset >= data + size)
			break;
	}
	if (processed)
		*processed = size;

	return true;
}

//--------------------------------------------------------------------

bool strip_it(char* data, int32 data_len, reader *info)
{
	bool			bracket = false;
	char			line[522];
	int32			count = 0;
	int32			index;
	int32			loop;
	int32			type;
	hyper_text		*enclosure;

	for (loop = 0; loop < data_len; loop++) {
		if ((info->quote) && ((!loop) || ((loop) && (data[loop - 1] == '\n')))) {
			strcpy(&line[count], QUOTE);
			count += strlen(QUOTE);
		}
		if ((!info->raw) && (loop) && (data[loop - 1] == '\n') && (data[loop] == '.'))
			continue;
		if ((!info->raw) && (info->incoming) && (loop < data_len - 7)) {
			type = 0;
			if (!cistrncmp(&data[loop], "http://", strlen("http://")))
				type = TYPE_HTML;
			else if (!cistrncmp(&data[loop], "ftp://", strlen("ftp://")))
				type = TYPE_FTP;
			else if (!cistrncmp(&data[loop], "mailto:", strlen("mailto:")))
				type = TYPE_MAILTO;
			if (type) {
				index = 0;
				while ((data[loop + index] != ' ') &&
					   (data[loop + index] != '\t') &&
					   (data[loop + index] != '>') &&
					   (data[loop + index] != ')') &&
					   (data[loop + index] != '"') &&
					   (data[loop + index] != '\'') &&
					   (data[loop + index] != '\r')) {
					index++;
				}

				if ((loop) && (data[loop - 1] == '<') && (data[loop + index] == '>')) {
					if (!insert(info, line, count - 1, false))
						return false;
					bracket = true;
				}
				else if (!insert(info, line, count, false))
					return false;
				count = 0;
				enclosure = (hyper_text *)malloc(sizeof(hyper_text));
				memset(enclosure, 0, sizeof(hyper_text));
				info->view->GetSelection(&enclosure->text_start,
										 &enclosure->text_end);
				enclosure->type = type;
				enclosure->name = (char *)malloc(index + 1);
				memcpy(enclosure->name, &data[loop], index);
				enclosure->name[index] = 0;
				if (bracket) {
					insert(info, &data[loop - 1], index + 2, true);
					enclosure->text_end += index + 2;
					bracket = false;
					loop += index;
				}
				else {
					insert(info, &data[loop], index, true);
					enclosure->text_end += index;
					loop += index - 1;
				}
				info->enclosures->AddItem(enclosure);
				continue;
			}
		}
		if ((!info->raw) && (info->mime) && (data[loop] == '=')) {
			if ((loop) && (loop < data_len - 1) && (data[loop + 1] == '\r'))
				loop += 2;
			else if ((loop < data_len - 2) && (isxdigit(data[loop + 1])) &&
										 (isxdigit(data[loop + 2]))) {
				data[loop] = data[loop + 1];
				data[loop + 1] = data[loop + 2];
				data[loop + 2] = 'x';
				line[count++] = strtol(&data[loop], NULL, 16);
				loop += 2;
			}
			else
				line[count++] = data[loop];
		}
		else if (data[loop] != '\r')
			line[count++] = data[loop];

		if ((count > 511) || ((count) && (loop == data_len - 1))) {
			if (!insert(info, line, count, false))
				return false;
			count = 0;
		}
	}
	return true;
}


//====================================================================

TSavePanel::TSavePanel(hyper_text *enclosure, TTextView *view)
		   :BFilePanel(B_SAVE_PANEL)
{
	fEnclosure = enclosure;
	fView = view;
	if (enclosure->name)
		SetSaveText(enclosure->name);
}

//--------------------------------------------------------------------

void TSavePanel::SendMessage(const BMessenger * /* messenger */, BMessage *msg)
{
	const char	*name = NULL;
	BMessage	save(M_SAVE);
	entry_ref	ref;

	if ((!msg->FindRef("directory", &ref)) && (!msg->FindString("name", &name))) {
		save.AddPointer("enclosure", fEnclosure);
		save.AddString("name", name);
		save.AddRef("directory", &ref);
		fView->Window()->PostMessage(&save, fView);
	}
}

//--------------------------------------------------------------------

void TSavePanel::SetEnclosure(hyper_text *enclosure)
{
	fEnclosure = enclosure;
	if (enclosure->name)
		SetSaveText(enclosure->name);
	else
		SetSaveText("");
	if (!IsShowing())
		Show();
	Window()->Activate();
}
