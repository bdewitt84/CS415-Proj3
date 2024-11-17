#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<string.h>

#include"string_parser.h"
#include"account.h"

//  Functions you need
// ○ pthread_create()
// ○ pthread_exit()
// ○ pthread_join()
// ○ pthread_mutex_lock()
// ○ pthread_mutex_unlock()
// ○ pthread_cond_wait()
// ○ pthread_cond_broadcast() or pthread_cond_signal()
// ○ pthread_barrier_wait()
// ○ pthread_barrier_init()
// ○ sched_yield() (optional but strongly recommended)
// ○ mmap()
// ○ munmap()
// ○ memcpy()

int DEBUG = 0;


void validate_args(int argc, char* argv[0]);

void load_file(char* path, FILE** input_stream);

void close_file(FILE* input_stream);

void get_accounts_from_file(account** accounts, int* num_accounts, FILE* input_stream);

void get_transactions(command_line** transactions, int* num_transactions, FILE* input_stream);

void print_accounts(account** accounts, int num_accounts);

void print_account_balances(account** accounts, int num_accounts);

void print_transactions(command_line** transactions, int num_transactions);

void free_transactions(command_line** transactions, int num_transactions);

int get_account_id(account** accounts, int num_accounts, char* account_number);

int deposit(account** accounts, int id, double amount);

int transfer(account** accounts, int from_id, int to_id, double amount);

int withdraw(account** accounts, int id, double amount);

double check_balance(account** accounts, int id);

int check_password(account** accounts, int id, char* password);

void process_all_transactions(account** accounts, int num_accounts, FILE* input_stream); 


int main(int argc, char* argv[]) {

  validate_args(argc, argv);

  char* input_file_path = argv[1];
  FILE* input_stream = NULL;
  account* accounts = NULL;
  command_line* transactions = NULL;
  int num_accounts = 0;
  int num_transactions = 0;
  // char* input_buffer = NULL;
  // size_t len;

  load_file(input_file_path, &input_stream);
  get_accounts_from_file(&accounts, &num_accounts, input_stream);
  print_accounts(&accounts, num_accounts);
  // get_transactions(&transactions, &num_transactions, input_stream);
  // print_transactions(&transactions, num_transactions);
  
  

  process_all_transactions(&accounts, num_accounts, input_stream);
  print_account_balances(&accounts, num_accounts);

  close_file(input_stream);


  free(accounts);
  // free_transactions(transactions, num_transactions);
  // free(transactions);

  return 0;
}

void process_all_transactions(account** accounts, int num_accounts, FILE* input_stream) {
  char* input = NULL;
  size_t len;
  command_line transaction_buffer;

  
  while (getline(&input, &len, input_stream) != -1) {
    transaction_buffer = str_filler(input, " ");
  
    char* cmd = transaction_buffer.command_list[0];

    // T src_account password dest_account transfer_amount”
    if (strcmp(cmd, "T") == 0) {
      char* src_account = transaction_buffer.command_list[1];
      char* password = transaction_buffer.command_list[2];
      char* dest_account = transaction_buffer.command_list[3];
      double transfer_amount = atof(transaction_buffer.command_list[4]);

      int src_id = get_account_id(accounts, num_accounts, src_account);
      int dest_id = get_account_id(accounts, num_accounts, dest_account);

      int error = 0;
      if (check_password(accounts, src_id, password) != 0) {
        // printf("transfer failed: incorrect password\n");
      }
      else {
        transfer(accounts, src_id, dest_id, transfer_amount);
        // printf("Transfer from %s to %s in the amount of %lf\n", src_account, dest_account, transfer_amount);
      }
  
    } else if (strcmp(cmd, "C") == 0) {
      // C account_num password
      char* src_account = transaction_buffer.command_list[1];
      char* password = transaction_buffer.command_list[2];

      int src_id = get_account_id(accounts, num_accounts, src_account);

      if (check_password(accounts, src_id, password) != 0) {
        // printf("check balance failed: incorrect password\n");
      } else {
        // printf("account balance for %s:\t %lf\n", src_account, check_balance(accounts, src_id));
      }

    } else if (strcmp(cmd, "D") == 0) {
      // D account_num password amount
      char* src_account = transaction_buffer.command_list[1];
      char* password = transaction_buffer.command_list[2];
      double amount = atof(transaction_buffer.command_list[3]);
      
      int src_id = get_account_id(accounts, num_accounts, src_account);

      if (check_password(accounts, src_id, password) != 0) {
        // printf("deposit failed: incorrect password\n");
      } else {
        deposit(accounts, src_id, amount);
        // printf("Deposit to %s in the amount of %lf\n", src_account, amount);
      }

    } else if (strcmp(cmd, "W") == 0) {
      // W account_num password amount
      char* src_account = transaction_buffer.command_list[1];
      char* password = transaction_buffer.command_list[2];
      double amount = atof(transaction_buffer.command_list[3]);

      int src_id = get_account_id(accounts, num_accounts, src_account);

      if (check_password(accounts, src_id, password) != 0) {
        // printf("withdraw failed: incorrect password\n");
      } else {
        withdraw(accounts, src_id, amount);
        // printf("withdraw from %s in the amount of %lf\n", src_account, amount);
      }

    } else {
      printf("Invalid transaction\n");
      exit(EXIT_FAILURE);
    }
    free_command_line(&transaction_buffer);
  }
  
    free(input);
}


void* process_transaction (void* arg);
// ■ This function will be run by a worker thread to handle the transaction
// requests assigned to them.
// ■ This function will return nothing (optional)
// ■ This function will take in one argument, the argument has to be one of the
// following types, char**, command_line*, struct (customized).


void* update_balance (void* arg);
// ■ This function will be run by a dedicated bank thread to update each
// account’s balance base on their reward rate and transaction tracker.
// ■ This function will return the number of times it had to update each account
// ■ This function does not take any argument


void validate_args(int argc, char* argv[0]) {
  if (argc != 2) {  
    printf("Usage:  %s <PATH>\n\n \t <PATH> \t Path to input file\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}


void load_file(char* input_file_path, FILE** input_stream) {
  *input_stream = fopen(input_file_path, "r");
  printf("%s\n", input_file_path);
  if (*input_stream == NULL) {
    perror("failed to open file");
    exit(EXIT_FAILURE);
  }
}

void close_file(FILE* input_stream) {
  if (fclose(input_stream) == EOF) {
    perror("error closing file");
    exit(EXIT_FAILURE);
  }
}


void get_accounts_from_file(account** accounts, int* num_accounts, FILE* input_stream) {

  printf("Getting accounts...");

  
  char* input = NULL;
  size_t len;

  getline(&input, &len, input_stream);

  *num_accounts = atoi(input);

  *accounts = malloc(sizeof(account) * (*num_accounts));

  if (*accounts == NULL) {
    fprintf(stderr, "\nMemory allocation failed\n");
    exit(EXIT_FAILURE);
  }
 
  for (int i = 0; i < *num_accounts; ++i){
    account new_account;

    // skip index
    getline(&input, &len, input_stream);

    // Get account number
    getline(&input, &len, input_stream);
    strncpy(new_account.account_number, input, sizeof(new_account.account_number) - 1);
    new_account.account_number[sizeof(new_account.account_number) - 1] = '\0';

    // Get password
    getline(&input, &len, input_stream);
    strncpy(new_account.password, input, sizeof(new_account.password) - 1);
    new_account.password[sizeof(new_account.password) - 1] = '\0';

    // get balance
    getline(&input, &len, input_stream);
    new_account.balance = atof(input);

    // get reward rate
    getline(&input, &len, input_stream);
    new_account.reward_rate = atof(input);

    // init mutex
    // pthread_mutex_init(new_account.ac_lock, NULL);

    (*accounts)[i] = new_account;

    
  }
  free(input);
  printf("Done.\n");
}


void print_accounts(account** accounts, int num_accounts) {
  for (int i = 0; i < num_accounts; ++i) {
    printf("ENTRY %d\n", i);
    printf("act number: \t %s \n",
     (*accounts)[i].account_number);
    printf("password: \t %s \n",
     (*accounts)[i].password);
    printf("balance: \t %lf \n",
     (*accounts)[i].balance);
    printf("reward rate: \t %lf \n\n",
     (*accounts)[i].reward_rate);
  }
  return;
}

void print_account_balances(account** accounts, int num_accounts) {
  for (int i = 0; i < num_accounts; ++i) {
    printf("%d balance: %lf\n", i, (*accounts)[i].balance);
  }
}

void get_transactions(command_line** transactions, int* num_transactions, FILE* input_stream) {
  printf("Getting transactions...");
  char* input = NULL;
  size_t len;
  int count = 0;
  while (getline(&input, &len, input_stream) != -1) {
    ++(*num_transactions);
  }
  
  
  printf("num transactions: %d\n", *num_transactions);
  free(input); 
  printf("Done\n");
}

void print_transactions(command_line** transactions, int num_transactions) {
  printf("print_transactions not implemented\n");
  return;
}

void free_transactions(command_line** transactions, int num_transactions) {
  for (int i = 0; i < num_transactions; ++i) {
    free_command_line(transactions[i]);
  }
  return;
}

int get_account_id(account** accounts, int num_accounts, char* account_number) {
  account result;
  
  for (int i = 0; i < num_accounts; ++i) {
    if (strcmp((*accounts)[i].account_number, account_number) == 0) {
      return i;
    }
  }
  return -1;
}

// returns 0 if password matches
int check_password(account** accounts, int id, char* password) {
  return strcmp((*accounts)[id].password, password);
}

int deposit(account** accounts, int id, double amount) {
  // TODO: Add reward tracker
  (*accounts)[id].balance += amount;
  return 0;
}

int transfer(account** accounts, int from_id, int to_id, double amount) {
  // TODO: add reward tracker
  (*accounts)[from_id].balance -= amount;
  (*accounts)[to_id].balance += amount;
  return 0;
}

int withdraw(account** accounts, int id, double amount) {
  // TODO: add reward tracker
  (*accounts)[id].balance -= amount;
  return 0;
}

double check_balance(account** accounts, int id) {
  return (*accounts)[id].balance;
}

