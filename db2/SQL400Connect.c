#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <iconv.h>
#include <sqlcli1.h>
#include "PaseCliInit.h"
#include "PaseCliAsync.h"

static SQLHANDLE env_hndl;
static int env_server_mode;

/*
 * env 
 */
SQLRETURN custom_SQLSetEnvCCSID( SQLHANDLE env, int myccsid) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLINTEGER yes = SQL_TRUE;
  if (!env_hndl) {
    env_hndl = env;
    switch(myccsid) {
    case 1208: /* UTF-8 */
      sqlrc = SQLSetEnvAttr((SQLHENV)env_hndl, (SQLINTEGER)SQL400_ATTR_PASE_CCSID, (SQLPOINTER)&myccsid, (SQLINTEGER) 0);
      sqlrc = SQLSetEnvAttr((SQLHENV)env_hndl, (SQLINTEGER)SQL_ATTR_UTF8, (SQLPOINTER)&yes, (SQLINTEGER) 0);
      break;
    case 1200: /* UTF-16 */
      sqlrc = SQLSetEnvAttr((SQLHENV)env_hndl, (SQLINTEGER)SQL400_ATTR_PASE_CCSID, (SQLPOINTER)&myccsid, (SQLINTEGER) 0);
      break;
    default:
      sqlrc = SQLSetEnvAttr((SQLHENV)env_hndl, (SQLINTEGER)SQL400_ATTR_PASE_CCSID, (SQLPOINTER)&myccsid, (SQLINTEGER) 0);
      break;
    }
  }
  return sqlrc;
}
SQLRETURN custom_SQLOverrideCCSID400( SQLINTEGER  newCCSID ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLINTEGER yes = SQL_TRUE;
  SQLHANDLE env = 0;
  int myccsid = 0;
  if (!env_hndl) {
    myccsid = init_CCSID400(newCCSID);
    sqlrc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    sqlrc = custom_SQLSetEnvCCSID(env, myccsid);
  }
  return sqlrc;
}

/*
 * connect
 */
SQLRETURN custom_SQL400ConnectBoth( 
 SQLCHAR * db,
 SQLCHAR * uid,
 SQLCHAR * pwd,
 SQLCHAR * qual, 
 SQLINTEGER * ohnd, 
 SQLINTEGER  acommit, 
 SQLCHAR * alibl, 
 SQLCHAR * acurlib, 
 SQLSMALLINT iswide, 
 SQLSMALLINT isPersistent) {
  int i = 0;
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLHENV henv = 0; 
  SQLHDBC hdbc = 0;
  SQLCHAR * db_str = (SQLCHAR *) db;
  SQLCHAR * uid_str = (SQLCHAR *) uid;
  SQLCHAR * pwd_str = (SQLCHAR *) pwd;
  SQLSMALLINT db_len = SQL_NTS;
  SQLSMALLINT uid_len = SQL_NTS;
  SQLSMALLINT pwd_len = SQL_NTS;
  int db_slen = strlen(db);
  int uid_slen = strlen(uid);
  int pwd_slen = strlen(pwd);

  SQLINTEGER attr = SQL_TRUE;

  /* nothing open, so far */
  *ohnd = 0;

  /* henv -- IBM i only one env (always 1) */
  if (!env_hndl) {
    sqlrc = custom_SQLOverrideCCSID400(0);
    henv = env_hndl;
  }
  henv = env_hndl;
  /* henv -- always server mode */
  if (!env_server_mode) {
    attr = SQL_TRUE;
    sqlrc = SQLSetEnvAttr((SQLHENV)henv, SQL_ATTR_SERVER_MODE, (SQLPOINTER)&attr, (SQLINTEGER) 0);
    env_server_mode = 1;
  }

  /* length user profile */
  switch(iswide) {
  case 1:
    db_slen = custom_strlen_utf16((unsigned int*)db);  /* wcslen(db);  - not work 64 bit due to header wchar_t changed __64__ */
    uid_slen = custom_strlen_utf16((unsigned int*)uid); /* wcslen(uid); - not work 64 bit due to header wchar_t changed __64__  */
    pwd_slen = custom_strlen_utf16((unsigned int*)pwd); /* wcslen(pwd); - not work 64 bit due to header wchar_t changed __64__  */
    break;
  default:
    db_slen = strlen(db);
    uid_slen = strlen(uid);
    pwd_slen = strlen(pwd);
    break;
  }
  /* null input */
  if (db == NULL || !db_slen) {
    db_str = (SQLCHAR *) NULL;
    db_len = 0;
  }
  if (uid == NULL || !uid_slen) {
    uid_str = (SQLCHAR *) NULL;
    uid_len = 0;
  }
  if (pwd == NULL || !pwd_slen) {
    pwd_str = (SQLCHAR *) NULL;
    pwd_len = 0;
  }
  /* possible persistent connection */
  if (isPersistent) {
    switch(iswide) {
    case 1:
      *ohnd = init_table_hash_2_conn_W((unsigned int*)db, (unsigned int*)uid, (unsigned int*)pwd, (unsigned int*)qual);
      break;
    default:
      *ohnd = init_table_hash_2_conn((char *)db, (char *)uid, (char *)pwd, (char *)qual);
      break;
    }
  }
  hdbc = *ohnd;
  /* re-use connection */
  if (hdbc > 1) {
    return SQL_SUCCESS;
  }
  /* hdbc -- new connection */
  sqlrc = SQLAllocHandle(SQL_HANDLE_DBC, henv, ohnd);
  if (sqlrc != SQL_SUCCESS) {
    return sqlrc;
  }
  hdbc = *ohnd;
  init_table_lock(hdbc, 0);
  /* connect */
  switch(iswide) {
  case 1:
    sqlrc = SQLConnectW( (SQLHDBC)*ohnd, (SQLWCHAR *)db_str, (SQLSMALLINT)db_len, (SQLWCHAR *)uid_str, (SQLSMALLINT)uid_len, (SQLWCHAR *)pwd_str, (SQLSMALLINT)pwd_len );
    break;
  default:
    sqlrc = SQLConnect( (SQLHDBC)*ohnd, (SQLCHAR *)db_str, (SQLSMALLINT)db_len, (SQLCHAR *)uid_str, (SQLSMALLINT)uid_len, (SQLCHAR *)pwd_str, (SQLSMALLINT)pwd_len );
    break;
  }
  if (sqlrc != SQL_SUCCESS) {
    init_table_unlock(hdbc, 0);
    return sqlrc;
  }
  /* possible persistent connection */
  if (isPersistent) {
    switch(iswide) {
    case 1:
      init_table_add_hash_W(hdbc, (unsigned int*)db, (unsigned int*)uid, (unsigned int*)pwd, (unsigned int*)qual, 0);
      break;
    default:
      init_table_add_hash(hdbc, db, uid, pwd, qual, 0);
      break;
    }
  }
  /* hdbc - commit isolation */
  if (acommit < SQL_TXN_NOCOMMIT || acommit > SQL_TXN_SERIALIZABLE) {
    acommit = SQL_TXN_NOCOMMIT;
  }
  attr = acommit;
  sqlrc = SQLSetConnectAttr((SQLHDBC)hdbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)&attr, 0);
  /* hdbc - system naming */
  attr = SQL_TRUE;
  sqlrc = SQLSetConnectAttr((SQLHDBC)hdbc, SQL_ATTR_DBC_SYS_NAMING, (SQLPOINTER)&attr, 0);
  /* hdbc - autocommit */
  attr = SQL_TRUE;
  sqlrc = SQLSetConnectAttr((SQLHDBC)hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)&attr, 0);
  /* hdbc - libl */
  if (alibl) {
    sqlrc = custom_SQL400ChgLibl((SQLHDBC)hdbc, (SQLCHAR *)alibl);
  }
  /* hdbc - curlib */
  if (acurlib) {
    sqlrc = custom_SQL400ChgCurLib((SQLHDBC)hdbc, (SQLCHAR *)acurlib);
  }
  /* unlock and return */
  init_table_unlock(hdbc, 0);
  return sqlrc;
}
SQLRETURN custom_SQL400Connect( SQLCHAR * db, SQLCHAR * uid, SQLCHAR * pwd, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  sqlrc = custom_SQL400ConnectBoth( db, uid, pwd, NULL, ohnd, acommit, alibl, acurlib, 0, 0);
  return sqlrc;
}
SQLRETURN custom_SQL400ConnectW( SQLWCHAR * db, SQLWCHAR * uid, SQLWCHAR * pwd, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  sqlrc = custom_SQL400ConnectBoth( (SQLCHAR *)db, (SQLCHAR *)uid, (SQLCHAR *)pwd, NULL, ohnd, acommit, alibl, acurlib, 1, 0);
  return sqlrc;
}
SQLRETURN custom_SQL400pConnect( SQLCHAR * db, SQLCHAR * uid, SQLCHAR * pwd, SQLCHAR * qual, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  sqlrc = custom_SQL400ConnectBoth( db, uid, pwd, qual, ohnd, acommit, alibl, acurlib, 0, 1);
  return sqlrc;
}
SQLRETURN custom_SQL400pConnectW( SQLWCHAR * db, SQLWCHAR * uid, SQLWCHAR * pwd, SQLCHAR * qual, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  sqlrc = custom_SQL400ConnectBoth( (SQLCHAR *)db, (SQLCHAR *)uid, (SQLCHAR *)pwd, qual, ohnd, acommit, alibl, acurlib, 1, 1);
  return sqlrc;
}

SQLRETURN custom_SQL400ConnectUtf8( SQLINTEGER myccsid, SQLCHAR * db, SQLCHAR * uid, SQLCHAR * pwd, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLHENV henv = env_hndl; 
  SQLCHAR db_utf8[11];
  SQLCHAR uid_utf8[11];
  SQLCHAR pwd_utf8[11];
  sqlrc = custom_SQL400IgnoreNullToUtf8(henv,  (SQLPOINTER) db, (SQLINTEGER)  strlen(db), (SQLPOINTER)  &db_utf8, (SQLINTEGER)  sizeof(db_utf8), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf8(henv, (SQLPOINTER) uid, (SQLINTEGER) strlen(uid), (SQLPOINTER) &uid_utf8, (SQLINTEGER) sizeof(uid_utf8), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf8(henv, (SQLPOINTER) pwd, (SQLINTEGER) strlen(pwd), (SQLPOINTER) &pwd_utf8, (SQLINTEGER) sizeof(pwd_utf8), myccsid);
  sqlrc = custom_SQL400ConnectBoth( db_utf8, uid_utf8, pwd_utf8, NULL, ohnd, acommit, alibl, acurlib, 0, 0);
  return sqlrc;
}
SQLRETURN custom_SQL400pConnectUtf8( SQLINTEGER myccsid, SQLCHAR * db, SQLCHAR * uid, SQLCHAR * pwd, SQLCHAR * qual, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLHENV henv = env_hndl; 
  SQLCHAR db_utf8[11];
  SQLCHAR uid_utf8[11];
  SQLCHAR pwd_utf8[11];
  SQLCHAR qual_utf8[11];
  sqlrc = custom_SQL400IgnoreNullToUtf8(henv, (SQLPOINTER)   db, (SQLINTEGER)   strlen(db), (SQLPOINTER)   &db_utf8, (SQLINTEGER)   sizeof(db_utf8), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf8(henv, (SQLPOINTER)  uid, (SQLINTEGER)  strlen(uid), (SQLPOINTER)  &uid_utf8, (SQLINTEGER)  sizeof(uid_utf8), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf8(henv, (SQLPOINTER)  pwd, (SQLINTEGER)  strlen(pwd), (SQLPOINTER)  &pwd_utf8, (SQLINTEGER)  sizeof(pwd_utf8), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf8(henv, (SQLPOINTER) qual, (SQLINTEGER) strlen(qual), (SQLPOINTER) &qual_utf8, (SQLINTEGER) sizeof(qual_utf8), myccsid);
  sqlrc = custom_SQL400ConnectBoth( db_utf8, uid_utf8, pwd_utf8, qual_utf8, ohnd, acommit, alibl, acurlib, 0, 1);
  return sqlrc;
}

SQLRETURN custom_SQL400ConnectUtf16( SQLINTEGER myccsid, SQLCHAR * db, SQLCHAR * uid, SQLCHAR * pwd, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLHENV henv = env_hndl; 
  SQLCHAR db_utf16[22];
  SQLCHAR uid_utf16[22];
  SQLCHAR pwd_utf16[22];
  sqlrc = custom_SQL400IgnoreNullToUtf16(henv, (SQLPOINTER)  db, (SQLINTEGER)  strlen(db), (SQLPOINTER)  &db_utf16, (SQLINTEGER)  sizeof(db_utf16), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf16(henv, (SQLPOINTER) uid, (SQLINTEGER) strlen(uid), (SQLPOINTER) &uid_utf16, (SQLINTEGER) sizeof(uid_utf16), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf16(henv, (SQLPOINTER) pwd, (SQLINTEGER) strlen(pwd), (SQLPOINTER) &pwd_utf16, (SQLINTEGER) sizeof(pwd_utf16), myccsid);
  sqlrc = custom_SQL400ConnectBoth( db_utf16, uid_utf16, pwd_utf16, NULL, ohnd, acommit, alibl, acurlib, 0, 0);
  return sqlrc;
}
SQLRETURN custom_SQL400pConnectUtf16( SQLINTEGER myccsid, SQLCHAR * db, SQLCHAR * uid, SQLCHAR * pwd, SQLCHAR * qual, SQLINTEGER * ohnd, SQLINTEGER  acommit, SQLCHAR * alibl, SQLCHAR * acurlib ) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLHENV henv = env_hndl; 
  SQLCHAR db_utf16[22];
  SQLCHAR uid_utf16[22];
  SQLCHAR pwd_utf16[22];
  SQLCHAR qual_utf16[22];
  sqlrc = custom_SQL400IgnoreNullToUtf16(henv, (SQLPOINTER)   db, (SQLINTEGER)   strlen(db), (SQLPOINTER)   &db_utf16, (SQLINTEGER)   sizeof(db_utf16), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf16(henv, (SQLPOINTER)  uid, (SQLINTEGER)  strlen(uid), (SQLPOINTER)  &uid_utf16, (SQLINTEGER)  sizeof(uid_utf16), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf16(henv, (SQLPOINTER)  pwd, (SQLINTEGER)  strlen(pwd), (SQLPOINTER)  &pwd_utf16, (SQLINTEGER)  sizeof(pwd_utf16), myccsid);
  sqlrc = custom_SQL400IgnoreNullToUtf16(henv, (SQLPOINTER) qual, (SQLINTEGER) strlen(qual), (SQLPOINTER) &qual_utf16, (SQLINTEGER)  sizeof(qual_utf16), myccsid);
  sqlrc = custom_SQL400ConnectBoth( db_utf16, uid_utf16, pwd_utf16, qual_utf16, ohnd, acommit, alibl, acurlib, 0, 1);
  return sqlrc;
}

/*
 * close connect
 */
SQLRETURN custom_SQL400CloseBoth(SQLHDBC hdbc) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  init_table_dtor(hdbc);
  sqlrc = SQLDisconnect(hdbc);
  sqlrc = SQLFreeConnect(hdbc);
  return sqlrc;
}

SQLRETURN custom_SQL400RollBackBoth(SQLHDBC hdbc) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  int auto_commit = 0;
  sqlrc = SQLGetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, &auto_commit, 0, NULL);
  if (!auto_commit) {
    sqlrc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
  }
  return sqlrc;
}

SQLRETURN custom_SQL400CloseAllStmts(SQLHDBC hdbc) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  SQLHSTMT hstmt = 0;
  /* free all statements */
  while (hstmt = init_table_find_stmt(hdbc)) {
    sqlrc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  }
  return SQL_SUCCESS;
}

SQLRETURN custom_SQL400Close(SQLHDBC hdbc) {
  int active = init_table_hash_active(hdbc, 0);
  /* always rollback (dtor language safe call) */
  SQLRETURN sqlrc = custom_SQL400RollBackBoth(hdbc);
  /* free all statements */
  sqlrc = custom_SQL400CloseAllStmts(hdbc);
  /* persistent connect only close with SQL400pClose */
  if (active) {
    return SQL_SUCCESS_WITH_INFO;
  }
  /* free connection */
  return custom_SQL400CloseBoth(hdbc);
}

SQLRETURN custom_SQL400pClose(SQLHDBC hdbc) {
  /* always rollback (dtor language safe call) */
  SQLRETURN sqlrc = custom_SQL400RollBackBoth(hdbc);
  /* free all statements */
  sqlrc = custom_SQL400CloseAllStmts(hdbc);
  /* free connection */
  return custom_SQL400CloseBoth(hdbc);
}

