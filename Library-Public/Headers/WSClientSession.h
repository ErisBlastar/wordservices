// WSClientSession.h
// 21 Aug 97 Mike Crawford crawford@scruznet.com
//
// If you need to maintain some sort of state during spellchecking,
// make a subclass of this and keep the state there.  If there's an
// action to do when it's done, make the destructor of your class fire off
// a message, for example to send an e-mail after spellchecking

class WSClientSession
{
	public:
		WSClientSession();
		virtual ~WSClientSession();
};