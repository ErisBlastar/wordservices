//--------------------------------------------------------------------
//	
//	Mail.cpp
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <Screen.h>

#include "Mail.h"
#include "Header.h"
#include "Content.h"
#include "Enclosures.h"
#include "Prefs.h"
#include "Signature.h"
#include "Status.h"
#include "FindWindow.h"
#include "Utilities.h"

// Word Services Begin
#include "ServiceMenu.h"
#include "WordServices.h"
#include "WSClientSession.h"
#include "SelectionClient.h"
#include "BrowserLauncher.h"
// Word Services End

const char *kUndoStrings[] = {
	"Can't Undo",
	"Undo Typing",
	"Undo Cut",
	"Undo Paste",
	"Undo Clear",
	"Undo Drop"
};
const char *kRedoStrings[] = {
	"Can't Redo",
	"Redo Typing",
	"Redo Cut",
	"Redo Paste",
	"Redo Clear",
	"Redo Drop"
};

bool		header_flag = false;
bool		wrap_mode = true;
char		*signature;
int32		level = L_BEGINNER;
entry_ref	open_dir;
BMessage	*print_settings = NULL;
BPoint		prefs_window;
BRect		signature_window;
BRect		mail_window;


//====================================================================

int main(void)
{	
	TMailApp	*myApp;

	myApp = new TMailApp();
	myApp->Run();

	delete myApp;
	return B_NO_ERROR;
}

//--------------------------------------------------------------------

TMailApp::TMailApp(void)
		 :BApplication("application/x-vnd.Be-MAIL")
{
	int32		len;
	float		size;
	BDirectory	dir;
	BEntry		entry;
	BFont		m_font = *be_plain_font;
	BPath		path;
	font_family	f_family;
	font_style	f_style;

	fWindowCount = 0;
	fFont = *be_plain_font;
	fFont.SetSize(FONT_SIZE);
	fPrefsWindow = NULL;
	fSigWindow = NULL;
	signature = (char *)malloc(strlen(SIG_NONE) + 1);
	strcpy(signature, SIG_NONE);
	mail_window.Set(0, 0, 0, 0);
	signature_window.Set(6, TITLE_BAR_HEIGHT, 6 + SIG_WIDTH, TITLE_BAR_HEIGHT + SIG_HEIGHT);
	prefs_window.Set(6, TITLE_BAR_HEIGHT);

	find_directory(B_USER_SETTINGS_DIRECTORY, &path, true);
	dir.SetTo(path.Path());
	if (dir.FindEntry("Mail_data", &entry) == B_NO_ERROR) {
		fPrefs = new BFile(&entry, O_RDWR);
		if (fPrefs->InitCheck() == B_NO_ERROR) {
			fPrefs->Read(&mail_window, sizeof(BRect));
			fPrefs->Read(&level, sizeof(level));
			fPrefs->Read(&f_family, sizeof(font_family));
			fPrefs->Read(&f_style, sizeof(font_style));
			fPrefs->Read(&size, sizeof(float));
			fPrefs->Read(&signature_window, sizeof(BRect));
			fPrefs->Read(&header_flag, sizeof(bool));
			fPrefs->Read(&wrap_mode, sizeof(bool));
			fPrefs->Read(&prefs_window, sizeof(BPoint));
			if (fPrefs->Read(&len, sizeof(int32)) > 0) {
				free(signature);
				signature = (char *)malloc(len);
				fPrefs->Read(signature, len);
			}

			if ((strlen(f_family)) && (strlen(f_style)))
				fFont.SetFamilyAndStyle(f_family, f_style);
			if (size >= 9)
				fFont.SetSize(size);
		}
		else {
			delete fPrefs;
			fPrefs = NULL;
		}
	}
	else {
		fPrefs = new BFile();
		if (dir.CreateFile("Mail_data", fPrefs) != B_NO_ERROR) {
			delete fPrefs;
			fPrefs = NULL;
		}
	}
	fFont.SetSpacing(B_BITMAP_SPACING);
}

//--------------------------------------------------------------------

TMailApp::~TMailApp(void)
{
	if (fPrefs)
		delete fPrefs;
}

//--------------------------------------------------------------------

void TMailApp::AboutRequested(void)
{
	(new BAlert("", B_UTF8_ELLIPSIS"by Robert Polic", "Big Deal"))->Go();
}

//--------------------------------------------------------------------

void TMailApp::ArgvReceived(int32 argc, char **argv)
{
	char		*addr;
	char		*names = NULL;
	int32		loop;
	entry_ref	ref;
	BEntry		entry;
	BMessage	msg(B_REFS_RECEIVED);

	for (loop = 1; loop < argc; loop++) {
		if (strncmp(argv[loop], "mailto:", 7) == 0) {
			addr = argv[loop] + 7;
			if (!names)
				names = strdup(addr);
			else {
				names = (char *)realloc(names, strlen(names) + 1 +
										strlen(addr) + 1 + 2);
				strcat(names, ", ");
				strcat(names, addr);
			}
		}
		else if (entry.SetTo(argv[loop]) == B_NO_ERROR) {
			entry.GetRef(&ref);
			msg.AddRef("refs", &ref);
			RefsReceived(&msg);
		}
	}

	if (names) {
		TMailWindow	*window = NewWindow(NULL, names);
		free(names);
		window->Show();
	}
}

//--------------------------------------------------------------------

void TMailApp::MessageReceived(BMessage* msg)
{
	bool			all = false;
	int32			size;
	int32			type;
	BFont			*font;
	BMenuItem		*item;
	BMessage		*message;
	entry_ref		ref;
	TMailWindow		*window = NULL;
	TMailWindow		*src_window;
	TPrefsWindow	*prefs;

	switch (msg->what) {
		case M_NEW:
			msg->FindInt32("type", &type);
			switch (type) {
				case M_NEW:
					window = NewWindow();
					break;

				case M_RESEND:
					msg->FindRef("ref", &ref);
					window = NewWindow(&ref, "", true);
					break;

				case M_FORWARD:
					msg->FindRef("ref", &ref);
					window = NewWindow();
					window->Lock();
					window->Forward(&ref);
					window->Unlock();
					break;

				case M_REPLY:
				case M_REPLY_ALL:
					msg->FindPointer("window", &src_window);
					if (!src_window->Lock())
						break;
					msg->FindRef("ref", &ref);
					window = NewWindow();
					window->Lock();
					window->Reply(&ref, src_window, type == M_REPLY_ALL);
					window->Unlock();
					src_window->Unlock();
					break;
			}
			if (window)
				window->Show();
			break;

		case M_WRAP_TEXT:
		{
			BMenuItem *item = NULL;
			if (msg->FindPointer("source", &item) == B_NO_ERROR) {
				wrap_mode = !wrap_mode;
				item->SetMarked(wrap_mode);
			}
			break;
		}

		case M_PREFS:
			if (fPrefsWindow)
				fPrefsWindow->Activate(true);
			else {
				fPrefsWindow = new TPrefsWindow(BRect(prefs_window.x, prefs_window.y,
					prefs_window.x + PREF_WIDTH, prefs_window.y + PREF_HEIGHT),
					&fFont, &level, &wrap_mode, &signature);
				fPrefsWindow->Show();
			}
			break;

		case M_EDIT_SIGNATURE:
			if (fSigWindow)
				fSigWindow->Activate(true);
			else {
				fSigWindow = new TSignatureWindow(signature_window, &fFont);
				fSigWindow->Show();
			}
			break;

		case M_FONT:
			FontChange();
			break;

		case M_BEGINNER:
		case M_EXPERT:
			level = msg->what - M_BEGINNER;
			break;

		case REFS_RECEIVED:
			if (msg->HasPointer("window")) {
				msg->FindPointer("window", &window);
				message = new BMessage(msg);
				window->PostMessage(message, window);
				delete message;
			}
			break;

		case WINDOW_CLOSED:
			switch (msg->FindInt32("kind")) {
				case MAIL_WINDOW:
					fWindowCount--;
					break;

				case PREFS_WINDOW:
					fPrefsWindow = NULL;
					break;

				case SIG_WINDOW:
					fSigWindow = NULL;
					break;
			}

			if ((!fWindowCount) && (!fSigWindow) && (!fPrefsWindow))
				be_app->PostMessage(B_QUIT_REQUESTED);
			break;

		case B_REFS_RECEIVED:
			RefsReceived(msg);
			break;

		default:
			BApplication::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

bool TMailApp::QuitRequested(void)
{
	int32		len;
	float		size;
	font_family	f_family;
	font_style	f_style;

	if (BApplication::QuitRequested()) {
		if (fPrefs) {
			fFont.GetFamilyAndStyle(&f_family, &f_style);
			size = fFont.Size();

			fPrefs->Seek(0, 0);
			fPrefs->Write(&mail_window, sizeof(BRect));
			fPrefs->Write(&level, sizeof(level));
			fPrefs->Write(&f_family, sizeof(font_family));
			fPrefs->Write(&f_style, sizeof(font_style));
			fPrefs->Write(&size, sizeof(float));
			fPrefs->Write(&signature_window, sizeof(BRect));
			fPrefs->Write(&header_flag, sizeof(bool));
			fPrefs->Write(&wrap_mode, sizeof(bool));
			fPrefs->Write(&prefs_window, sizeof(BPoint));
			len = strlen(signature) + 1;
			fPrefs->Write(&len, sizeof(int32));
			fPrefs->Write(signature, len);
		}
		return true;
	}
	else
		return false;
}

//--------------------------------------------------------------------

void TMailApp::ReadyToRun(void)
{
	TMailWindow	*window;

	if (!fWindowCount) {
		window = NewWindow();
		window->Show();
	}
}

//--------------------------------------------------------------------

void TMailApp::RefsReceived(BMessage *msg)
{
	bool		have_names = false;
	char		*name;
	char		*names = NULL;
	char		type[B_FILE_NAME_LENGTH];
	int32		item = 0;
	BFile		file;
	BNodeInfo	*node;
	TMailWindow	*window;
	entry_ref	ref;
	attr_info	info;

	while (msg->HasRef("refs", item)) {
		msg->FindRef("refs", item++, &ref);
		if (window = FindWindow(ref))
			window->Activate(true);
		else {
			file.SetTo(&ref, O_RDONLY);
			if (file.InitCheck() == B_NO_ERROR) {
				node = new BNodeInfo(&file);
				node->GetType(type);
				delete node;
				if (!strcmp(type, B_MAIL_TYPE)) {
					window = NewWindow(&ref);
					window->Show();
				}
				else if (!strcmp(type, "application/x-person")) {
					if (file.GetAttrInfo("META:email", &info) == B_NO_ERROR) {
						name = (char *)malloc(info.size);
						file.ReadAttr("META:email", B_STRING_TYPE, 0, name, info.size);
						if (strlen(name)) {
							if (!names) {
								names = (char *)malloc(strlen(name) + 1);
								strcpy(names, name);
							}
							else {
								names = (char *)realloc(names, strlen(names) + 1 + 1 + strlen(name));
								strcat(names, ", ");
								strcat(names, name);
							}
							have_names = true;
						}
						free(name);
					}
				}
			}
		}
	}

	if (have_names) {
		window = NewWindow(NULL, names);
		free(names);
		window->Show();
	}
}

//--------------------------------------------------------------------

TMailWindow* TMailApp::FindWindow(entry_ref ref)
{
	int32		index = 0;
	TMailWindow	*window;

	while (window = (TMailWindow *)WindowAt(index++)) {
		if ((window->FindView("m_header")) && (window->fRef) && (*(window->fRef) == ref))
			return window;
	}
	return NULL;
}

//--------------------------------------------------------------------

void TMailApp::FontChange(void)
{
	int32		index = 0;
	BMessage	msg;
	BWindow		*window;
	
	msg.what = CHANGE_FONT;
	msg.AddPointer("font", &fFont);

	for (;;) {
		window = WindowAt(index++);
		if (!window)
			break;
		window->PostMessage(&msg);
	}
}

//--------------------------------------------------------------------

TMailWindow* TMailApp::NewWindow(entry_ref *ref, char *to, bool resend)
{
	char			*str1;
	char			*str2;
	char			*title = NULL;
	BFile			file;
	BPoint			win_pos;
	BRect			r;
	TMailWindow		*window;
	attr_info		info;

	BScreen screen( B_MAIN_SCREEN_ID );
	BRect screen_frame = screen.Frame();

	if ((mail_window.Width()) && (mail_window.Height()))
		r = mail_window;
	else
		r.Set(6, TITLE_BAR_HEIGHT,
			  6 + WIND_WIDTH, TITLE_BAR_HEIGHT + WIND_HEIGHT);
	r.OffsetBy(fWindowCount * 20, fWindowCount * 20);
	if ((r.left - 6) < screen_frame.left)
		r.OffsetTo(screen_frame.left + 8, r.top);
	if ((r.left + 20) > screen_frame.right)
		r.OffsetTo(6, r.top);
	if ((r.top - 26) < screen_frame.top)
		r.OffsetTo(r.left, screen_frame.top + 26);
	if ((r.top + 20) > screen_frame.bottom)
		r.OffsetTo(r.left, TITLE_BAR_HEIGHT);
	if (r.Width() < WIND_WIDTH)
		r.right = r.left + WIND_WIDTH;
	fWindowCount++;

	if (ref) {
		file.SetTo(ref, O_RDONLY);
		if (file.InitCheck() == B_NO_ERROR) {
			if (file.GetAttrInfo(B_MAIL_ATTR_NAME, &info) == B_NO_ERROR) {
				str1 = (char *)malloc(info.size);
				file.ReadAttr(B_MAIL_ATTR_NAME, B_STRING_TYPE, 0, str1, info.size);
				if (file.GetAttrInfo(B_MAIL_ATTR_SUBJECT, &info) == B_NO_ERROR) {
					str2 = (char *)malloc(info.size);
					file.ReadAttr(B_MAIL_ATTR_SUBJECT, B_STRING_TYPE, 0, str2, info.size);
					title = (char *)malloc(strlen(str1) + strlen(str2) + 3);
					sprintf(title, "%s->%s", str1, str2);
					free(str1);
					free(str2);
				}
				else
					title = str1;
			}
		}
	}
	if (!title) {
		title = (char *)malloc(strlen("BeMail") + 1);
		sprintf(title, "BeMail");
	}
	window = new TMailWindow(r, title, ref, to, &fFont, resend);
	free(title);
	return window;
}


//====================================================================

TMailWindow::TMailWindow(BRect rect, char *title, entry_ref *ref, char *to,
						 BFont *font, bool resending)
			:BWindow(rect, title, B_TITLED_WINDOW, 0)
{
	bool		done = false;
	char		str[256];
	char		status[272];
	char		*header;
	char		*list;
	char		*recipients;
	int32		index = 0;
	int32		index1;
	int32		len;
	uint32		message;
	float		height;
	BMenu		*menu;
	BMenu		*sub_menu;
	BMenuBar	*menu_bar;
	BMenuItem	*item;
	BMessage	*msg;
	BRect		r;
	attr_info	info;

	fPanel = NULL;
	fZoom = rect;
	fDelete = false;
	fReplying = false;
	fResending = resending;
	fSent = false;
	fSigAdded = false;
	if (ref) {
		fRef = new entry_ref(*ref);
		fFile = new BFile(fRef, O_RDONLY);
		fIncoming = true;
	}
	else {
		fRef = NULL;
		fFile = NULL;
		fIncoming = false;
	}

	r.Set(0, 0, 32767, 15);
	menu_bar = new BMenuBar(r, "");
	menu = new BMenu("Message");

	msg = new BMessage(M_NEW);
	msg->AddInt32("type", M_NEW);
	menu->AddItem(item = new BMenuItem("New Mail Message", msg, 'N'));
	item->SetTarget(be_app);
	menu->AddSeparatorItem();
	
	if ((!resending) && (fIncoming)) {
		menu->AddItem(new BMenuItem("Reply to Sender",
						new BMessage(M_REPLY), 'R'));
		menu->AddItem(new BMenuItem("Reply to All",
						new BMessage(M_REPLY_ALL), 'R', B_SHIFT_KEY));
		menu->AddItem(new BMenuItem("Forward", new BMessage(M_FORWARD), 'J'));
		menu->AddItem(new BMenuItem("Resend", new BMessage(M_RESEND)));
		menu->AddItem(new BMenuItem("Move to Trash", new BMessage(M_DELETE), 'T'));
		menu->AddSeparatorItem();
		menu->AddItem(fHeader = new BMenuItem("Show Header",
								new BMessage(M_HEADER), 'H'));
		if (header_flag)
			fHeader->SetMarked(true);
		menu->AddItem(fRaw = new BMenuItem("Show Raw Message",
								new BMessage(M_RAW)));

		sub_menu = new BMenu("Save Address");
		recipients = (char *)malloc(1);
		recipients[0] = 0;
		len = header_len(fFile);
		header = (char *)malloc(len);
		fFile->Seek(0, 0);
		fFile->Read(header, len);
		get_recipients(&recipients, header, len, true);
		list = recipients;
		while (1) {
			if ((!list[index]) || (list[index] == ',')) {
				if (!list[index])
					done = true;
				else
					list[index] = 0;
				index1 = 0;
				while (item = sub_menu->ItemAt(index1)) {
					if (strcmp(list, item->Label()) == 0)
						goto skip;
					if (strcmp(list, item->Label()) < 0)
						break;
					index1++;
				}
				msg = new BMessage(M_SAVE);
				msg->AddString("address", list);
				sub_menu->AddItem(new BMenuItem(list, msg), index1);

skip:			if (!done) {
					list += index + 1;
					index = 0;
					while (*list) {
						if (*list != ' ')
							break;
						else
							list++;
					}
				}
				else
					break;
			}
			else
				index++;
		}
		free(header);
		free(recipients);
		menu->AddItem(sub_menu);
	}
	else {
		menu->AddItem(fSendNow = new BMenuItem("Send Now",
								new BMessage(M_SEND_NOW), 'M', B_SHIFT_KEY));
		menu->AddItem(fSendLater = new BMenuItem("Send Later",
								new BMessage(M_SEND_LATER), 'M'));
	}
	menu->AddSeparatorItem();
	menu->AddItem(fPrint = new BMenuItem("Page Setup"B_UTF8_ELLIPSIS,
								new BMessage(M_PRINT_SETUP)));
	menu->AddItem(fPrint = new BMenuItem("Print"B_UTF8_ELLIPSIS,
								new BMessage(M_PRINT), 'P'));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("About BeMail"B_UTF8_ELLIPSIS,
								new BMessage(B_ABOUT_REQUESTED)));
	item->SetTarget(be_app);
	menu->AddSeparatorItem();
	if ((!resending) && (fIncoming)) {
		sub_menu = new BMenu("Close");
		if (fFile->GetAttrInfo(B_MAIL_ATTR_STATUS, &info) == B_NO_ERROR)
			fFile->ReadAttr(B_MAIL_ATTR_STATUS, B_STRING_TYPE, 0, str, info.size);
		else
			str[0] = 0;
		if (!strcmp(str, "New")) {
			sub_menu->AddItem(item = new BMenuItem("Leave As 'New'",
							new BMessage(M_CLOSE_SAME), 'W', B_SHIFT_KEY));
			sub_menu->AddSeparatorItem();
			sub_menu->AddItem(item = new BMenuItem("Set To 'Read'",
							new BMessage(M_CLOSE_READ), 'W'));
			message = M_CLOSE_READ;
		}
		else {
			if (strlen(str))
				sprintf(status, "Leave As '%s'", str);
			else
				sprintf(status, "Leave same");
			sub_menu->AddItem(item = new BMenuItem(status,
							new BMessage(M_CLOSE_SAME), 'W'));
			message = M_CLOSE_SAME;
			AddShortcut('W', B_COMMAND_KEY | B_SHIFT_KEY, new BMessage(M_CLOSE_SAME));
			sub_menu->AddSeparatorItem();
		}
		sub_menu->AddItem(new BMenuItem("Set To 'Saved'",
							new BMessage(M_CLOSE_SAVED), 'W', B_CONTROL_KEY));
		sub_menu->AddItem(new BMenuItem(new TMenu("Set To"B_UTF8_ELLIPSIS, INDEX_STATUS, M_STATUS),
							new BMessage(M_CLOSE_CUSTOM)));
		menu->AddItem(sub_menu);
	}
	else
		menu->AddItem(new BMenuItem("Close",
								new BMessage(B_CLOSE_REQUESTED), 'W'));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Quit",
								new BMessage(B_QUIT_REQUESTED), 'Q'));
	item->SetTarget(be_app);
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
	menu->AddSeparatorItem();
	item->SetTarget(NULL, this);
	menu->AddItem(new BMenuItem("Find"B_UTF8_ELLIPSIS, new BMessage(M_FIND), 'F'));
	menu->AddItem(new BMenuItem("Find Again", new BMessage(M_FIND_AGAIN), 'G'));
	if (!fIncoming) {
		menu->AddSeparatorItem();
		menu->AddItem(fQuote = new BMenuItem("Quote", new BMessage(M_QUOTE),
								B_RIGHT_ARROW));
		menu->AddItem(fRemoveQuote = new BMenuItem("Remove Quote",
								new BMessage(M_REMOVE_QUOTE), B_LEFT_ARROW));
		fSignature = new TMenu("Add Signature", INDEX_SIGNATURE, M_SIGNATURE);
		menu->AddItem(new BMenuItem(fSignature));
	}
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Preferences"B_UTF8_ELLIPSIS,
								new BMessage(M_PREFS)));
	item->SetTarget(be_app);
	menu->AddItem(item = new BMenuItem("Signatures"B_UTF8_ELLIPSIS,
								new BMessage(M_EDIT_SIGNATURE)));
	item->SetTarget(be_app);
	menu_bar->AddItem(menu);

	if (!fIncoming) {
		menu = new BMenu("Enclosures");
		menu->AddItem(fAdd = new BMenuItem("Add"B_UTF8_ELLIPSIS, new BMessage(M_ADD), 'O'));
		menu->AddItem(fRemove = new BMenuItem("Remove",
									new BMessage(M_REMOVE), 'D'));
		menu_bar->AddItem(menu);
	}

	// Word Services Begin
	menu = new ServiceMenu( this );
	menu_bar->AddItem( menu );
	// Word Services End

	Lock();
	AddChild(menu_bar);
	height = menu_bar->Bounds().bottom + 1;
	Unlock();

	r.top = height;
	r.left -= 1;
	if (!fIncoming)
		r.bottom = height + HEADER_HEIGHT;
	else
		r.bottom = height + MIN_HEADER_HEIGHT;
	fHeaderView = new THeaderView(r, rect, fIncoming, fFile, resending);

	r = Frame();
	r.OffsetTo(0, 0);
	r.right = 32767;
	r.left -= 1;
//	if (fIncoming) {
		r.bottom += ENCLOSURES_HEIGHT;
		fEnclosuresView = NULL;
//	}
//	else {
//		r.bottom++;
//		r.top = r.bottom - ENCLOSURES_HEIGHT;
//		fEnclosuresView = new TEnclosuresView(r, rect);
//	}

	r.right = Frame().right - Frame().left + 2;
	if (fIncoming)
		r.top = height + MIN_HEADER_HEIGHT - 1;
	else
		r.top = height + HEADER_HEIGHT - 1;
	r.bottom -= ENCLOSURES_HEIGHT - 1;
	fContentView = new TContentView(r, fIncoming, fFile, font);

	Lock();
	AddChild(fHeaderView);
	AddChild(fContentView);
	if (fEnclosuresView)
		AddChild(fEnclosuresView);
	Unlock();

	if (to) {
		Lock();
		fHeaderView->fTo->SetText(to);
		Unlock();
	}

	SetSizeLimits(WIND_WIDTH, 32767,
				  HEADER_HEIGHT + ENCLOSURES_HEIGHT + height + 50, 32767);

	AddShortcut('n', B_COMMAND_KEY, new BMessage(M_NEW));
}

//--------------------------------------------------------------------

TMailWindow::~TMailWindow(void)
{
	BMessage	msg('Ttrs');
	BMessenger	*tracker;

	if (fFile) {
		if (fDelete) {
			tracker = new BMessenger("application/x-vnd.Be-TRAK", -1, NULL);
			if (tracker->IsValid()) {
				msg.AddRef("refs", fRef);
				tracker->SendMessage(&msg);
			}
			else
				(new BAlert("", "Need Tracker to move items to Trash.", "Sorry"))->Go();
		}
		delete fFile;
		delete fRef;
	}
	mail_window = Frame();

	if (fPanel)
		delete fPanel;
}

//--------------------------------------------------------------------

void TMailWindow::FrameResized(float width, float height)
{
	fContentView->FrameResized(width, height);
}

//--------------------------------------------------------------------

void TMailWindow::MenusBeginning(void)
{
	bool		enable;
	int32		finish = 0;
	int32		len = 0;
	int32		start = 0;
	BTextView	*text_view;

	if (!fIncoming) {
		enable = strlen(fHeaderView->fTo->Text()) ||
				 strlen(fHeaderView->fBcc->Text());
		fSendNow->SetEnabled(enable);
		fSendLater->SetEnabled(enable);

		be_clipboard->Lock();
		fPaste->SetEnabled(be_clipboard->Data()->HasData("text/plain", B_MIME_TYPE) &&
						   ((fEnclosuresView == NULL) || !fEnclosuresView->fList->IsFocus()));
		be_clipboard->Unlock();

		fQuote->SetEnabled(false);
		fRemoveQuote->SetEnabled(false);

		fAdd->SetEnabled(true);
		fRemove->SetEnabled((fEnclosuresView != NULL) && 
							(fEnclosuresView->fList->CurrentSelection() >= 0));
	}
	else {
		if (fResending) {
			enable = strlen(fHeaderView->fTo->Text());
			fSendNow->SetEnabled(enable);
			fSendLater->SetEnabled(enable);
			text_view = (BTextView *)fHeaderView->fTo->ChildAt(0);
			if (text_view->IsFocus()) {
				text_view->GetSelection(&start, &finish);
				fCut->SetEnabled(start != finish);
				be_clipboard->Lock();
				fPaste->SetEnabled(be_clipboard->Data()->HasData("text/plain", B_MIME_TYPE));
				be_clipboard->Unlock();
			}
			else {
				fCut->SetEnabled(false);
				fPaste->SetEnabled(false);
			}
		}
		else {
			fCut->SetEnabled(false);
			fPaste->SetEnabled(false);
		}
	}

	if ((!fIncoming) || (fResending))
		fHeaderView->BuildMenus();

	fPrint->SetEnabled(fContentView->fTextView->TextLength());

	text_view = (BTextView *)fHeaderView->fTo->ChildAt(0);
	if (text_view->IsFocus())
		text_view->GetSelection(&start, &finish);
	else {
		text_view = (BTextView *)fHeaderView->fSubject->ChildAt(0);
		if (text_view->IsFocus())
			text_view->GetSelection(&start, &finish);
		else {
			if (fContentView->fTextView->IsFocus()) {
				fContentView->fTextView->GetSelection(&start, &finish);
				if (!fIncoming) {
					fQuote->SetEnabled(true);
					fRemoveQuote->SetEnabled(true);
				}
			}
			else if (!fIncoming) {
				text_view = (BTextView *)fHeaderView->fCc->ChildAt(0);
				if (text_view->IsFocus())
					text_view->GetSelection(&start, &finish);
				else {
					text_view = (BTextView *)fHeaderView->fBcc->ChildAt(0);
					if (text_view->IsFocus())
						text_view->GetSelection(&start, &finish);
				}
			}
		}
	}

	fCopy->SetEnabled(start != finish);
	if (!fIncoming)
		fCut->SetEnabled(start != finish);

	// Undo stuff	
	bool		isRedo = false;
	undo_state	undoState = B_UNDO_UNAVAILABLE;

	BTextView *focusTextView = dynamic_cast<BTextView *>(CurrentFocus());
	if (focusTextView != NULL)
		undoState = focusTextView->UndoState(&isRedo);

	fUndo->SetLabel((isRedo) ? kRedoStrings[undoState] : kUndoStrings[undoState]);
	fUndo->SetEnabled(undoState != B_UNDO_UNAVAILABLE);
}

//--------------------------------------------------------------------

void TMailWindow::MessageReceived(BMessage* msg)
{
	bool			now = false;
	bool			raw;
	char			*arg;
	char			*str;
	int32			item = 0;
	BEntry			entry;
	BMenuItem		*menu;
	BMessage		*message;
	BMessage		open(B_REFS_RECEIVED);
	BMessenger		*me;
	BMessenger		*tracker;
	BQuery			query;
	BRect			r;
	BVolume			vol;
	BVolumeRoster	volume;
	entry_ref		ref;
	attr_info		info;
	status_t		result;

	switch(msg->what) {
		// Word Services Begin
		case BrowserLauncher::OpenURL:
			BrowserLauncher::GoToWebPage( msg );
			break;
			
		case ServiceMenu::msgStartBatchCheck:
			StartBatchCheck( msg );
			break;

		case kWSTerminateBatchCheck:
			HandleCheckTermination();
			break;
		// Word Services End

		case LIST_INVOKED:
			PostMessage(msg, fEnclosuresView);
			break;

		case CHANGE_FONT:
			PostMessage(msg, fContentView);
			break;

		case M_NEW:
			message = new BMessage(M_NEW);
			message->AddInt32("type", msg->what);
			be_app->PostMessage(message);
			delete message;
			break;

		case M_REPLY:
		case M_REPLY_ALL:
		case M_FORWARD:
		case M_RESEND:
			message = new BMessage(M_NEW);
			message->AddRef("ref", fRef);
			message->AddPointer("window", this);
			message->AddInt32("type", msg->what);
			be_app->PostMessage(message);
			delete message;
			break;

		case M_DELETE:
			if (level == L_BEGINNER) {
				beep();
				if (!(new BAlert("",
						"Are you sure you want to move this message to the trash?",
						"Cancel", "Trash", NULL, B_WIDTH_AS_USUAL,
						B_WARNING_ALERT))->Go())
					break;
			}
			fDelete = true;
			PostMessage(message = new BMessage(B_CLOSE_REQUESTED));
			delete message;
			break;

		case M_CLOSE_READ:
			message = new BMessage(B_CLOSE_REQUESTED);
			message->AddString("status", "Read");
			PostMessage(message);
			delete message;
			break;

		case M_CLOSE_SAVED:
			message = new BMessage(B_CLOSE_REQUESTED);
			message->AddString("status", "Saved");
			PostMessage(message);
			delete message;
			break;

		case M_CLOSE_SAME:
			message = new BMessage(B_CLOSE_REQUESTED);
			message->AddString("status", "");
			message->AddString("same", "");
			PostMessage(message);
			delete message;
			break;

		case M_CLOSE_CUSTOM:
			if (msg->HasString("status")) {
				msg->FindString("status", &str);
				message = new BMessage(B_CLOSE_REQUESTED);
				message->AddString("status", str);
				PostMessage(message);
				delete message;
			}
			else {
				r = Frame();
				r.left += ((r.Width() - STATUS_WIDTH) / 2);
				r.right = r.left + STATUS_WIDTH;
				r.top += 40;
				r.bottom = r.top + STATUS_HEIGHT;
				if (fFile->GetAttrInfo(B_MAIL_ATTR_STATUS, &info) == B_NO_ERROR) {
					str = (char *)malloc(info.size);
					fFile->ReadAttr(B_MAIL_ATTR_STATUS, B_STRING_TYPE, 0, str, info.size);
				}
				else {
					str = (char *)malloc(1);
					str[0] = 0;
				}
				new TStatusWindow(r, this, str);
				free(str);
			}
			break;

		case M_STATUS:
			msg->FindPointer("source", &menu);
			message = new BMessage(B_CLOSE_REQUESTED);
			message->AddString("status", menu->Label());
			PostMessage(message);
			delete message;
			break;

		case M_HEADER:
			header_flag = !(fHeader->IsMarked());
			fHeader->SetMarked(header_flag);
			message = new BMessage(M_HEADER);
			message->AddBool("header", header_flag);
			PostMessage(message, fContentView->fTextView);
			delete message;
			break;

		case M_RAW:
			raw = !(fRaw->IsMarked());
			fRaw->SetMarked(raw);
			message = new BMessage(M_RAW);
			message->AddBool("raw", raw);
			PostMessage(message, fContentView->fTextView);
			delete message;
			break;

		case M_SEND_NOW:
			now = true;
			// yes, we are suppose to fall through
		case M_SEND_LATER:
			Send(now);
			break;

		case M_SAVE:
			if (msg->FindString("address", &str) == B_NO_ERROR) {
				arg = (char *)malloc(strlen("META:email ") + strlen(str) + 1);
				volume.GetBootVolume(&vol);
				query.SetVolume(&vol);
				sprintf(arg, "META:email=%s", str);
				query.SetPredicate(arg);
				query.Fetch();
				if (query.GetNextEntry(&entry) == B_NO_ERROR) {
					tracker = new BMessenger("application/x-vnd.Be-TRAK", -1, NULL);
					if (tracker->IsValid()) {
						entry.GetRef(&ref);
						open.AddRef("refs", &ref);
						tracker->SendMessage(&open);
					}
					delete tracker;
				}
				else {
					sprintf(arg, "META:email %s", str);
					result = be_roster->Launch("application/x-person", 1, &arg);
					if (result != B_NO_ERROR)
						(new BAlert("", "Sorry, could not find an application that supports the 'Person' data type.", "OK"))->Go();
				}
				free(arg);
			}
			break;

		case M_PRINT_SETUP:
			PrintSetup();
			break;

		case M_PRINT:
			Print();
			break;

		case M_SELECT:
			break;

		case M_FIND:
		case M_FIND_AGAIN:
			if (FindWindow::mFindWindow == NULL) {
				new FindWindow();
				FindWindow::mFindWindow->Show();
			} else
				if (msg->what == M_FIND_AGAIN)
					FindWindow::mFindWindow->FindAgain();
				else
					FindWindow::mFindWindow->Activate();
			FindWindow::mFindWindow->WindowGuess(this);
			break;

		case M_QUOTE:
		case M_REMOVE_QUOTE:
			PostMessage(msg->what, fContentView);
			break;

		case M_SIGNATURE:
			message = new BMessage(msg);
			PostMessage(message, fContentView);
			delete message;
			fSigAdded = true;
			break;

		case M_ADD:
			if (!fPanel) {
				me = new BMessenger(this);
				fPanel = new BFilePanel(B_OPEN_PANEL, me, &open_dir, 
								   false, true, &BMessage(REFS_RECEIVED));
				delete me;
			}
			else if (!fPanel->Window()->IsHidden())
					fPanel->Window()->Activate();

			if (fPanel->Window()->IsHidden())
				fPanel->Window()->Show();
			break;

		case M_REMOVE:
			PostMessage(msg->what, fEnclosuresView);
			break;

		case M_TO_MENU:
		case M_CC_MENU:
		case M_BCC_MENU:
			fHeaderView->SetAddress(msg);
			break;

		case REFS_RECEIVED:
			if ((fEnclosuresView == NULL) && (!fIncoming)) {
				BRect r = Frame();
				r.OffsetTo(0, 0);
				r.right = 32767;
				r.left -= 1;
				r.bottom++;
				r.top = r.bottom - ENCLOSURES_HEIGHT;

				fContentView->ResizeBy(0.0, -r.Height() + 1.0);

				fEnclosuresView = new TEnclosuresView(r, Frame());
				AddChild(fEnclosuresView);
			}

			if ((fEnclosuresView) && (msg->HasRef("refs"))) {
				PostMessage(msg, fEnclosuresView);

				msg->FindRef("refs", &ref);
				entry.SetTo(&ref);
				entry.GetParent(&entry);
				entry.GetRef(&open_dir);
			}
			break;

		default:
			BWindow::MessageReceived(msg);
	}
}

//--------------------------------------------------------------------

bool TMailWindow::QuitRequested(void)
{
	char		status[256];
	const char	*str = NULL;
	int32		result;
	BFile		file;
	BMessage	message(WINDOW_CLOSED);
	attr_info	info;

	if ((level == L_BEGINNER) && ((!fIncoming) ||
		((fIncoming) && (fResending))) && (!fSent) &&
					((strlen(fHeaderView->fTo->Text())) ||
					(strlen(fHeaderView->fSubject->Text())) ||
					(strlen(fHeaderView->fCc->Text())) ||
					(strlen(fHeaderView->fBcc->Text())) ||
					(strlen(fContentView->fTextView->Text())) ||
					(fEnclosuresView != NULL &&
					(fEnclosuresView->fList->CountItems())))) {
		result = (new BAlert("",
			"Close this message without sending?",
			"Send", "Cancel", "Close", B_WIDTH_AS_USUAL,
			B_WARNING_ALERT))->Go();
		switch (result) {
			case 0:	//send
				if (Send(false) == B_NO_ERROR) {
					(new BAlert("", "The message has been queued to be sent", "OK"))->Go();
				}
				return false;
			case 1:	//cancel
				return false;
			case 2:	//OK
				break;
		}
	}
		
	message.AddInt32("kind", MAIL_WINDOW);
	be_app->PostMessage(&message);

	if ((CurrentMessage()) && (CurrentMessage()->HasString("status"))) {
		if (!CurrentMessage()->HasString("same"))
			str = CurrentMessage()->FindString("status");
	}
	else if (fFile) {
		if (fFile->GetAttrInfo(B_MAIL_ATTR_STATUS, &info) == B_NO_ERROR) {
			fFile->ReadAttr(B_MAIL_ATTR_STATUS, B_STRING_TYPE, 0, status, info.size);
			if (strcmp(status, "New") == 0)
				strcpy(status, "Read");
			str = status;
		}
	}
	if (str) {
		file.SetTo(fRef, O_RDWR);
		if (file.InitCheck() == B_NO_ERROR) {
			file.RemoveAttr(B_MAIL_ATTR_STATUS);
			file.WriteAttr(B_MAIL_ATTR_STATUS, B_STRING_TYPE, 0, str, strlen(str) + 1);
		}
	}
	return true;
}

//--------------------------------------------------------------------

void TMailWindow::Show(void)
{
	BTextView	*text_view;

	Lock();
	if ((!fResending) && ((fIncoming) || (fReplying)))
		fContentView->fTextView->MakeFocus(true);
	else {
		text_view = (BTextView *)fHeaderView->fTo->ChildAt(0);
		fHeaderView->fTo->MakeFocus(true);
		text_view->Select(0, text_view->TextLength());
	}
	Unlock();
	BWindow::Show();
}

//--------------------------------------------------------------------

void TMailWindow::Zoom(BPoint pos, float x, float y)
{
	float		height;
	float		width;
	BScreen		screen( this );
	BRect		r;
	BRect		s_frame = screen.Frame();

	r = Frame();
	width = 80 * ((TMailApp*)be_app)->fFont.StringWidth("M") +
			(r.Width() - fContentView->fTextView->Bounds().Width() + 6);
	if (width > (s_frame.Width() - 8))
		width = s_frame.Width() - 8;
	height = max_c(fContentView->fTextView->CountLines(), 20) *
			  fContentView->fTextView->LineHeight(0) +
			  (r.Height() - fContentView->fTextView->Bounds().Height());
	if (height > (s_frame.Height() - 29))
		height = s_frame.Height() - 29;
	r.right = r.left + width;
	r.bottom = r.top + height;

	if ((abs(Frame().Width() - r.Width()) < 5) &&
		(abs(Frame().Height() - r.Height()) < 5))
		r = fZoom;
	else {
		fZoom = Frame();
		s_frame.InsetBy(6, 6);

		if (r.Width() > s_frame.Width())
			r.right = r.left + s_frame.Width();
		if (r.Height() > s_frame.Height())
			r.bottom = r.top + s_frame.Height();

		if (r.right > s_frame.right) {
			r.left -= r.right - s_frame.right;
			r.right = s_frame.right;
		}
		if (r.bottom > s_frame.bottom) {
			r.top -= r.bottom - s_frame.bottom;
			r.bottom = s_frame.bottom;
		}
		if (r.left < s_frame.left) {
			r.right += s_frame.left - r.left;
			r.left = s_frame.left;
		}
		if (r.top < s_frame.top) {
			r.bottom += s_frame.top - r.top;
			r.top = s_frame.top;
		}
	}

	ResizeTo(r.Width(), r.Height());
	MoveTo(r.LeftTop());
}

//--------------------------------------------------------------------

void TMailWindow::AddSignature(BMailMessage *mail)
{
	char			predicate[B_FILE_NAME_LENGTH * 2];
	char			*str;
	BEntry			entry;
	BFile			file;
	BQuery			query;
	BVolume			vol;
	BVolumeRoster	volume;
	off_t			size;

	volume.GetBootVolume(&vol);
	query.SetVolume(&vol);
	sprintf(predicate, "%s = %s", INDEX_SIGNATURE, signature);
	query.SetPredicate(predicate);
	query.Fetch();

	if (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDWR);
		if (file.InitCheck() == B_NO_ERROR) {
			file.GetSize(&size);
			str = (char *)malloc(size);
			size = file.Read(str, size);
			mail->AddContent(str, size);
		}
	}
}

//--------------------------------------------------------------------

void TMailWindow::Forward(entry_ref *ref)
{
	char		*str;
	char		*str1;
	BFile		*file;
	attr_info	info;

	file = new BFile(ref, O_RDONLY);
	if (file->InitCheck() == B_NO_ERROR) {
		if (file->GetAttrInfo(B_MAIL_ATTR_SUBJECT, &info) == B_NO_ERROR) {
			str = (char *)malloc(info.size);
			file->ReadAttr(B_MAIL_ATTR_SUBJECT, B_STRING_TYPE, 0, str, info.size);
			if ((strstr(str, "fwd")) || (strstr(str, "forward")) ||
				(strstr(str, "FW")) || (strstr(str, "FORWARD")))
				fHeaderView->fSubject->SetText(str);
			else {
				str1 = (char *)malloc(strlen(str) + 1 + 6);
				sprintf(str1, "%s (fwd)", str);
				fHeaderView->fSubject->SetText(str1);
				free(str1);
			}
			free(str);
		}
		fContentView->fTextView->fHeader = true;
		fContentView->fTextView->LoadMessage(file, false, true, "Forwarded message:\n");
	}
	else
		delete file;
}

//--------------------------------------------------------------------

void TMailWindow::Print(void)
{
	int32			lines;
	int32			lines_page;
	int32			loop;
	int32			pages;
	float			line_height;
	BPrintJob		print("mail_print");
	BRect			r;

	if (print_settings)
		print.SetSettings(new BMessage(print_settings));
	else {
		PrintSetup();
		if (!print_settings)
			return;
	}

	lines = fContentView->fTextView->CountLines();
	line_height = fContentView->fTextView->LineHeight();
	if ((lines) && ((int)line_height) && (print.ConfigJob() == B_NO_ERROR)) {
		r = print.PrintableRect();
		lines_page = r.Height() / line_height;
		pages = lines / lines_page;
		r.top = 0;
		r.bottom = line_height * lines_page;
		r.right -= r.left;
		r.left = 0;

		print.BeginJob();
		if (!print.CanContinue())
			goto out;
		for (loop = 0; loop <= pages; loop++) {
			print.DrawView(fContentView->fTextView, r, BPoint(0, 0));
			print.SpoolPage();
			r.top += (line_height * lines_page);
			r.bottom += (line_height * lines_page);
			if (!print.CanContinue())
				goto out;
		}
		print.CommitJob();
out:;
	}
}

//--------------------------------------------------------------------

void TMailWindow::PrintSetup(void)
{
	BPrintJob	print("mail_print");
	status_t	result;

	if (print_settings)
		print.SetSettings(new BMessage(print_settings));

	if ((result = print.ConfigPage()) == B_NO_ERROR) {
		delete print_settings;
		print_settings = print.Settings();
	}
}

//--------------------------------------------------------------------

void TMailWindow::Reply(entry_ref *ref, TMailWindow *wind, bool all)
{
	char		*to = NULL;
	char		*cc;
	char		*header;
	char		*str;
	char		*str1;
	int32		finish;
	int32		len;
	int32		loop;
	int32		start;
	BFile		*file = NULL;
	attr_info	info;

	file = new BFile(ref, O_RDONLY);
	if (file->InitCheck() == B_NO_ERROR) {
		if (file->GetAttrInfo(B_MAIL_ATTR_REPLY, &info) == B_NO_ERROR) {
			to = (char *)malloc(info.size);
			file->ReadAttr(B_MAIL_ATTR_REPLY, B_STRING_TYPE, 0, to, info.size);
			fHeaderView->fTo->SetText(to);
		}
		else if (file->GetAttrInfo(B_MAIL_ATTR_FROM, &info) == B_NO_ERROR) {
			to = (char *)malloc(info.size);
			file->ReadAttr(B_MAIL_ATTR_FROM, B_STRING_TYPE, 0, to, info.size);
			fHeaderView->fTo->SetText(to);
		}

		if (file->GetAttrInfo(B_MAIL_ATTR_SUBJECT, &info) == B_NO_ERROR) {
			str = (char *)malloc(info.size);
			file->ReadAttr(B_MAIL_ATTR_SUBJECT, B_STRING_TYPE, 0, str, info.size);
			if (cistrncmp(str, "re:", 3) != 0) {
				str1 = (char *)malloc(strlen(str) + 1 + 4);
				sprintf(str1, "Re: %s", str);
				fHeaderView->fSubject->SetText(str1);
				free(str1);
			}
			else
				fHeaderView->fSubject->SetText(str);
			free(str);
		}

		wind->fContentView->fTextView->GetSelection(&start, &finish);
		if (start != finish) {
			str = (char *)malloc(finish - start + 1);
			wind->fContentView->fTextView->GetText(start, finish - start, str);
			if (str[strlen(str) - 1] != '\n') {
				str[strlen(str)] = '\n';
				finish++;
			}
			fContentView->fTextView->SetText(str, finish - start);
			free(str);

			finish = fContentView->fTextView->CountLines() - 1;
			for (loop = 0; loop < finish; loop++) {
				fContentView->fTextView->GoToLine(loop);
				fContentView->fTextView->Insert((const char *)QUOTE);
			}
			fContentView->fTextView->GoToLine(0);
		}
		else if (file)
			fContentView->fTextView->LoadMessage(file, true, true, NULL);

		if (all) {
			cc = (char *)malloc(1);
			cc[0] = 0;
			len = header_len(file);
			header = (char *)malloc(len);
			file->Seek(0, 0);
			file->Read(header, len);
			get_recipients(&cc, header, len, false);
			if (strlen(cc)) {
				if (to) {
					if (str = cistrstr(cc, to)) {
						len = 0;
						if (str == cc) {
							while ((strlen(to) + len < strlen(cc)) &&
								   ((str[strlen(to) + len] == ' ') ||
									(str[strlen(to) + len] == ','))) {
								len++;
							}
						}
						else {
							while ((str > cc) && ((str[-1] == ' ') || (str[-1] == ','))) {
								str--;
								len++;
							}
						}
						memmove(str, &str[strlen(to) + len], &cc[strlen(cc)] - 
														 &str[strlen(to) + len] + 1);
					}
				}
				fHeaderView->fCc->SetText(cc);
			}
			free(cc);
			free(header);
		}
		if (to)
			free(to);
		fReplying = true;
	}
}

//--------------------------------------------------------------------

status_t TMailWindow::Send(bool now)
{
	bool			close = false;
	char			mime[256];
	int32			index = 0;
	int32			len;
	BMailMessage	*mail;
	status_t		result;
	TListItem		*item;

	if (fResending)
		result = forward_mail(fRef, fHeaderView->fTo->Text(), now);
	else {
		mail = new BMailMessage();
				
		if (len = strlen(fHeaderView->fTo->Text()))
			mail->AddHeaderField(B_MAIL_TO, fHeaderView->fTo->Text());

		if (len = strlen(fHeaderView->fSubject->Text()))
			mail->AddHeaderField(B_MAIL_SUBJECT, fHeaderView->fSubject->Text());

		if (len = strlen(fHeaderView->fCc->Text()))
			mail->AddHeaderField(B_MAIL_CC, fHeaderView->fCc->Text());

		if (len = strlen(fHeaderView->fBcc->Text()))
			mail->AddHeaderField(B_MAIL_BCC, fHeaderView->fBcc->Text());

		if (len = fContentView->fTextView->TextLength())
			fContentView->fTextView->AddAsContent(mail, wrap_mode);

		if ((!fSigAdded) && (strcmp(signature, SIG_NONE)))
			AddSignature(mail);

		if (fEnclosuresView != NULL) {
			while (item = (TListItem *)fEnclosuresView->fList->ItemAt(index++)) {
				mail->AddEnclosure(item->Ref());
			}
		}
		result = mail->Send(now);
		delete mail;
	}

	switch (result) {
		case B_NO_ERROR:
			close = true;
			fSent = true;
			break;

		case B_MAIL_NO_DAEMON:
			close = true;
			fSent = true;
			sprintf(mime, "The mail_daemon is not running.  The message is \
queued and will be sent when the mail_daemon is started.");
			break;

		case B_MAIL_UNKNOWN_HOST:
		case B_MAIL_ACCESS_ERROR:
			sprintf(mime, "An error occurred trying to connect with the SMTP \
host.  Check your SMTP host name.");
			break;

		case B_MAIL_NO_RECIPIENT:
			sprintf(mime, "You must have either a \"To\" or \"Bcc\" recipient.");
			break;

		default:
			sprintf(mime, "An error occurred trying to send mail (0x%.8x).",
							result);
	}
	if (result != B_NO_ERROR) {
		beep();
		(new BAlert("", mime, "OK"))->Go();
	}
	if (close)
		PostMessage(B_QUIT_REQUESTED);
	return result;
}

// Word Services Begin
void TMailWindow::WindowActivated( bool active )
{
	if ( active && mClientSession ){
		BMessage abort( kWSBatchCheckAbort );
		mClientSession->GetServerMessenger()->SendMessage( &abort );
		delete mClientSession;
		mClientSession = NULL;
	}
	
	return;
}
void TMailWindow::StartBatchCheck( BMessage *msg )
{
	// We create a message to send to the speller.  This message
	// contains one or more BMessengers.  Each BMessenger specifies
	// one text block IN THE CLIENT that is to be spellchecked.  The
	// speller will use the messengers to send messages back to the text
	// block to get and set its data.
	
	// New in WS SDK 1.0d6 - allow word services in the header items too

	entry_ref	server_ref;
	
	status_t stat;

	stat = msg->FindRef( "Server", &server_ref );
	
	app_info	info;
	
	be_roster->GetAppInfo( &server_ref, &info );
	
	if ( info.thread == -1 ){
		team_id	team;
		be_roster->Launch( &server_ref, (BMessage*)NULL, &team );
		be_roster->GetRunningAppInfo( team, &info );
		// Give the server time to get started
		snooze( 100000 );
	}
	
	BMessenger	*serverMessenger = new BMessenger( NULL, info.team );
	
	
	BView *focusView = CurrentFocus();
	
	// Search for the text field with the focus
	
	if ( focusView == fContentView->fTextView ){
		mClientSession = new SelectionClient( fContentView->fTextView, serverMessenger );
	}else if ( focusView == fHeaderView->fTo->TextView() ){
		mClientSession = new SelectionClient( fHeaderView->fTo, serverMessenger );
	}else if ( focusView == fHeaderView->fBcc->TextView() ){
		mClientSession = new SelectionClient( fHeaderView->fBcc, serverMessenger );
	}else if ( focusView == fHeaderView->fCc->TextView() ){
		mClientSession = new SelectionClient( fHeaderView->fCc, serverMessenger );
	}else if ( focusView == fHeaderView->fSubject->TextView() ){
		mClientSession = new SelectionClient( fHeaderView->fSubject, serverMessenger );
	}
	
	BMessage batch( kWSBatchCheckMe );
	
	BMessenger mesr( mClientSession->GetHandler() );
	
	
	stat = batch.AddMessenger( "Target", mesr );
	
	// A Word Services server can have more than one service, for example
	// encryption and decryption.  The service index is stored in the menu
	// item message.
	
	int32	index;
	stat = msg->FindInt32( "Index", &index );
	
	stat = batch.AddInt32( "Index", index );
	
	serverMessenger->SendMessage( &batch, this );
	
	return;
}

void TMailWindow::HandleCheckTermination()
{
	delete mClientSession;
	
	mClientSession = NULL;
	
	return;
}
// Word Services End


//====================================================================

TMenu::TMenu(const char *name, const char *attribute, int32 message)
	  :BMenu(name)
{
	fAttribute = (char *)malloc(strlen(attribute) + 1);
	strcpy(fAttribute, attribute);
	fPredicate = (char *)malloc(strlen(fAttribute) + 5);
	sprintf(fPredicate, "%s = *", fAttribute);
	fMessage = message;
	BuildMenu();
}

//--------------------------------------------------------------------

TMenu::~TMenu(void)
{
	free(fAttribute);
	free(fPredicate);
}

//--------------------------------------------------------------------

void TMenu::AttachedToWindow(void)
{
	BuildMenu();
	BMenu::AttachedToWindow();
}

//--------------------------------------------------------------------

void TMenu::BuildMenu(void)
{
	char			name[B_FILE_NAME_LENGTH];
	int32			index = 0;
	BEntry			entry;
	BFile			file;
	BMenuItem		*item;
	BMessage		*msg;
	BQuery			query;
	BVolume			vol;
	BVolumeRoster	volume;
	entry_ref		ref;

	while (item = RemoveItem((int32)0)) {
		free(item);
	}
	volume.GetBootVolume(&vol);
	query.SetVolume(&vol);
	query.SetPredicate(fPredicate);
	query.Fetch();

	while (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDONLY);
		if (file.InitCheck() == B_NO_ERROR) {
			msg = new BMessage(fMessage);
			entry.GetRef(&ref);
			msg->AddRef("ref", &ref);
			file.ReadAttr(fAttribute, B_STRING_TYPE, 0, name, sizeof(name));
			if (index < 9)
				AddItem(new BMenuItem(name, msg, '1' + index));
			else
				AddItem(new BMenuItem(name, msg));
			index++;
		}
	}
}


//====================================================================

int32 header_len(BFile *file)
{
	char	*buffer;
	int32	len;
	int32	result = 0;
	off_t	size;

	if (file->ReadAttr(B_MAIL_ATTR_HEADER, B_INT32_TYPE, 0, &result, sizeof(int32)) != sizeof(int32)) {
		file->GetSize(&size);
		buffer = (char *)malloc(size);
		if (buffer) {
			file->Seek(0, 0);
			if (file->Read(buffer, size) == size) {
				while ((len = linelen(buffer + result, size - result, true)) > 2) {
					result += len;
				}
				result += len;
			}
			free(buffer);
			file->WriteAttr(B_MAIL_ATTR_HEADER, B_INT32_TYPE, 0, &result, sizeof(int32));
		}
	}
	return result;
}