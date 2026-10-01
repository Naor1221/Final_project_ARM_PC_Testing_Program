Pc testing program part of final project
Testing program sends struct packed struct which includes:
Test id(uint32_t), periphery to be checked(uint8_t), number of iterations(uint8_t), message length and messgae(uint8_t)
**Periphery to be checked is a uint8_t number,which acts like a register. 
It can select one periphery or more at the same time
bit 0 means Timer
bit 1 means UART
bit 2 means SPI 
bit 3 means I2C
bit 4 means ADC
Example for Timer and I2C on: 
9 -> 00001001
**Message is a fixed array of uint8_t in size of 256 bytes, acting like "buffer".
**Message length is measured by suitable uint8_t function called "my_strlen"
**Packed struct was selected for preventing padding.

Sending struct to board is limited up to 37 seconds(max time that measured for sending message in 255 bytes size and 255 iterations).
Response from the board will be received in another packed struct including:
Test id(uint32_t) and test result(uint8_t).
**Test result can be either 1(success) or 0xff(failure).

This response ,along with date and time length ,will be inserted into sqlite3 table named "test.db".
Sending and Receiving data(structs) is via UDP protocol as required:
Server port is 12345
Sending struct is always 263 bytes
Receiving struct is always 5 bytes

** Example of usage:
1)Create struct instace of mess_to_deliver(sending struct).
2)Initiat struct message with init_struct_message function.
3)Adding struct details with struct_details function. 
  **message can be written directly as an arrgumant, or be sent as a char * variable. 
4)Sending struct to board with send_struct function. 



