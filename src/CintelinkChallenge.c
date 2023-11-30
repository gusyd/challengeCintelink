#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

//Global constants
#define INPUT_LENGHT 34
#define OUTPUT_LENGHT 70 //69 characters plus \n

/*Notas:
 * Arm casi siempre usa little endian
 * Año input es yyyy --- año output es yy
 * El maximo tiempo de POSIX es hasta el año 2038
 * strptime = string parser -- strftime = string formatter
 */


//4 transactions for testing, stored in an implicit char array for convenience:
const char input[] = {
		0x31, 0x30, 0x2F, 0x32, 0x33, 0x2F, 0x32, 0x30, 0x32, 0x33, 0x20, 0x31, 0x34, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x30, 0x55, 0x4A, 0x43, 0x20, 0x33, 0x31, 0x37, 0x31, 0x51, 0x37, 0x00, 0x00, 0x00, 0x67, 0x2B,
		0x30, 0x34, 0x2F, 0x31, 0x34, 0x2F, 0x32, 0x30, 0x32, 0x33, 0x20, 0x31, 0x30, 0x3A, 0x31, 0x31, 0x3A, 0x31, 0x32, 0x4D, 0x46, 0x4B, 0x20, 0x33, 0x31, 0x35, 0x37, 0x53, 0xA0, 0x0F, 0x00, 0x00, 0xCE, 0x56,

};

//To parse the input data, a struct is defined (with the output format as a reminder):
struct data {
    time_t timestamp;			//POSIX time in seconds
    uint16_t transaction_id;	//2 bytes for transaction ID
    char veh_registration[9];	//8 bytes for vehicle ID
    char product;				//1 byte for product type
    int32_t mililiters;			//4 bytes for milliliters
};
struct data transaction[100];	//a max of 100 transactions can be sent by the buffer

//Prototypes
int log_transactions(const char *data, char *log, size_t transaction_count);
int parseInput(const char *input, struct data *transaction, size_t totalTransactions);
int reArrange (struct data *transaction, size_t totalTransactions);


int main(void) {
	printf("%lu\n", sizeof(input));
	/*
	 calculate the number of transactions based on the lenght (assuming no incomplete transactions),
	 using size_t for compatibility. Unnecessary, the "transaction_count" has this info,
	 but for precaution.
	 */
	size_t totalTransactions = sizeof(input) / INPUT_LENGHT;
	struct data transaction[100];	//a max of 100 transactions can be sent by the buffer

	//parseInput read the data provided from the address and store it in structs
	parseInput(input, transaction, totalTransactions);

	//reArrange sorts the structs using bubble sorting
	reArrange(transaction, totalTransactions);
	return 0;
}//end main

int parseInput(const char *input, struct data *transaction, size_t totalTransactions){
	struct tm timestampInput;
	for(int i=0; i < totalTransactions; i++) {
		//Extracting the timestamp:
		char timestampConversion[20] = {0};	//char array to store the POSIX timestamp temporally
		memcpy(timestampConversion, &input[INPUT_LENGHT*i], 19);	//copying the first 19 bytes from the structure transaction
		strptime(timestampConversion, "%m/%d/%Y %T",&timestampInput);
		//To save the struct tm on my own structure, I have to convert it to time_t with mktime():
		transaction[i].timestamp = mktime(&timestampInput);
		//Extracting the vehicle ID:
		transaction[i].veh_registration[8] = 0;		//Null byte for array
		memcpy(transaction[i].veh_registration, &input[INPUT_LENGHT*i+19], 8);	//copying the 8 following bytes
		//Extracting the product:
		transaction[i].product = input[INPUT_LENGHT*i+19+8];	//copying one byte
		//Extracting the milliliters. I repeat the memcpy function:
		memcpy(&transaction[i].mililiters, &input[INPUT_LENGHT*i+19+8+1], 4);	//copying the 4 following bytes
		memcpy(&transaction[i].transaction_id, &input[INPUT_LENGHT*i+19+8+1+4], 2);	//copying the last 2 bytes
	}//end for

	//printf("Struct: Timestamp %i\nID: %su\nVehicle: %lu\nProduct: %c\nMilliliters: %i\n",transaction[1].timestamp, transaction[1].transaction_id, transaction[1].veh_registration, transaction[1].product, transaction[1].mililiters);
	return 0;
}//end parseInput

//void *memcpy(void *to, const void *from, size_t numBytes);
//char * strptime (const char *s, const char *fmt, struct tm *tp)

int reArrange (struct data *transaction, size_t totalTransactions){
	struct data sortingTransaction;		//temporal struct to sort the transactions
	for (int passes = 0; passes < totalTransactions -1; passes++){
		for (int comp = 0; comp < totalTransactions - passes -1; comp++){
			if (transaction[comp].timestamp > transaction[comp+1].timestamp){
				sortingTransaction = transaction[comp];
				transaction[comp] = transaction[comp+1];
				transaction[comp+1] = sortingTransaction;
			}//end if
		}//end internal loop
	}//end external loop

	return 0;
}




