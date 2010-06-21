#include <stdio.h>
#include <string>
#include "stl_to_sql.h"
#include <pthread.h>
#include <vector>
#include "Account.h"

using namespace std;



void * thread_sqlite(void *data){
  const char **queries;
  queries = (const char **)sqlite3_malloc(sizeof(char *) * 1);
  int failure=0;
  queries[0] = "CREATE VIRTUAL TABLE account USING stl(account_no TEXT,balance FLOAT)";
  failure = register_table("foo.db", 1, queries, data, 1);
  printf("Thread sqlite returning..\n");
  sqlite3_free(queries);
  return (void *)failure;
}


/* comparison function for datastructure if needed
struct classcomp{
    bool operator() (const USER_CLASS& uc1, const USER_CLASS& uc2) const{
        return (uc1.get_known_type()<uc2.get_known_type());
    }
};
// in main: include classcomp in template arguments
*/


int main(){
  int re_sqlite;
  void *data;

  Account acc1("10068", 500.0);
  Account acc2("10234", 394.28);
  vector<Account> accounts;
  accounts.push_back(acc1);
  accounts.push_back(acc2);
  data = (void *)&accounts;


  // declare and fill datastructure;

  pthread_t sqlite_thread;
  re_sqlite = pthread_create(&sqlite_thread, NULL, thread_sqlite, data);
  pthread_join(sqlite_thread, NULL);
  printf("Thread sqlite returned %i\n", re_sqlite);
}
