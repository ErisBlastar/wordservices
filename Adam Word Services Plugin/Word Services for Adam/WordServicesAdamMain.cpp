/*
 *  Sample EMS plugin: Upper-Lower
 *
 *  Filename: CaseTran.cpp
 *
 *  Last edited: October 22, 1996
 *
 *  Author: Scott Manjourides
 *
 *  Copyright 1995, 1996 QUALCOMM Inc.
 *
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 */

#ifdef _DEBUG
#include <ctype.h>
#endif // _DEBUG

//#include <afxwin.h> // MFC & Windows master header
//#include "resource.h" // Resource IDs for dialogs and icons
//#include "UpprLowr.h" // CUpprLowrDLL

//#include <ctype.h>

#include <malloc.h> // malloc, free
#include <string.h> // strdup
//#include <istream.h> // ifstream class
//#include <fstream.h> // ofstream class

//#include <windows.h>
#include "ems-be.h" // The EMS API
#include "cistrncmp.h"
#include "AdamAddOn.h"

#include "mimetype.h"   

#ifndef WIN32    
#include <ctype.h>
#endif

#include "WordServicesAdam.h"

/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 11;
static const char *kPluginDescription = "UPPERlower Sample Plugin, v1.3";
//static const unsigned long kPluginIconID = IDI_MAIN;

static const int kWordServicesID = 1;

static const int kNumTrans       = 1;


static const int kBufferSize = 128000;

static const char *kFileErrorStr = "File I/O Error";
static const char *kTransFailedStr = "Translator Failed";

/*****************************************************************************/
/* GLOBALS */

// This externs into UpprLowr.cpp
//extern CUpprLowrDLL theDLL;

static struct TransInfoStruct {
	char *description;
	long type;
	long subtype;
	unsigned long flags;
	char *MimeType;
	char *MimeSubtype;
	unsigned long nIconID;
} gTransInfo[] = {
	{
		"Word Services",
		EMST_LANGUAGE,
		1L, // unused
		EMSF_ON_REQUEST,
		"text", "plain",
		0//IDI_UPPER
	}
};


/*****************************************************************************/
/* TYEPDEFS */

// Yes, this is a pointer to a function
typedef void (*FilterFuncType)(char *, UINT, int) ;

/*****************************************************************************/
/* MACROS */

#define safefree(p) { if (p) { free(p); p=NULL; } }

/*****************************************************************************/
/* LOCAL FUNCTION PROTOTYPES */

// Generalized functions
int CheckValidContext(long trans_id, long context);
int CheckValidMimeType(long trans_id, emsMIMEtypeP in_mime);
void DoIconInit(long trans_id, HICON FAR*FAR* trans_icon);
//int DoFilter(ifstream& inFile, ofstream& outFile, emsProgress progress, FilterFuncType filter);
int DoFilter(BFile &inFile, BFile &outFile, emsProgress progress, FilterFuncType filter);

// The actual filter algorithms
int ismyspace(int c);
void UpperFilter(register char *cp, UINT size, int Ignored);
void LowerFilter(register char *cp, UINT size, int Ignored);
void ToggleFilter(register char *cp, UINT size, int Ignored);
void WordFilter(register char *cp, UINT size, int Ignored);
void SentenceFilter(register char *cp, UINT size, int nAfterBufChar);

int IsUpper( int c );
int IsLower( int c );
int ToUpper( int c );
int ToLower( int c );


/*****************************************************************************/
/* TRANSLATER API FUNCTIONS */

/*****\
*
* ems_plugin_version: Get the version of the API used for this plugin
*
\*****/

extern "C" long WINAPI ems_plugin_version(
    short FAR* api_version      /* Place to return api version */
)
{
	*api_version = EMS_VERSION;
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_init: Initialize plugin and get its basic info
*
\*****/

extern "C" long WINAPI ems_plugin_init(
    void FAR*FAR* ,          /* Out: Return for allocated instance structure */
    short ,        /* In: The API version eudora is using */
    emsMailConfigP ,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
{
	// We don't use 'globals'
	pluginInfo->numTrans = kNumTrans;
	
	pluginInfo->desc = strdup(kPluginDescription);
	
	DoIconInit(-1, &(pluginInfo->icon));
	
	pluginInfo->id = kPluginID;
	
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translator_info: Get details about a translator in a plugin
*
\*****/

extern "C" long WINAPI ems_translator_info(
    void FAR* ,          /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo    /* In/Out: Return Translator Information */
)
{
	TransInfoStruct *InfoPtr = NULL;

	if ( (transInfo->id <= 0) || (transInfo->id > kNumTrans) )
		return (EMSR_INVALID_TRANS);

	InfoPtr = gTransInfo + (transInfo->id - 1);

	transInfo->type = InfoPtr->type;
	
	transInfo->flags = InfoPtr->flags;
	
	transInfo->desc = strdup(InfoPtr->description);

	DoIconInit(transInfo->id, &(transInfo->icon));

	return (EMSR_OK);
}

/*****\
*
* ems_can_translate_file:
*     Check and see if a translation can be performed (file version)
*
\*****/

extern "C" long WINAPI ems_can_translate(
    void FAR* ,          /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,       /* In: Translator Info */
    emsDataFileP inTransData,   /* In: What to translate */
    emsResultStatusP  /* Out: Translations Status information */
)
{
	if ((transInfo) && (inTransData))
	{
		const long id = (long) transInfo->id;

		if ((id <= 0) || (id > kNumTrans))
			return (EMSR_INVALID_TRANS);

		const long context = inTransData->context;
		const emsMIMEtypeP in_mime = inTransData->info;

		if ( (CheckValidContext(id, context)) && (CheckValidMimeType(id, in_mime)) )
		{
			return (EMSR_NOW);
		}
	}

	return (EMSR_CANT_TRANS);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translate_file: Actually perform a translation on a file
*
\*****/

extern "C" long WINAPI ems_translate_file(
    void FAR*,          /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,   /* In: Translator Info */
    emsDataFileP inFilePB,      /* In: What to translate */
    emsProgress progress,       /* Func to report progress/check for abort */
    emsDataFileP outFilePB,       /* Out: Result of the translation */
    emsResultStatusP transStatus /* Out: Translations Status information */
)
{ 
	const char *in_file = inFilePB->fileName;
	const char *out_file = outFilePB->fileName;

#if 0
	ifstream inFile(in_file, ios::binary);
#else
	BFile	inFile;
#endif

	if (inFile.SetTo(in_file, B_READ_ONLY))
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kFileErrorStr);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_NO_INPUT_FILE);
	}

#if 0
	ofstream outFile(out_file, ios::trunc | ios::binary);
#else
	BFile	outFile;
#endif

	if (outFile.SetTo(out_file, B_READ_WRITE | B_CREATE_FILE))
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kFileErrorStr);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_CANT_CREATE);
	}

	int err = 0;

	//err = DoFilter(inFile, outFile, progress, filterArr[transInfo->id]);

	err = DoWordServices( inFile, outFile, progress );
	
//	inFile.close();
//	outFile.close();

	// Check if anything went wrong 
	if (err)
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kTransFailedStr);
			transStatus->code = (long) __LINE__;
		}
		return (EMSR_TRANS_FAILED);
	}

	// Required to set the returned MIME type
	if (outFilePB)
		outFilePB->info = make_mime_type("text", "plain", "1.0");

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_finish: End use of a plugin and clean up
*
\*****/

extern "C" long WINAPI ems_plugin_finish(
    void FAR*            /* Pointer to translator instance structure */
)
{
	// We don't use 'globals'
	return (EMSR_OK); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_free: Free memory allocated by EMS plug-in
*
\*****/

extern "C" long WINAPI ems_free(
    void FAR* mem               /* Memory to free */
)
{
	if (mem)
		safefree(mem);

	return (EMSR_OK);
}

/*****************************************************************************/
/* THE TRANSLATOR ALGORITHMS */

void UpperFilter(register char *cp, UINT size, int)
{
	if (cp)
	{
		register char *end_buf;

		for (end_buf = (cp + size); cp < end_buf; cp++)
			if (IsLower(*cp))
				*cp = ToUpper(*cp);
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0
int DoFilter(BFile &inFile, BFile &outFile, emsProgress, FilterFuncType filter)
{
	char *buffer = (char *) malloc(kBufferSize);

	if (!buffer)
		return (-1);

	filter(NULL, 0, 0); // Initialize the filter
	int nCharsRead;

	while ((inFile.IsReadable()) && (outFile.IsWritable()))// && (!inFile.eof()))
	{
		nCharsRead = inFile.Read(buffer, kBufferSize);
		if (nCharsRead == 0) break;

		filter(buffer, nCharsRead, inFile.Position());
		outFile.Write(buffer, nCharsRead);
	}

	safefree(buffer);

	return (0);
}
#endif

/*****************************************************************************/
/* HELPER FUNCTIONS */

int CheckValidContext(
	long trans_id,  /* ID of translator */
	long context     /* Context for check; e.g. EMSF_ON_xxx */
)
{
	return ((context & gTransInfo[trans_id-1].flags) ? 1 : 0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int CheckValidMimeType(
    long trans_id,             /* ID of translator to check */
    emsMIMEtypeP in_mime        /* MIME type of incomming data */
)
{
	char *pType = gTransInfo[trans_id-1].MimeType;
	char *pSubtype = gTransInfo[trans_id-1].MimeSubtype;

	if ( ((!pType) || (strcmp(in_mime->type, pType) == 0))
		&& ((!pSubtype) || (strcmp(in_mime->subType, pSubtype) == 0)) )
		return TRUE;

	return FALSE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void DoIconInit(
    long trans_id,         /* ID of translator to get info for */
    HICON FAR*FAR* icon   /* Return for icon data */
)
{
	if (!icon)
		return;

//	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*icon = (HICON *)malloc(sizeof(HICON));

	if (trans_id < 0) /* Main plugin icon, not specific translator */
	{
	//	**icon = theDLL.LoadIcon(kPluginIconID); /* 32x32 */
	}
	else /* The actual translators */
	{
	//	**icon = theDLL.LoadIcon(gTransInfo[trans_id-1].nIconID); /* 16x16 */
	}
}


int main(int argc, char **argv){ printf("This is an add-on for Adam.\n"); }
const int32	addon_IDVersion(void)	{return(ADAM_ADDON_VERSION);}
const char *addon_IDName(void *) 	{return(kPluginDescription);}
const char *addon_IDAuthor(void *)	{return("Michael D. Crawford crawford@scruznet.com");}
const char *addon_IDNotice(void *)	{return("Copyright 1997 Working Software Inc.");}
