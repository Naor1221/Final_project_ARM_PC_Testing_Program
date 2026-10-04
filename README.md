PC testing program
Project Overview:
Program sends structure which includes: ID, priphery or pripheries to be checked, message and message length.
The structure is transfered via UDP protocol using usb cable to the board STM32F756ZG for making tests(will be explianed in Final_project_ARM_STM32F756ZG README).
The response from the board wiil be received up to 37 seconds, which is the time measured for maximal size of data length and iteration.
Only in case of receiving answer it first will be saved inside another structure, including ID and test result. 
And then the structure details will be transfered into sqlite3 data structure named "test.db". 
This data structure includes: ID, date and time of sending structure to board, time length and result(1 is Passed and 0xff is Failed).
That "test.db" will be printed on damaned(as will be explianed later).
Type of arrguments: 
In this project there was done using in uint32_t and uint8_t variables, for ensuring fixed size to be transfered indepently to compiler or proccesor. 

Hardware Requirements:
**Notice this code was developed on intel X86_64 CPU with virtual machine (Oracle virtual box), which runs linux mint Ubunto(64 bit). 
This code was not tested on other CPU architecture. 
Your pc should be able to have network appliction, enabling IPv4 protocol. 
Also it's highly recommended to have enough storage, since the program saves data on sqlite3 file ("test.db") on your pc. 

Software Requirements:
VSC text editor 1.106.2 version. 
gcc compiler 13.3 version. 
C99
**other version were not tested. 
It's highly recommended to use json attached files. 

Installition: 
Install VSC text editor, with gcc compiler with versions mentioned above. 
Opening the attached directory in git on VSC, includeing json files.
Entering the "Testing_program.c" file
*this file already includes the required libriries for running the code. 
Which are: 
#include <stdio.h>
#include <string.h> //vital for memset. 
#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sqlite3.h>

How to unistall: 
Deleting the directory which was downloaded on Github. 

How to use: 
For using this check program, you need first to run the code of STM32F756ZG, which is explained on Final_project_ARM_STM32F756ZG . 
**important details: 
The server port is 12345 
The board gateway address is 12.34.56.1/24 
The board IPv4 address is 12.34.56.78/24 

Assuming that board code is already running:
1)Connecting to board's network via getway address. 
2)Openning the "Testing_program.c" on and run init_database();
3)Create an instance of the struct "mess_to_deliver"
4)Running the function struct_details
5)Running the function send_struct
Optional(if you wish to printing the "test.db" table):
Running the function print_table



Example of using:
inside terminal: 
sudo ip addr add 12.34.56.1/24 dev (here write your own device name).
inside main:
int main(void){
 init_database();
 struct mess_to_deliver m1;
 struct_details(&m1,123,15,mess,255);
 send_struct(&m1);
 print_table("table_name");
 return 0;
}
**Explantion of each arrgumant is found above each function inside the "testing_program.c" file .


