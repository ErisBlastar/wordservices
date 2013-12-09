#include "WorkingApplication.h"

WorkingApplication::WorkingApplication( char *signature )
	: BApplication( signature ),
	mExecutableFile( NULL ),
	mResources( NULL )
{
	mExecutableFile = GetAppFile( O_RDONLY );
	
	mResources = new BResources( mExecutableFile );

	return;
}

WorkingApplication::~WorkingApplication()
{
	delete mResources;
	
	delete mExecutableFile;

	return;
}

bool WorkingApplication::Validate()
{
	if ( !mResources )
		return false;
	
	if ( !mExecutableFile )
		return false;
	
	return true;
}

BFile *WorkingApplication::GetAppFile( uint32 mode )
{
	app_info	info;
	
	GetAppInfo( &info );
	
	BFile *file = new BFile;
	
	if ( !file )
		return NULL;
	
	status_t stat;
	
	stat = file->SetTo( &info.ref, mode );
	
	if ( stat ){
		delete file;
		return NULL;
	}
	
	return file;
}

void *WorkingApplication::GetResource( type_code type, int32 id, size_t *lenPtr )
{
	void *ptr;
	size_t	len;
	char *name;
	
	if ( !mResources->GetResourceInfo( type, id, &name, &len ) )
		return NULL;
	
	ptr = new char[ len ];
	if ( !ptr )
		return NULL;
	
	status_t stat = mResources->ReadResource( type, id, ptr, 0, len );
	if ( stat ){
		delete [] ptr;
		return NULL;
	}
	
	if ( lenPtr )
		*lenPtr = len;

	return ptr;
}

BPicture *WorkingApplication::GetPicture( int32 id )
{
	char *ptr;
	size_t len;

	ptr = (char*)GetResource( 'PICT', id, &len );
	if ( !ptr )
		return NULL;
	
	BMessage msg;
	
	msg.Unflatten( ptr );
	
	BArchivable *unarchived = instantiate_object( &msg );
	
	BPicture *pic = (BPicture*)cast_as( unarchived, BPicture );
	
	return pic;

}