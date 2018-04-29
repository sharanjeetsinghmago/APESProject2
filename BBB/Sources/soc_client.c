#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>


#define PORT_ADR    2000
#define IP_ADR      "127.0.0.1" /* Loppback IP Address*/

typedef struct
{
  char    buf[20];
  size_t  buf_len;
  bool    usrLED_OnOff;
}payload_t;


int main()
{
  int client_socket = 0;
  struct sockaddr_in serv_addr = {0};
  char msg[20] = "Message from Client";
  payload_t ploadSend;
  int sent_b;
  size_t pload_size;
  char r_data[4] = {0};

  /* Enter the message into payload structure 
  memcpy(ploadSend.buf,msg,strlen(msg)+1);
  ploadSend.buf_len = strlen(ploadSend.buf);
  ploadSend.usrLED_OnOff = 1;*/

while(1)
{
  /* create socket */
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("[Client] [ERROR] Socket creation Error\n");
    return -1;
  }
  else
    printf("[Client] Socket Created Successfully\n");

  /* Fill the socket address structure */
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT_ADR);
   



  /* convert the IP ADDR to proper format */
  if(inet_pton(AF_INET, IP_ADR, &serv_addr.sin_addr)<=0) 
  {
    printf("[Client] [ERROR] Address Conversion Error\n");
    return -1;
  }


  
  /* connect the socket before sending the data */
  if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("[Client] [ERROR] Connection Failed \n");
    return -1;
  }

  
    printf("\n\n Enter The API Message :");
  gets(&msg);

  

  printf("\n You entered : %s\n",msg);
  /* Enter the message into payload structure */
  memcpy(ploadSend.buf,msg,strlen(msg)+1);
  ploadSend.buf_len = strlen(ploadSend.buf);
  ploadSend.usrLED_OnOff = 1;

  /*send the size of the incoming payload */
  pload_size = sizeof(ploadSend);
  sent_b = send(client_socket,&pload_size,sizeof(size_t), 0);
  printf("[Client] Sent payload size: %d\n", pload_size);

  /*Sending the payload */
  sent_b = send(client_socket , (char*)&ploadSend , sizeof(ploadSend), 0 );
  /* check whether all the bytes are sent or not */
  if(sent_b < sizeof(ploadSend))
  {
    printf("[Client] [ERROR] Complete data not sent\n");
    return 1;
  }
  
  /* display the date sent */
  printf("[Client] Message sent from Client\n{\n Message: %s\n MessageLen: %d\n USRLED: %d\n}\n", \
                           ploadSend.buf, ploadSend.buf_len, ploadSend.usrLED_OnOff);
  
  /* read data sent by server */
  read(client_socket, r_data, 25);
  printf("[Client]  Message received from Server: %s\n",r_data);

  /* close socket */ 
  close(client_socket);
  }
  return 0;
}
