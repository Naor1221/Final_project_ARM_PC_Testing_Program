#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sqlite3.h>

#define SERVER_PORT 12345
#define MAX_MESS_LEN 256
#define BYTES_RECV_LEN 5
#define MAX_WAITING_TIME 37
enum time_states{
    START=0,
    END
};
enum stat_struct{
    OK=0,
    BAD
};
volatile int stat=0;
static sqlite3 *db;
struct __attribute__((packed)) mess_to_deliver{
    uint32_t test_id;
    uint8_t per_tested;
    uint8_t itr;
    uint8_t mess_len;
    uint8_t message[MAX_MESS_LEN];
};
struct __attribute__((packed)) answer{
    uint32_t test_id;
    uint8_t test_result;
};

/* Assigns current date and time 
 * t      - Pointer to time_t variable to store current time
 * result - Pointer to char buffer where the formatted date/time string will be copied
 */
void get_date_and_time(time_t *t,char *result){
    struct tm* ptr;    
    *t=time(NULL);  
    ptr = localtime(t);
    strcpy(result,asctime(ptr));
}

/* Calculates the duration between two timestamps
 * end   - End time (later timestamp)
 * start - Start time (earlier timestamp)
 */
time_t time_length(time_t end,time_t start){
    return end-start;
}
/*Simple suitable implementation for uint8_t of strlen
* str - Pointer to the byte string terminated by '\0'
*/
uint8_t my_strlen(uint8_t *str){
    uint8_t count=0;
    while(str[count]!='\0'){
        count++;
    }
    return count;
}
/*Simple suitable implementation for uint8_t of strcpy
* dest - Destination uint8_t pointer
* src  - Source char string pointer
*/
void my_strcpy(uint8_t *dest,char *src){
    uint8_t idx=0;
    while(src[idx]!='\0'){
        dest[idx]=src[idx];
        idx++;
    }
}

/* SqLite callback function executed for each row returned by a SELECT query
 * data     - Generic user-defined pointer passed from sqlite3_exec
 * argc     - Number of columns in the current row
 * argv     - Array of strings representing column values
 * col_name - Array of strings representing column names
 */

static int callback(void *data, int argc, char **argv, char **col_name) {
    (void)data; //only to over come on Wunused-parmater warrning flag
    for (int i = 0; i < argc; i++) {
        printf("%s: %s | ", col_name[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

/* Prints all rows from an SQL database table to the console
 * table_name - Constant string representing the name of the table to print
 */

void print_table(const char *table_name) {
    char sql[128];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s;", table_name);
    char *err_msg = NULL;
    int rc=sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK) {
        printf("Error: %s\n", err_msg);
        sqlite3_free(err_msg); 
    }
    rc = sqlite3_exec(db, sql, callback, 0, &err_msg);    
    if (rc != SQLITE_OK) {
        printf("SQLite Error: %s\n", err_msg);
        sqlite3_free(err_msg); 
    }
    sqlite3_close(db);

}


/* Inserts  new values into the SQLite database table
 * id     - Test ID 
 * date   - String containing the date and time of the test
 * time   - Duration of the test in seconds
 * result - Result status code of the test
 * notice! although 
 */
void sql_func(uint32_t id,char* date,long time,uint8_t result){
    char *errMsg = 0;
    int rc = sqlite3_open("test.db", &db);
   
    char sql[512];
    snprintf(sql, sizeof(sql), "INSERT INTO table_name VALUES (%u, '%s', %ld, %u);", id, date, time, result);    
    rc = sqlite3_exec(db,sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        printf("Error : %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Done successfully!\n");
    }
    
    sqlite3_close(db);
}
/* Initializes message buffer with zeros for my_strlen functionality
 * message - Pointer to the message uint8_t array to be cleared
 */
void init_struct_message(uint8_t *message){
    memset(message,0,MAX_MESS_LEN-1);
}


/* Fills struct details to be sent and validates input parameters
 * m1      - Pointer to mess_to_deliver struct to be sent
 * id      - Test ID (will be cast to uint32_t)
 * per     - Periphery tested (will be cast to uint8_t)
 * message - Pointer to the message string
 * iter    - Number of iterations (will be cast to uint8_t)
 */
void struct_details(struct mess_to_deliver *m1,long id,int per,char *message,int iter){
    //making sure struct in OK stat
    if(stat!=OK){
        stat=OK;
    }
    //making sure legal values were accepted:
    if(message==NULL){
        printf("Error, message can not be NULL\n");
        stat=BAD;
        return;
    }
    
    else if(per>UINT8_MAX || per<0){
        printf("Error, periphery value must be uint8 type\n");
        stat=BAD;
        return;
    }
    else if (id>UINT32_MAX || id<0){
        printf("Error, id value must be uint32 type\n");
        stat=BAD;
        return;
    }
    else if(iter>UINT8_MAX || iter<0){
        printf("Error, iteration value must be uint8 type\n");
        stat=BAD;
        return;
    }
    int l_msg=my_strlen(m1->message);
    if(l_msg>MAX_MESS_LEN-1){
        printf("Error, max message length is %d\n",MAX_MESS_LEN-1);
        stat=BAD;
        return;
    }
    init_struct_message(m1->message);
    m1->test_id=(uint32_t)id;
    m1->per_tested=(uint8_t)per;
    my_strcpy(m1->message,message);
    m1->mess_len=my_strlen(m1->message);
    m1->itr=(uint8_t)iter;
}

/* Initializes the database file and creates the SQL table if it does not exist
 * Takes no arguments (void)
 */
void init_database(void){
    char *err_msg = NULL;
    int rc=sqlite3_open("test.db",&db);
    if(rc!=SQLITE_OK){
        printf("Error, cant open or create sql table\n");
        return;
    }
    rc=sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS table_name (Test_id INT, Date_and_time_to_UUT TEXT, Test_length INT, Result INT);", 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Table creation error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    sqlite3_close(db);
}

/* Sends the message structure over a UDP socket, waits for a response from the STM32,
 * if response returns before 37 seconds,it saves the result to the database, else it ignores.
 * m1 - Pointer to the fully initialized mess_to_deliver struct ready for transmission
 */
void send_struct(struct mess_to_deliver *m1){
    if(stat==BAD){
        printf("One of the arrguments is ilegal! cannot send struct\n");
        return;
    }
    if(m1->mess_len==0){
        printf("Error, message len cannot be zero\n");
        return;
    }
    struct sockaddr_in stm32_addr;
    struct answer a1;
    char date[30]={0};
    time_t test_length=0;

    int sock=socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

    if (sock < 0) {
        printf("Error creating socket\n");
        return;
    }
    stm32_addr.sin_family=AF_INET;
    stm32_addr.sin_port=htons(SERVER_PORT);
    stm32_addr.sin_addr.s_addr=inet_addr("12.34.56.78");

    
    ssize_t bytes_sent=sendto(sock,m1,sizeof(*m1),0,(struct sockaddr*)&stm32_addr,sizeof(stm32_addr));

    if(bytes_sent<0){
        perror("Data was not sent\n");
    }
    else{
        printf("Data was sent!\n");
        time_t start;
        get_date_and_time(&start,date);
        socklen_t adder_len=sizeof(stm32_addr);
        ssize_t bytes_received;
        time_t cur_time=0;
        //if answer is not received within 37 seconds,the sending is considered as a failure.
        while( ( ((cur_time=time(NULL))-start)<=MAX_WAITING_TIME) ){
            bytes_received=recvfrom(sock,&a1,sizeof(a1),0,(struct sockaddr*)&stm32_addr,&adder_len);
            if(bytes_received==BYTES_RECV_LEN){
                break;
            }                    
        }
        
        if(bytes_received<0){
            printf("Recieved failed\n");
        }
        else if(bytes_received==BYTES_RECV_LEN){ 
            test_length=time_length(time(NULL),start);
            sql_func(a1.test_id,date,test_length,a1.test_result);

        }
    }
    close(sock);
}



int main(void) {
    //code
    //allow watching in debuging
    setbuf(stdout, NULL);

    init_database();
    //example of using for m1 to max capacity(all peripharies and max length of 255 chars):
    struct mess_to_deliver m1;
    char mess[256];
    memset(mess,'A',MAX_MESS_LEN-2);
    mess[MAX_MESS_LEN-1]=0;  
    struct_details(&m1,123,31,mess,255);
    send_struct(&m1);
    //m1 can be reediting also:
    struct_details(&m1,555,1,"hi",8);
    send_struct(&m1);

    struct mess_to_deliver m2;
    struct_details(&m2,456,31,"message2",255);
    send_struct(&m2);

    
    //example of using print_table function:
    print_table("table_name");

    return 0;
}