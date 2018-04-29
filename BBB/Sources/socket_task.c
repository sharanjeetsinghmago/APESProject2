#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
//#include "main_task.h"


extern float alti,humid;

#define PORT_ADR    2000

typedef struct
{
  char    buf[20];
  int  buf_len;
  bool    usrLED_OnOff;
}payload_t;


int socket_task()
{
  struct sockaddr_in addr, peer_addr;
  int addr_len = sizeof(peer_addr);
  char rdbuff[1024] = {0};
  int server_socket, accepted_soc, opt = 1;
  int i = 0;
  payload_t *ploadptr;
  int read_b;
  int pload_len = 0;
  char ackbuf[50];
  float temp,lumen;

 
  /* create socket */
  if((server_socket = socket(AF_INET,SOCK_STREAM,0)) == 0)
  {
    printf("[Server] [ERROR] Socket Creation Error\n");
    return 1;
  }
  else
    printf("[Server] Socket Created Successfully\n");

  /* set socket options */
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(opt), sizeof(opt)))
  {
    printf("[Server] [ERROR] Socket options set error\n");
    return 1;
  }

  /*Set the sockaddr_in structure */
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;	
  addr.sin_port = htons(PORT_ADR);

  /*bind socket to a address */
  if((bind(server_socket,(struct sockaddr*)&addr, sizeof(addr))) < 0)
  {
    printf("[Server] [ERROR] Bind socket Error\n");
    return 1;
  }
  else
    printf("[Server] Socket binded Successfully\n");



  /* listen for connections*/
  if(listen(server_socket,5) < 0)
  {
    printf("[Server] [ERROR] Can't listen connection\n");
    return 1;
  }
while(1)
 {
  /*accept connection */
  accepted_soc = accept(server_socket, (struct sockaddr*)&peer_addr,(socklen_t*)&addr_len);
  if(accepted_soc < 0)
  {
    printf("[Server] [ERROR] Can't accept connection\n");
    return 1;
  }

  /* read payload length */
  read_b = read(accepted_soc, &pload_len, sizeof(int));
  if(read_b == sizeof(int))
  {
    printf("[Server] Size of incoming payload: %d\n",pload_len);
  }	
  else
  {
    printf("[Server] [ERROR] Invalid data\n");
    return 1;
  } 

  /* read payload */
  while((read_b = read(accepted_soc, rdbuff+i, 1024)) < pload_len)
  {
    i+=read_b;	
  }
  ploadptr= (payload_t*)rdbuff;
  /* display data */
  printf("[Server] Message Recvd from Client\n{\n Message:%s\n MessageLen:%d\n USRLED:%d\n}\n",ploadptr->buf, ploadptr->buf_len, ploadptr->usrLED_OnOff);
  
  // printf("ID = %d\n", rec.log_id);
  // printf("Data = %f\n", rec.data);

  if(strcmp(ploadptr->buf,"get_humidity")==0)
  {
  	printf("You Want Temperature in Celcius\n");
   // temp = read_temp_data_reg(0);
    //printf("Temp in cel %f\n",temp );
    snprintf(ackbuf, 50, "Humidity = %f",humid);
    send(accepted_soc , ackbuf , 50, 0);
  }
  else if(strcmp(ploadptr->buf,"get_altitude")==0)
  {
  	printf("You Want Temperature in Kelvin\n");
   // temp = read_temp_data_reg(1);
    //temp = get_altitude();
    //printf("Temp in cel %f\n",temp );
    snprintf(ackbuf, 50, " Altitude = %f",alti);
    send(accepted_soc , ackbuf , 50, 0);
  }
  else if(strcmp(ploadptr->buf,"get_temp_fahrenheit")==0)
  {
  	printf("You Want Temperature in Fahrenheit\n");
   // temp = read_temp_data_reg(2);
    //printf("Temp in cel %f\n",temp );
    snprintf(ackbuf, 50, "Temp in celcius %f",temp);
    send(accepted_soc , ackbuf , 50, 0);
  }
  else if(strcmp(ploadptr->buf,"isitday")==0)
  {
  	printf("Day ? Don't Know!!\n");
   // lumen = get_lux();
    if(lumen < 10)
    {
      send(accepted_soc , "No, it is Night" , 50, 0);
    }
    else
    {
      send(accepted_soc , "Yes, it is Day" , 50, 0);
    }
  }
  else if(strcmp(ploadptr->buf,"isitnight")==0)
  {
  	printf("Night ? Don't Know!!\n");
   // lumen = get_lux();
    if(lumen < 10)
    {
      send(accepted_soc , "Yes, it is Night" , 50, 0);
    }
    else
    {
      send(accepted_soc , "No, it is Day" , 50, 0);
    }
  }
  else if(strcmp(ploadptr->buf,"get_lux")==0)
  {
    printf("You want the lumen value!!\n");
    //lumen = get_lux();
    printf("Lux value %f\n",lumen );
    snprintf(ackbuf, 50, "Lux Value is %f",lumen);
    send(accepted_soc , ackbuf , 50, 0);

  }else
  {
  	printf("I Don't Understand !!");
    send(accepted_soc , "I Don't Understand !!" , 50, 0);
    printf("[Socket] Alti = %f\n",alti);
  }

  /* send message from server to client */
//  send(accepted_soc , "ACK" , 4, 0);
//  printf("[Server] Message sent from Server: ACK\n");

  /*close socket */
  close(accepted_soc);
	}
  return 0;
}
