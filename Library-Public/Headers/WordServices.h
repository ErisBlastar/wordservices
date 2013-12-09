/* WordServices.h
 * Interface to Word Services Suite.  Definitions of constants and such.
 * ©1992 Working Software, Inc.
 * This source code is copyrighted.  Permission is granted to use the Word Services
 * portion of the Writeswell Jr. source code in your own programs, but you 
 * may not distribute the Writeswell Jr. word-processor code as a 
 * commercial product.  If you modify the code, please do not call it 
 * Writeswell Jr. (or Writeswell.)  This will ensure that people understand the 
 * program and don’t have to deal with a number of different versions with 
 * who-knows-what going on in the code.
 * 
 * Writeswell Jr. and Writeswell are trademarks of Working Software, Inc.
 * 10 Sep 91 Mike Crawford
 */

/* Apple Events */

#define kWordServicesClass			'WSrv'	/* The word services suite class */

#define kWSBatchCheckMe				'Btch'	/* Batch check this object */ 

#define pBackgroundHilite			'pBgH'	/* Elements that are highlit in background */

#define kWSTerminateBatchCheck		'WStp'	// Speller tells client done checking 

#define kWSBatchCheckAbort			'WSAb'	// Client tells speller to stop check

#define pBatchMenuString			'pBMs'	/* Menu string describing batch processing */

#define pInteractiveMenuString		'pIMs'	/* Menu string describing interactive processing */

#define pLocation					'pALc'	/* Alias record giving app's location */

#define pLockTransactionID			'pLID'

#define pCleanText					'pCTx'

#define pMenuIcon					'pMIc'	/* Small icon for service menu */

#define keyListHead					'Lhed'

#define keyClientAddress			'Cadr'

#define keyWantCleanReport			'ClRp'

#define kWSCheckWord				'CkWd'	/* Spell check a single word */

#define keyWSGuessCount				'GsCt'	/* Number of guesses requested */

#define kWSStartInteractive			'SInt'	/* Start interactive spelling session */

#define typeSmallIcon				'SICN'	/* Small Icon descriptor type */

#define kWSGuessWord				'Gess'	/* Make list of guesses for a misspelled word */

