#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

//Global constants
#define INPUT_LENGHT 34
#define OUTPUT_LENGHT 70 //69 characters plus \n

/*Notas:
 * Arm casi siempre usa little endian. Arme los datos en little endian.
 * Año input es yyyy --- año output es yy
 * El maximo tiempo de POSIX es hasta el año 2038
 */


//4 transactions for testing, stored in an implicit char array for convenience:
const char data[] = {
		0x31, 0x30, 0x2F, 0x32, 0x33, 0x2F, 0x32, 0x30, 0x32, 0x33, 0x20, 0x31, 0x34, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x30, 0x55, 0x4A, 0x43, 0x20, 0x33, 0x31, 0x37, 0x31, 0x51, 0x37, 0x00, 0x00, 0x00, 0x67, 0x2B,
		0x30, 0x34, 0x2F, 0x31, 0x34, 0x2F, 0x32, 0x30, 0x32, 0x33, 0x20, 0x31, 0x30, 0x3A, 0x31, 0x31, 0x3A, 0x31, 0x32, 0x4D, 0x46, 0x4B, 0x20, 0x33, 0x31, 0x35, 0x37, 0x53, 0xA0, 0x0F, 0x00, 0x00, 0xCE, 0x56,
		0x30, 0x38, 0x2F, 0x30, 0x33, 0x2F, 0x32, 0x30, 0x32, 0x33, 0x20, 0x30, 0x38, 0x3A, 0x31, 0x30, 0x3A, 0x34, 0x30, 0x46, 0x4D, 0x5A, 0x20, 0x35, 0x37, 0x31, 0x32, 0x44, 0x20, 0xA1, 0x07, 0x00, 0x35, 0x82,
		0x30, 0x34, 0x2F, 0x31, 0x34, 0x2F, 0x32, 0x30, 0x32, 0x33, 0x20, 0x31, 0x30, 0x3A, 0x32, 0x30, 0x3A, 0x31, 0x32, 0x4D, 0x46, 0x4B, 0x20, 0x33, 0x31, 0x35, 0x37, 0x53, 0xA0, 0x0F, 0x00, 0x00, 0x9C, 0xAD
};

/*The data is:
 * Timestamp		Veh		Product	Milliliter	Trans ID
10/23/2023 14:02:00	UJC 3171	Q	00000055	11111
04/14/2023 10:11:12	MFK 3157	S	00004000	22222
08/03/2023 08:10:40	FMZ 5712	D	00500000	33333
04/14/2023 10:20:12	MFK 3157	S	00004000	44444
 */

char log[700] = {0}; //max 100 transactions of 70 bytes each.
//To parse the input data, a struct is defined (with the output format as a reminder):
struct transaction {
    time_t timestamp;			//POSIX time in seconds
    uint16_t transaction_id;	//2 bytes for transaction ID
    char veh_registration[9];	//8 bytes for vehicle ID
    char product;				//1 byte for product type
    int32_t mililiters;			//4 bytes for milliliters
};
struct transaction transactions[100];	//a max of 100 transactions can be sent by the buffer

//Prototypes
/*
 * Me resulto mas intuitivo desarrollar una funcion para cada accion necesaria,
 * por lo que no termine usando log_transactions(), pero dejo el prototipo declarado.
 * El programa fue probado y funciona con 4 registros.
 * Elr esto de los comentarios son en ingles.
 */
int log_transactions(const char *data, char *log, size_t transaction_count);
int parseInput(const char *data, struct transaction *transactions, size_t totalTransactions);
int reArrange (struct transaction *transactions, size_t totalTransactions);
int saveToBuffer (struct transaction *transactions, size_t totalTransactions, char *log);


int main(void) {
	int bytesBufferSalida = 0;
	printf("Bytes ingresados: %lu\n", sizeof(data));
	/*
	 calculate the number of transactions based on the lenght (assuming no incomplete transactions),
	 using size_t for compatibility. Unnecessary, the "transaction_count" has this info,
	 but for precaution.
	 */
	size_t totalTransactions = sizeof(data) / INPUT_LENGHT;
	struct transaction transactions[100] = {0};	//a max of 100 transactions can be sent by the buffer

	//parseInput read the data provided from the address and store it in structs
	parseInput(data, transactions, totalTransactions);

	//reArrange sorts the structs using bubble sorting
	reArrange(transactions, totalTransactions);

	//saveToBuffer send the information and formats the data
	bytesBufferSalida = saveToBuffer(transactions, totalTransactions, log);

	//outputing the values:
	printf("Bytes escritos: %i\n%s", bytesBufferSalida, log);
	return 0;
}//end main


int parseInput(const char *data, struct transaction *transactions, size_t totalTransactions){
	struct tm timestampInput = {0};		//Initialized empty to avoid issues on timestamp
	for(int i=0; i < totalTransactions; i++) {
		//Extracting the timestamp:
		char timestampConversion[20] = {0};	//char array to store the POSIX timestamp temporally
		memcpy(timestampConversion, &data[INPUT_LENGHT*i], 19);	//copying the first 19 bytes from the structure transaction
		strptime(timestampConversion, "%m/%d/%Y %T",&timestampInput);
		//To save the struct tm on my own structure, I have to convert it to time_t with mktime():
		transactions[i].timestamp = mktime(&timestampInput);
		//Extracting the vehicle ID:
		transactions[i].veh_registration[8] = 0;		//Null byte for array
		memcpy(transactions[i].veh_registration, &data[INPUT_LENGHT*i+19], 8);	//copying the 8 following bytes
		//Extracting the product:
		transactions[i].product = data[INPUT_LENGHT*i+19+8];	//copying one byte
		//Extracting the milliliters. I repeat the memcpy function:
		memcpy(&transactions[i].mililiters, &data[INPUT_LENGHT*i+19+8+1], 4);	//copying the 4 following bytes
		memcpy(&transactions[i].transaction_id, &data[INPUT_LENGHT*i+19+8+1+4], 2);	//copying the last 2 bytes
	}//end for
	return 0;
}//end parseInput

int reArrange (struct transaction *transactions, size_t totalTransactions){
	struct transaction sortingTransaction;		//temporal struct to sort the transactions
	for (int passes = 0; passes < totalTransactions -1; passes++){
		for (int comp = 0; comp < totalTransactions - passes -1; comp++){
			if (transactions[comp].timestamp > transactions[comp+1].timestamp){
				sortingTransaction = transactions[comp];
				transactions[comp] = transactions[comp+1];
				transactions[comp+1] = sortingTransaction;
			}//end if
		}//end internal loop
	}//end external loop
	return 0;
}//end reArrange

int saveToBuffer (struct transaction *transactions, size_t totalTransactions, char *log) {
	int writtenBytes = 0;		//Used to store the number of bytes written returned by the function sprintf
	for(int i = 0; i < totalTransactions; i++){
		//To convert to the format required, I must take the POSIX time and convert to a struct tm.
		struct tm fullDateTime = {0}; //initialize an empty struct to avoid conflicts.
		gmtime_r(&transactions[i].timestamp, &fullDateTime);
		char formattedDateTime[18] = {0}; //For the output I use yy instead of YYYY
		strftime(&formattedDateTime, 18, "%d/%m/%y %T", &fullDateTime);
		writtenBytes += sprintf(&log[OUTPUT_LENGHT*i],"[%s] id: %05"PRIu16", reg: %s, prod: %c, ltrs: %+08"PRId32"\n",	//05 and 08 to fill the missing characters
				formattedDateTime, transactions[i].transaction_id,
				transactions[i].veh_registration, transactions[i].product,
				transactions[i].mililiters);
	}//end for
	return writtenBytes;
}//end saveToBuffer

