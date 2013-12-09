class WorkingApplication: public BApplication
{
	public:
		WorkingApplication( char *signature );
		virtual ~WorkingApplication();
		
		BFile *GetAppFile( uint32 mode );
		
		void *GetResource( type_code type, int32 id, size_t *lenPtr = NULL );
		BPicture *GetPicture( int32 id );
		
		bool Validate();
		
	protected:
	
	private:
		BFile		*mExecutableFile;
		BResources	*mResources;
	
};