Edit: 12/09/2013 by Eris Blastar
Mike Crawford's new email is goingware@gmail.com
His new website is http://www.warplife.com/

He asked me to save his old projects if I could find them and then host them at Github for him. I was lucky to find this one.
erisblastar@gmail.com
erisblastar@att.net
http://blastar.in
http://www.blastar.org
Twitter @erisblastar

Word Services SDK

by Michael D. Crawford, crawford@scruznet.com

Developer's web page:		http://www.wordservices.org/Developers/bespec.html

User's web page:			http://www.wordservices.org/be.html

This is the first alpha release of the Word Services SDK for the Preview Release.  The code works well, and about everything needed for Word Services is implemented but there are minor bugs.  I have fixed the bugs reported by the users.  Try out the example programs, read the documentation that I will place on the web page, and start implementing it in your programs.  Feel free to ask me questions.

To demonstrate Word Services:

• Launch the Spellswell Demo.  Spellswell will ensure that its "WordServicesServer" attribute is present and is indexed.  (It is only necessary to prelaunch Spellswell for its initial installation).

• Launch QL Ltr Writer.  It will query each disk for files with the WordServicesServer attribute, then fetch the menu strings from each server, and add them to its services menu.  A server can provide one or more menu strings - an encryption program would want to provide "encrypt" and "decrypt" commands.

• Type some misspelled text.  

• New in 1.0d3 - Select the text you want to have checked.

• Select Check Spelling from the services menu.  Spellswell will get the text and suggest a replacement.

• New in 1.0d5 - the erroneous word is highlighted in its original document, and Spellswell hides its context view.  If you don't yet support background highlighting, Spellswell will still show the context itself.

• Use the suggestion or type your own, then press the Replace button.  

• The text is replaced back in the original document.

The source code to QL Ltr Writer is included.  The real QuickLetter will be a commercial product, but we'll branch this code off and name it Writeswell Jr. instead, and maintain it as a bare-bones word processor just suitable for doing Word Services.

New in 1.0d5: Spellcheck your BeMail e-mails!

It took me only about two hours to integrate Word Services into the BeMail source code that Be supplies in the optional/sample-code folder.  Background highlighting came right along with it.

New in 1.0d4: Spellcheck your Adam e-mails!

• Place the WordServicesAdam add-on in /boot/home/config/add-ons/adam.

• Type a misspelled Adam e-mail message.

• Select Word Services from the Actions menu.

• Select Check Spelling from the Services menu of the window that appears.

The Adam add-on is very primitive and will be substantially improved.  Source code is included if you want to help.

I'll have more information, and more builds, real soon now.

Regards,

Mike Crawford
Crawford Software Consulting

Change Log:

1.0.1d2

The code now builds and runs under R3, and for Intel.  I don't have an Intel adam add-on yet.

1.0a1

• Background highlighting of erroneous text works better but still has some small cosmetic problems.

• Beginning in 1.0a1, the Spellswell Demo is provided rather than the full Spellswell product.  If you are actually developing a Word Services client, I can give you a Spellswell for testing - contact me at crawford@scruznet.com to ask for one.

• The Services menu displays icons from the server applications

1.0d5

• Erroneous text is highlighted in the original document.  Spellswell tests whether this is supported, and resizes its window accordingly to show or hide the context view.

• Many user interface improvements to Spellswell.

• The nasty icons are just there as placeholders.  We've got a professional user interface designer working on nicer ones.

1.0d4

• The alphabetic view of the dictionary is just starting to work.  The horizontal scroll bar takes you through pages of the dictionary.  This is a feature that is unique to Spellswell as far as I know.

• Added the Adam Word Services add-on.

1.0d3

• Implemented selection checking in QuickLetter Letter Writer.  When a Word Services Session is started, an object called a SelectionClient is created and a pointer to it stored in the window object.  The SelectionClient contains a SelectionHandler that knows about its BTextView and the selection.  The selection handler handles the GET and SET messages from the speller.

• Added a termination message sent by the speller when checking is complete.  This allows QuickLetter to delete the SelectionClient.  An e-mail program could use this to queue a message after it's checked.

• Fixed a bug in Spellswell that would incorrectly specify the length of the new text.

1,0d2

• Included libprefs.so in the distribution.  Spellswell won't launch without it installed.

• I rewrite the WordServicesServer attribute if the index is created - Spellswell comes with the attribute, but the index doesn't know about it unless I rewrite the attribute.

• Added splash screens to both programs.

• Added a "Word Services Web Page" item to the Services menu.  This will bring up a vendor-neutral user-oriented page, explaining Word Services and listing available client and server programs.

• I had misunderstood how a replace works in Be scripting.  To replace a word in a BTextView, I now first delete it with a SET message with no "data" item, then insert the replacement with another SET message.  Note that this is quite different from the way it is done on MacOS, and brings up the issue of preserving the font and style during the transaction.

• Made some improvements to the appearance of Spellswell.

1.0d1 First release
