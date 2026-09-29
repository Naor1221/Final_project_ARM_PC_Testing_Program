#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
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

enum time_states{
    START=0,
    END
};


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
void get_date_and_time(time_t *t,char *result){
    struct tm* ptr;    
    *t=time(NULL);  
    ptr = localtime(t);
    strcpy(result,asctime(ptr));
}

time_t time_length(time_t end,time_t start){
    return end-start;
}
//simple suitable implementation for uint8_t of strlen
uint8_t my_strlen(uint8_t *str){
    uint8_t count=0;
    while(str[count]!='\0'){
        count++;
    }
    return count;
}
//simple suitable implementation for uint8_t of strcpy
void my_strcpy(uint8_t *dest,char *src){
    uint8_t idx=0;
    while(src[idx]!='\0'){
        dest[idx]=src[idx];
        idx++;
    }
}
void sql_func(int id,char* date,long time,int result){
    sqlite3 *db;
    char *errMsg = 0;
    int rc = sqlite3_open("test.db", &db);

    char sql[512];
    snprintf(sql,sizeof(sql),"CREATE TABLE IF NOT EXISTS table_name (Test_id INT, Date_and_time_to_UUT TEXT, Test_length INT,Result INT);"
                "INSERT INTO table_name VALUES (%u, '%s', %ld,%u);",id,date,time,result);
    
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        printf("Error : %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Done successfully!\n");
    }
    
    sqlite3_close(db);
}



void send_struct(struct mess_to_deliver *m1){
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
        //if answer is not received within 20 seconds,the sending is considered as a failure.
        while( ( ((cur_time=time(NULL))-start)<=20) ){
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
    struct mess_to_deliver m1;
    // struct answer a1;
    memset(m1.message,0,MAX_MESS_LEN);
    m1.test_id=1234;
    m1.per_tested=31;    
    my_strcpy(m1.message,"Message for uart!1234569875555555555555555555555555555555555555552222222222222222211999999999999999999966666666666666666666666");
    m1.mess_len=my_strlen(m1.message);
    m1.itr=150;
    send_struct(&m1);
    m1.test_id=456;
    m1.per_tested=32;
    my_strcpy(m1.message,"not working");
    m1.mess_len=my_strlen(m1.message);
    m1.itr=3;
    send_struct(&m1);

    // char date[30]={0};
    // time_t test_length=0;

    // int sock=socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

    // if (sock < 0) {
    //     printf("Error creating socket\n");
    //     return 1;
    // }
    // stm32_addr.sin_family=AF_INET;
    // stm32_addr.sin_port=htons(SERVER_PORT);
    // stm32_addr.sin_addr.s_addr=inet_addr("12.34.56.78");

    
    // ssize_t bytes_sent=sendto(sock,&m1,sizeof(m1),0,(struct sockaddr*)&stm32_addr,sizeof(stm32_addr));

    // if(bytes_sent<0){
    //     // printf("Data was not sent\n");
    //     perror("Data was not sent\n");
    // }
    // else{
    //     printf("Data was sent!\n");
    //     time_t start;
    //     get_date_and_time(&start,date);
    //     socklen_t adder_len=sizeof(stm32_addr);
    //     ssize_t bytes_received;
    //     time_t cur_time=0;
    //     //if answer is not received within 20 seconds,the sending is considered as a failure.
    //     while( ( ((cur_time=time(NULL))-start)<=20) ){
    //         bytes_received=recvfrom(sock,&a1,sizeof(a1),0,(struct sockaddr*)&stm32_addr,&adder_len);
    //         if(bytes_received==BYTES_RECV_LEN){
    //             break;
    //         }                    
    //     }
        
    //     if(bytes_received<0){
    //         printf("Recieved failed\n");
    //     }
    //     else if(bytes_received==BYTES_RECV_LEN){ 
    //         test_length=time_length(time(NULL),start);
    //         sql_func(a1.test_id,date,test_length,a1.test_result);

    //     }
    // }
    // close(sock);

    return 0;
}