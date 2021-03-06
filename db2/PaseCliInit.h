#ifndef _PASECLIINIT_H
#define _PASECLIINIT_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <iconv.h>
#include <as400_types.h>
#include <as400_protos.h>
#include "PaseCliLic.h"

/* customer replace json parser (toolkit/parser-json) */
#define DB2JSONPARSER32_ENV_VAR "DB2JSONPARSER32"
#define DB2JSONPARSER32 "libjson400.a(shr.o)"
#define DB2JSONPARSER64_ENV_VAR "DB2JSONPARSER64"
#define DB2JSONPARSER64 "libjson400.a(shr_64.o)"

/* IBM i DB2 CLI interface (ILE) */
#define DB2CLISRVPGM "QSYS/QSQCLI"

/* IBM i DB2 max handles per-process (32k actually) */
#define PASECLIMAXRESOURCE 33000
typedef struct PaseCliResource {
  int hstmt;                       /* index hstmt or hdbc   */
  int hdbc;                        /* index to hdc (above)  */
  pthread_t tid;                   /* thread id (hdc or hstmt) */ 
  pthread_mutex_t threadMutexLock; /* lock hdbc or hstmt    */
  pthread_mutexattr_t threadMutexAttr; /* recursive lock    */
  int thread_init;                 /* mutext init complete */
  int in_progress;                 /* operation in progress */
  int use_flag;                    /* flag operation        */
  int use_size;                    /* flag operation size   */
  char *hKey;                      /* persistent key        */
  char *use_data;                  /* flag operation data   */
} PaseCliResource;

#define PASECLIMAXCCSID 3000
typedef struct PaseConvResource {
  int cssid_Ascii;
  int ccsid_Utf;
  char * charset_Ascii;
  char * charset_Utf;
  iconv_t AsciiToUtf;
  iconv_t UtfToAscii;
  int in_progress;
  pthread_mutex_t threadMutexLock;
  pthread_mutexattr_t threadMutexAttr;
} PaseConvResource;


/* trace CLI env var
 * export TRACE=on (file)
 * export TRACE=off
 * export TRACE=ws (console)
 * export TRACE=stop (file + stop coredump)
 * export TRACE=dbx (file + stop in dbx)
 * export DBXSRC="/home/srcdir1:/home/srcdir1:... "
 */
#define DB2CLITRACE "TRACE"
#define DB2CLITRACE_OFF 0
#define DB2CLITRACE_ON 1
#define DB2CLITRACE_FILE 1
#define DB2CLITRACE_WS 2
#define DB2CLITRACE_FILE_STOP 3
#define DB2CLITRACE_FILE_DBX 4
#define DB2CLIDBX "DBXSRC"

/* trace on? */
int init_cli_trace();
char ** init_cli_dbx();

/* load */
void * init_json_dlsym();
int init_cli_srvpgm();
int init_CCSID400( int newCCSID );
int init_job_CCSID400();

/* hdbc/hstmt scope locking */
void init_table_ctor(int handle, int hdbc);
void init_table_dtor(int handle);
void init_table_lock(int handle,int flag);
void init_table_unlock(int handle,int flag);
int init_table_in_progress(pthread_t tid,int flag);
/*
 * find any hstmt associated with this hdbc
 * (connection lock assumed client)
 */
int init_table_find_stmt(int hdbc);
/*
 * find hdbc associated with this hstmt
 * (connection lock assumed client)
 */
int init_table_stmt_2_conn(int hstmt);

/*
 * statment usage
 * 0 - normal
 */
#define DB2CLI_USAGE_NORMAL 0
void init_table_use_set(int handle, int usage);
int init_table_use_flag(int handle);
void * init_table_use_data(int handle);
void * init_table_use_data_new(int handle, int size, int nullterm);
void * init_table_use_data_copy_in(int handle, char *data, int size, int nullterm);
int init_table_use_data_copy_out(int handle, char *data, int size, int nullterm);

/* persistent connection */
void init_table_add_hash(int handle, char * db, char * uid, char * pwd, char * qual, int flag);
int init_table_hash_2_conn(char * db, char * uid, char * pwd, char * qual);
void init_table_add_hash_W(int handle, unsigned int * db, unsigned int * uid, unsigned int * pwd, unsigned int * qual, int flag);
int init_table_hash_2_conn_W(unsigned int * db, unsigned int * uid, unsigned int * pwd, unsigned int * qual);
int init_table_hash_active(int handle, int flag);

/* utilities */
int custom_strlen_utf16(unsigned int * src);
void * custom_alloc_zero(int sz);

/* conversion */
int custom_iconv(int isInput, char *fromBuffer, char *toBuffer, size_t sourceLen, size_t bufSize, int myccsid, int utfccsid);

/* rare use allocators only */
void init_lock();
void init_unlock();

#endif /* _PASECLIINIT_H */

