#include <list>
#include "Account.h"
#include "PremiumAccount.h"
#include "SavingsAccount.h"

using namespace std;
$

CREATE STRUCT VIEW PremiumAccount ( 
       overlimit DOUBLE FROM overlimit
)$

CREATE VIRTUAL TABLE PremiumAccount
USING STRUCT VIEW PremiumAccount
WITH REGISTERED C TYPE PremiumAccount$

CREATE STRUCT VIEW SavingsAccount ( 
       due_date STRING FROM due_date
)$

CREATE VIRTUAL TABLE SavingsAccount
USING STRUCT VIEW SavingsAccount
WITH REGISTERED C TYPE SavingsAccount$

CREATE STRUCT VIEW Account ( 
       description STRING FROM description, 
       balance DOUBLE FROM get_balance(), 
       available_amount DOUBLE FROM available_amount, 
       binded_amount DOUBLE FROM binded_amount, 
       FOREIGN KEY(premiumaccount_id) FROM tuple_iter
       	       		    REFERENCES PremiumAccount POINTER,
       FOREIGN KEY(savingsaccount_id) FROM tuple_iter
       	       		    REFERENCES SavingsAccount POINTER
)$

CREATE VIRTUAL TABLE Account
USING STRUCT VIEW Account
WITH REGISTERED C NAME accounts 
WITH REGISTERED C TYPE list<Account*>$
