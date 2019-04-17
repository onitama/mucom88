
//
//	membuf.cpp structures
//
#ifndef __membuf_h
#define __membuf_h

//  growmem class

/*
	rev 53
	mingw : warning : クラスは仮想関数を持つのに仮想デストラクタでない。
	に対処。
*/

class CMemBuf {
public:
	CMemBuf();
	CMemBuf( int sz );
	virtual ~CMemBuf();
	void AddIndexBuffer( void );
	void AddIndexBuffer( int sz );

	char *GetBuffer( void );
	int GetBufferSize( void );
	int *GetIndexBuffer( void );
	void SetIndex( int idx, int val );
	int GetIndex( int idx );
	int GetIndexBufferSize( void );
	int SearchIndexValue( int val );

	void RegistIndex( int val );
	void Index( void );
	void Put( int data );
	void Put( short data );
	void Put( char data );
	void Put( unsigned char data );
	void Put( float data );
	void Put( double data );
	void PutStr( const char *data );
	void PutStrDQ( const char *data );
	void PutStrBlock( const char *data );
	void PutCR( void );
	void PutData( void *data, int sz );
	void PutStrf( const char *format, ... );
	int PutFile( const char *fname );
	int SaveFile( const char *fname );
	char *GetFileName( void );
	int GetSize( void ) { return cur; }
	void ReduceSize( int new_cur );
	char *PreparePtr( int sz );

private:
	virtual void InitMemBuf( int sz );
	virtual void InitIndexBuf( int sz );

	//		Data
	//
	int		limit_size;			// Separate size
	int		size;				// Main Buffer Size
	int		cur;				// Current Size
	char	*mem_buf;			// Main Buffer

	int		idxflag;			// index Mode Flag
	int		*idxbuf;			// Index Buffer
	int		idxmax;				// Index Buffer Max
	int		curidx;				// Current Index

	char	name[256];			// File Name
};


#endif
