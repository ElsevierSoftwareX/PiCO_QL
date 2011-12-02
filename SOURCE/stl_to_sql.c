#include "stl_to_sql.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


#define DEBUGGING


// construct the sql query
void create(sqlite3 *db, int argc, const char * const * as, char *q){ 
  int i;
  q[0] = '\0';
  strcat(q,"CREATE TABLE ");
  strcat(q, as[2]);
  strcat(q,"(");
  for(i = 3; i < argc; i++){
    strcat(q,as[i]);
    if( i+1 < argc ) strcat(q,",");
    else strcat(q,");");
  }
  q[strlen(q)] = '\0';
#ifdef DEBUGGING
  printf("query is: %s with length %i \n", q, (int)strlen(q));
#endif
}

int init_vtable(int iscreate, sqlite3 *db, void *paux, int argc, 
		const char * const * argv, sqlite3_vtab **ppVtab,
		char **pzErr){
  stlTable *stl;
  int nDb, nName, nByte, nCol, nString, count, i, re;
  char *temp;
  nDb = (int)strlen(argv[1]) + 1;
  nName = (int)strlen(argv[2]) + 1;
  nString=0;
  // explore fts3 way
  for(i=3; i<argc; i++){
    nString += (int)strlen(argv[i]) + 1;
  }
  nCol = argc - 3;
  assert( nCol > 0 );
  nByte = sizeof(stlTable) + nCol * sizeof(char *) + nDb + nName + nString;
  stl = (stlTable *)sqlite3_malloc(nByte);
  if( stl == 0 ){
    return SQLITE_NOMEM;
  }
  memset(stl, 0, nByte);
  stl->db = db;
  int q = 0;
  char str[1024];
  stl->nColumn = nCol;
  stl->azColumn=(char **)&stl[1];
  temp=(char *)&stl->azColumn[nCol];

  stl->zName = temp;
  memcpy(temp, argv[2], nName);
  temp += nName;
  stl->zDb = temp;
  memcpy(temp, argv[1], nDb);
  temp += nDb;

  int n;
  for(i=3; i<argc; i++){
    n = (int)strlen(argv[i]) + 1;
    memcpy(temp, argv[i], n);
    stl->azColumn[i-3] = temp;
    temp += n;
    assert( temp <= &((char *)stl)[nByte] );
  }

  char query[arrange_size(argc, argv)];
  create(db, argc, argv, query);
#ifdef DEBUGGING
  printf("query is: %s \n", query);
#endif
  if( !(*pzErr) ){
    int output = sqlite3_declare_vtab(db, query);
    if( output == 1 ){
      *pzErr = sqlite3_mprintf("Error while declaring the virtual table\n");
      printf("%s \n", *pzErr);
      return SQLITE_ERROR;
    }else if( output==0 ){
      *ppVtab = &stl->vtab;
      if ( (re = realloc_carrier(*ppVtab, paux, argv[2], pzErr)) != SQLITE_OK )
	return re;
#ifdef DEBUGGING
      printf("Virtual table declared successfully\n");
#endif
      return SQLITE_OK;
    }
  }else{
    *pzErr = sqlite3_mprintf("Unknown error");
    printf("%s \n", *pzErr);
    return SQLITE_ERROR;
  } 
}

//xConnect
int connect_vtable(sqlite3 *db, void *paux, int argc,
		   const char * const * argv, sqlite3_vtab **ppVtab,
		   char **pzErr){
#ifdef DEBUGGING
  printf("Connecting vtable %s \n\n", argv[2]);
#endif
  return init_vtable(0, db, paux, argc, argv, ppVtab, pzErr);
}

// xCreate
int create_vtable(sqlite3 *db, void *paux, int argc,
		  const char * const * argv, sqlite3_vtab **ppVtab,
		  char **pzErr){
#ifdef DEBUGGING
  printf("Creating vtable %s \n\n", argv[2]);
#endif
  return init_vtable(1, db, paux, argc, argv, ppVtab, pzErr);
}

int update_vtable(sqlite3_vtab *pVtab, int argc, sqlite3_value **argv,
		  sqlite_int64 *pRowid){
}

// xDestroy
int destroy_vtable(sqlite3_vtab *ppVtab){
  stlTable *st = (stlTable *)ppVtab;
#ifdef DEBUGGING
  printf("Destroying vtable %s \n\n", st->zName);
#endif
  int result;
  // Need to destroy additional structures. So far none.
  result = disconnect_vtable(ppVtab);
  return result;
}

// xDisconnect
int disconnect_vtable(sqlite3_vtab *ppVtab){
  stlTable *s=(stlTable *)ppVtab;
  dsData *d = (dsData *)s->data;
#ifdef DEBUGGING
  printf("Disconnecting vtable %s \n\n", s->zName);
#endif
  if ( d->parents != NULL )
    sqlite3_free(d->parents);
  if ( d->nntv != NULL )
    sqlite3_free(d->nntv);
  sqlite3_free(s);
  return SQLITE_OK;
}

// xBestindex
int bestindex_vtable(sqlite3_vtab *pVtab, sqlite3_index_info *pInfo){
  stlTable *st=(stlTable *)pVtab;
  if ( pInfo->nConstraint>0 ){            // no constraint no setting up
    char op, iCol;
    char nidxStr[pInfo->nConstraint*2+1];
    memset(nidxStr, 0, sizeof(nidxStr));

    assert( pInfo->idxStr==0 );
    int i, j=0;
    for(i=0; i<pInfo->nConstraint; i++){
      struct sqlite3_index_constraint *pCons = &pInfo->aConstraint[i];
      if( pCons->usable==0 ) continue;
      switch ( pCons->op ){
      case SQLITE_INDEX_CONSTRAINT_LT:
	op='A'; 
	break;
      case SQLITE_INDEX_CONSTRAINT_LE:  
	op='B'; 
	break;
      case SQLITE_INDEX_CONSTRAINT_EQ:  
	op='C'; 
	break;
      case SQLITE_INDEX_CONSTRAINT_GE:  
	op='D'; 
	break;
      case SQLITE_INDEX_CONSTRAINT_GT:  
	op='E'; 
	break;
	//    case SQLITE_INDEX_CONSTRAINT_MATCH: nidxStr[i]="F"; break;
      }
      iCol = pCons->iColumn - 1 + 'a';

      assert( j<sizeof(nidxStr)-1 );
      nidxStr[j++] = op;
      nidxStr[j++] = iCol;
      //    UNUSED_PARAMETER(pVtab);
      pInfo->aConstraintUsage[i].argvIndex = i+1;
      pInfo->aConstraintUsage[i].omit = 1;
    }
    pInfo->needToFreeIdxStr = 1;
    if( (j>0) && 0==(pInfo->idxStr=sqlite3_mprintf("%s", nidxStr)) )
      return SQLITE_NOMEM;
  }
  return SQLITE_OK;
}

// xFilter
int filter_vtable(sqlite3_vtab_cursor *cur, int idxNum, const char *idxStr,
		  int argc, sqlite3_value **argv){
  stlTableCursor *stc=(stlTableCursor *)cur;
  int i, j=0, re = 0;
  char *constr = (char *)sqlite3_malloc(sizeof(char) * 3);
  memset(constr, 0, sizeof(constr));
  // Initialize size of resultset data structure.
  stc->size = 0;

  // Initial cursor position.
  stc->current = -1;

  // In case of a join, xfilter will be called many times, x times for x 
  // eligible rows of the paired table in this case isEof will be set to 
  // terminate at row level and has to be reset to allow matching all 
  // eligible rows.
  stc->isEof = 0;

  // First_constr is used to signal that the current constr encountered is the
  // first (value 1) or not (value 0).
  stc->first_constr = 1;

  //Empty where clause.
  if( argc==0 ) {
    if ( (re = search(cur, NULL, NULL)) != 0 )
      return re;
  }else{
    for(i=0; i<argc; i++) {
      constr[0] = idxStr[j++];
      constr[1] = idxStr[j++];
      constr[2] = '\0';
      if ( (re = search(cur, constr, argv[i])) != 0 )
	return re;
    }
  }
  sqlite3_free(constr);
  return next_vtable(cur);
}

//xNext
int next_vtable(sqlite3_vtab_cursor *cur){
  int re;
  stlTable *st=(stlTable *)cur->pVtab;
  stlTableCursor *stc=(stlTableCursor *)cur;
  stc->current++;
#ifdef DEBUGGING
  printf("Table %s, now stc->current: %i \n\n", st->zName, stc->current);
#endif
  if ( stc->current >= stc->size )
    stc->isEof = 1;
  return SQLITE_OK;
}


// xOpen
int open_vtable(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCsr){
  stlTable *st=(stlTable *)pVtab;
  int re;
#ifdef DEBUGGING
  printf("Opening vtable %s\n\n", st->zName);
#endif

  sqlite3_vtab_cursor *pCsr;               /* Allocated cursor */

  *ppCsr = pCsr = 
    (sqlite3_vtab_cursor *)sqlite3_malloc(sizeof(stlTableCursor));
  if( !pCsr ){
    return SQLITE_NOMEM;
  }
  stlTableCursor *stc = (stlTableCursor *)pCsr;
  memset(pCsr, 0, sizeof(stlTableCursor));
#ifdef DEBUGGING
  printf("ppCsr = %lx, pCsr = %lx \n", (long unsigned int)ppCsr, (long unsigned int)pCsr);
#endif
  return SQLITE_OK;
}

//xColumn
int column_vtable(sqlite3_vtab_cursor *cur, sqlite3_context *con, int n){
  return retrieve(cur, n, con);
}

//xRowid
int rowid_vtable(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
}

//xClose
int close_vtable(sqlite3_vtab_cursor *cur){
  stlTable *st=(stlTable *)cur->pVtab;
  stlTableCursor *stc=(stlTableCursor *)cur;
#ifdef DEBUGGING
  printf("Closing vtable %s \n\n",st->zName);
#endif
  free_resultset(cur);
  reset_nonNative(cur);
  sqlite3_free(stc);
  return SQLITE_OK;
}

//xEof
int eof_vtable(sqlite3_vtab_cursor *cur){
  return ((stlTableCursor *)cur)->isEof;
}

// Fill module's function pointers with implemented callback functions.
void fill_module(sqlite3_module *m){
  m->iVersion = 1;
  m->xCreate = create_vtable;
  m->xConnect = connect_vtable;
  m->xBestIndex = bestindex_vtable;
  m->xDisconnect = disconnect_vtable;
  m->xDestroy = destroy_vtable;
  m->xOpen = open_vtable;
  m->xClose = close_vtable;
  m->xEof = eof_vtable;
  m->xFilter = filter_vtable;
  m->xNext = next_vtable;
  m->xColumn = column_vtable;
  m->xRowid = rowid_vtable;
  m->xUpdate = 0;
  m->xFindFunction = 0;
  m->xBegin = 0;
  m->xSync = 0;
  m->xCommit = 0;
  m->xRollback = 0;
  m->xRename = 0;
}

// Estimates the query length by counting the length of the parameters passed.
int arrange_size(int argc, const char * const * argv){
  int length = 28;          // Length of standard keywords of sql queries.
  int i;
  for(i=0; i<argc; i++){
    // + length for all keywords except db_name.
    // + 1 for following identifier.
    if( i!=1 ) length += strlen(argv[i]) + 1;
  }
  // Sentinel character.
  length += 1;
#ifdef DEBUGGING
  printf("length is %i \n",length);
#endif
  return length;
}
