/*
 *  Hello World for the CodeWarrior
 *  1998 Metrowerks Corp.
 *
 *  Questions and comments to:
 *       <mailto:support@metrowerks.com>
 *       <http://www.metrowerks.com/>
 */



int main(void)
{
	BApplication		app("application/x-whatever");
	
	BAlert*				alert = new BAlert("hello", "Hello World!", "OK");
	alert->Go();
	
	return 0;
}

