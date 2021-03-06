#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sqlcli1.h>
#include "test.h"
#include "PaseCliAsync.h"

#define MAX_TEST_BUFFER 5000000

int main(int argc, char * argv[]) {
  SQLRETURN sqlrc = SQL_SUCCESS;
  char injson[MAX_TEST_BUFFER];
  int inlen = sizeof(injson);
  char outjson[MAX_TEST_BUFFER];
  int outlen = sizeof(outjson);
  char * ptr = NULL;
  int len = 0;
  char fp_buf[1024];
  char fp_file_json[1024];
  FILE *fp_json = NULL;
  char fp_file_exp[1024];
  FILE *fp_exp = NULL;
  char * fp_prefix = argv[1];
  char * bad = "_bad.";
  char * fixup = NULL;
  int fixlen = 0;
  pthread_t tid = 0;
  SQL400JsonStruct * tdata = NULL;
  SQLHANDLE hdbc = 0;
  char *db  = NULL;
  char *uid = NULL;
  char *pwd = NULL;
  char *libl  = NULL;
  char *curlib = NULL;
  char *trace  = NULL;
  int fast_wait = 0;
  /* profile db2 */
  db  = getenv(SQL_DB400);
  uid = getenv(SQL_UID400);
  pwd = getenv(SQL_PWD400);

  /* bad */
  if (argc < 2) {
    printf("%s j0000_parm_required (none specified)\n",argv[0]);
    printf("success (%d)\n",sqlrc);
    return;
  }
  /* fix up */
  fixup = strrchr(fp_prefix,'.');
  if (fixup && (!strcmp(fixup,".json") || !strcmp(fixup,".exp"))) {
    fixlen = strlen(fixup);
    memset(fixup,0,fixlen);
  }

  /* json test file */
  sprintf(fp_file_json,"%s.json",fp_prefix);
  fp_json = fopen(fp_file_json,"r");
  if (!fp_json) {
    printf("file '%s' not found\n",fp_file_json);
    sqlrc = SQL_ERROR;
  }
  memset(injson,0,sizeof(injson));
  while (sqlrc == SQL_SUCCESS && (fgets(fp_buf, sizeof(fp_buf), fp_json) != NULL)) {
    strcat(injson,fp_buf);
  } 

  /* exp test file */
  sprintf(fp_file_exp,"%s.exp",fp_prefix);
  fp_exp = fopen(fp_file_exp,"r");
  if (!fp_exp) {
    printf("file '%s' not found\n",fp_file_exp);
    sqlrc = SQL_ERROR;
  }

  /* test */
  printf("input(%d):\n%s\n",inlen,injson);

  /* sqlrc = SQLOverrideCCSID400( 1208 ); */

  /* json call (hdbc=0 - json handles connection) */
  /* SQL400 aggregate API -- convert db, uid, pwd to UTF8, set-up env, sys naming, server mode, etc. */
  sqlrc = SQL400ConnectUtf8(819, (SQLCHAR *) db, (SQLCHAR *) uid, (SQLCHAR *) pwd, &hdbc, SQL_TXN_NO_COMMIT, libl, curlib);
  tid = SQL400JsonAsync(hdbc, injson, inlen, outjson, outlen, NULL);
  printf("call(tid=%d)\n",tid);
  tdata = SQL400JsonJoin (tid, SQL400_FLAG_JOIN_NO_WAIT);
  while(!tdata) {
    printf("waiting(tid=%d)\n",tid);
    tdata = SQL400JsonJoin (tid, SQL400_FLAG_JOIN_NO_WAIT);
    if (!tdata) sleep(1);
  }
  free(tdata);
  printf("output(%d):\n%s\n\n",strlen(outjson),outjson);

  /* output */
  printf("result:\n");
  /* bad expected ? */
  if (sqlrc == SQL_ERROR) {
    ptr = strstr(fp_file_json,bad);
    if (ptr) {
      printf("expected bad (%d)\n",sqlrc);
      sqlrc = SQL_SUCCESS;
    }
  }
  while (sqlrc == SQL_SUCCESS && (fgets(fp_buf, sizeof(fp_buf), fp_exp) != NULL)) {
    len = strlen(fp_buf);
    if (!len) continue;
    fp_buf[len-1] = '\0';
    ptr = strstr(outjson,fp_buf);
    if (!ptr) {
      printf("fail missing (%s)\n",fp_buf);
      sqlrc = SQL_ERROR;
    }
  } 
  if (sqlrc == SQL_SUCCESS) {
    printf("success (%d)\n",sqlrc);
  } else {
    printf("fail (%d)\n",sqlrc);
  }

  sqlrc = SQLDisconnect(hdbc);
  sqlrc = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);

  return sqlrc;
}


