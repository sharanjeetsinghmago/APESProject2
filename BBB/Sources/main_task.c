#include "main_task.h"
#include <mqueue.h>
#include <float.h>

#define HB_PORT_ADR 5000
#define IP_ADR      "127.0.0.1"

pthread_t comm_id, logger_id, alert_id, socket_id;

char* PATH = "/sys/class/leds/beaglebone:green:usr1/trigger";
char* LEDPATH = "/sys/class/leds/beaglebone:green:usr1/brightness";

void remove_trigger(void) {
        FILE* fp = NULL;
        if((fp = fopen(PATH, "r+")))
        {
                fwrite("none", 1, 4, fp);
                fclose(fp);
        }
        else
                printf("Error\n");
}


void LEDOff(void)
{
        FILE* LED = NULL;
        remove_trigger();
        if((LED = fopen(LEDPATH, "r+")))
        {
                fwrite("0", 1, 1, LED);
                fclose(LED);
        }
        else
                printf("LEDOff error\n");
}


void LEDOn(void)
{
        FILE* LED = NULL;
        remove_trigger();
        if((LED = fopen(LEDPATH, "r+")))
        {
                fwrite("1", 1, 1, LED);
                fclose(LED);
        }
        else
                printf("LEDOn error\n");
}

int comm_client()
{
  int client_socket = 0;
  struct sockaddr_in serv_addr = {0};
  const char* msg = "Comm Task Alive";
  payload_t ploadSend;
  int sent_b;
  size_t pload_size;
  char r_data[4] = {0};

  /* Enter the message into payload structure */
  memcpy(ploadSend.buf,msg,strlen(msg)+1);
  ploadSend.buf_len = strlen(ploadSend.buf);
  ploadSend.usrLED_OnOff = 1;

  /* create socket */
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    //printf("[Client] [ERROR] Socket creation Error\n");
    return -1;
  }
  else
    //printf("[Client] Socket Created Successfully\n");

  /* Fill the socket address structure */
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(HB_PORT_ADR);
      
  /* convert the IP ADDR to proper format */
  if(inet_pton(AF_INET, IP_ADR, &serv_addr.sin_addr)<=0) 
  {
    //printf("[Client] [ERROR] Address Conversion Error\n");
    return -1;
  }
  
  /* connect the socket before sending the data */
  if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    //printf("[Client] [ERROR] Connection Failed \n");
    return -1;
  }

  /*send the size of the incoming payload */
  pload_size = sizeof(ploadSend);
  sent_b = send(client_socket,&pload_size,sizeof(size_t), 0);
  //printf("[Client] Sent payload size: %d\n", pload_size);

  /*Sending the payload */
  sent_b = send(client_socket , (char*)&ploadSend , sizeof(ploadSend), 0 );
  /* check whether all the bytes are sent or not */
  if(sent_b < sizeof(ploadSend))
  {
    //printf("[Client] [ERROR] Complete data not sent\n");
    return 1;
  }
  
  /* display the date sent */
  //printf("[Client] Message sent from Client\n{\n Message: %s\n MessageLen: %d\n USRLED: %d\n}\n", \
                           ploadSend.buf, ploadSend.buf_len, ploadSend.usrLED_OnOff);
  
  /* read data sent by server */
  //read(client_socket, r_data, 4);
  //printf("[Client]  Message received from Server: %s\n",r_data);

  /* close socket */ 
  close(client_socket);
  
  //return 0;

}

void *func_comm()
{
	int i;
	int n,fd;
	mqd_t mq1;
	fd = uart_init();
	printf("[Communication Thread] Communication Thread Started\n");
   	     mq1 = mq_open("/my_queue",O_RDWR | O_CREAT, 0666, NULL);
	while(1)
	{
		// printf("Signal from Comm Task\n");
		// for(i=0;i<1000000000;i++);
		if((n=read(fd,&rec,sizeof(rec))) < 0)
	    {
    	         printf("\n[UART] Read Fail\n");
		}
		else if(n>0)
		{
  			//printf("y= %c\n", y);
  			//printf("Log ID = %d  ",rec.log_id );
  			//printf("%d\n",rec.log_level );
  			//printf("Data = %f\n\n",rec.data );
  			//printf("Data recv\n");
			if(rec.log_level == 1)
			{
				if(rec.log_id == 1)
				{
					alti=rec.data;	
					printf("Altitude = %f\n",rec.data);
					//printf("Alti = %f\n",alti);
					mq_send(mq1,(char *)&rec,sizeof(rec),1);
				}

				if(rec.log_id == 2)
				{
					humid=rec.data;
					printf("Humidity = %f\n",rec.data);
					//printf("Humid = %f\n\n",humid);
					mq_send(mq1,(char *)&rec,sizeof(rec),1);
				}
            }
            else if(rec.log_level == 2)
            {
            	if(rec.log_id == 1)
				{
					
					printf("[Altitude Task] Error in altitude task\n");
					mq_send(mq1,(char *)&rec,sizeof(rec),1);
				}

				if(rec.log_id == 2)
				{
					printf("[Humidity Task] Error in humidity task\n");
					mq_send(mq1,(char *)&rec,sizeof(rec),1);
				}	
            }           
		}
		comm_client();

	 }

	 printf("[Communication Thread] Communication Thread Finished\n");

}

float get_altitude()
{
	printf("Alti req = %f\n",alti);
	return alti;
}

void *func_logger()
{
	printf("[Logger Thread] Logger Thread Started\n");
    FILE *fptr;
    mqd_t my_queue; 
	log_packet given;
    fptr = fopen("log.txt","w");   //use logger_thread -> filename
	fprintf(fptr,"In logger task of BBB.\n");
    my_queue = mq_open("/my_queue",O_RDWR | O_CREAT, 0666, NULL);
    struct mq_attr *pact;
    pact = malloc(sizeof(struct mq_attr));
    mq_getattr(my_queue,pact);
    //fprintf(fptr,"Message queue initialised\n");
    printf("[Logger Thread] Message queue initialised.\n");
    fclose(fptr);
	while(1)
	{
        fptr = fopen("log.txt","a");
		mq_receive(my_queue,(char *)&given,pact->mq_msgsize,NULL);
		if(given.log_id == 1)
		{
	
        	 fprintf(fptr,"Timestamp:%s, Log level:%d, Log ID:%d, Altitude is: %f\n",given.timestamp,given.log_level,given.log_id,given.data);
		}
		else if (given.log_id == 2)
		{
	
        	 fprintf(fptr,"Timestamp:%s, Log level:%d, Log ID:%d, Humidity is: %f\n",given.timestamp,given.log_level,given.log_id,given.data);
		}
        fclose(fptr);
	}
    printf("[Logger Thread] Terminating message queue\n");
    printf("[Logger Thread] Logger Thread Finished\n");
        return fptr;
}

void *func_socket()
{
	printf("[Socket Thread] Socket Thread Started\n");
	socket_task();
	printf("[Socket Thread] Socket Thread Finished\n");
}

int alert_client()
{
  int client_socket = 0;
  struct sockaddr_in serv_addr = {0};
  const char* msg = "Alert Task Alive";
  payload_t ploadSend;
  int sent_b;
  size_t pload_size;
  char r_data[4] = {0};

  /* Enter the message into payload structure */
  memcpy(ploadSend.buf,msg,strlen(msg)+1);
  ploadSend.buf_len = strlen(ploadSend.buf);
  ploadSend.usrLED_OnOff = 1;

  /* create socket */
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    //printf("[Client] [ERROR] Socket creation Error\n");
    return -1;
  }
  else
    //printf("[Client] Socket Created Successfully\n");

  /* Fill the socket address structure */
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(HB_PORT_ADR);
      
  /* convert the IP ADDR to proper format */
  if(inet_pton(AF_INET, IP_ADR, &serv_addr.sin_addr)<=0) 
  {
    //printf("[Client] [ERROR] Address Conversion Error\n");
    return -1;
  }
  
  /* connect the socket before sending the data */
  if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    //printf("[Client] [ERROR] Connection Failed \n");
    return -1;
  }

  /*send the size of the incoming payload */
  pload_size = sizeof(ploadSend);
  sent_b = send(client_socket,&pload_size,sizeof(size_t), 0);
  //printf("[Client] Sent payload size: %d\n", pload_size);

  /*Sending the payload */
  sent_b = send(client_socket , (char*)&ploadSend , sizeof(ploadSend), 0 );
  /* check whether all the bytes are sent or not */
  if(sent_b < sizeof(ploadSend))
  {
    //printf("[Client] [ERROR] Complete data not sent\n");
    return 1;
  }
  
  /* display the date sent */
  //printf("[Client] Message sent from Client\n{\n Message: %s\n MessageLen: %d\n USRLED: %d\n}\n", \
                           ploadSend.buf, ploadSend.buf_len, ploadSend.usrLED_OnOff);
  
  /* read data sent by server */
  //read(client_socket, r_data, 4);
  //printf("[Client]  Message received from Server: %s\n",r_data);

  /* close socket */ 
  close(client_socket);
  
  //return 0;

}

void *func_alert()
{
	int alti_flag=0,humid_flag=0,p;
	printf("[Alert Thread] Alert Thread Started\n");
	
	while(1)
	{
		if(alti >= 8843)
		{
			//printf("[Alert Task]You have reached top of the world!!!\n");
			alti_flag = 1;
		}
		else
		{
			alti_flag = 0;
		}

		if(humid >= 20)
		{
			//printf("[Alert Task]Humidity less than 5!!!\n");
			humid_flag = 1;
			LEDOn();
		}
		else
		{
			humid_flag = 0;
			LEDOff();
		}

		alert_client();

		for(p=0;p<50000000;p++);
	}
	
	printf("[Alert Thread] Alert Thread Finished\n");
}

int check_status()
{
  struct sockaddr_in addr, peer_addr;
  int addr_len = sizeof(peer_addr);
  char rdbuff[1024] = {0};
  int server_socket, accepted_soc, opt = 1;
  int i = 0;
  payload_t *ploadptr;
  int read_b;
  size_t pload_len = 0;

  /* create socket */
  if((server_socket = socket(AF_INET,SOCK_STREAM,0)) == 0)
  {
    printf("[HBServer] [ERROR] Socket Creation Error\n");
    return 1;
  }
  else
    printf("[HBServer] Socket Created Successfully\n");

  /* set socket options */
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(opt), sizeof(opt)))
  {
    printf("[HBServer] [ERROR] Socket options set error\n");
    return 1;
  }

  /*Set the sockaddr_in structure */
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;  
  addr.sin_port = htons(HB_PORT_ADR);

  /*bind socket to a address */
  if((bind(server_socket,(struct sockaddr*)&addr, sizeof(addr))) < 0)
  {
    printf("[HBServer] [ERROR] Bind socket Error\n");
    return 1;
  }
  else
    printf("[HBServer] Socket binded Successfully\n");

  /* listen for connections*/
  if(listen(server_socket,5) < 0)
  {
    printf("[HBServer] [ERROR] Can't listen connection\n");
    return 1;
  }
while(1)
{
  /*accept connection */
  accepted_soc = accept(server_socket, (struct sockaddr*)&peer_addr,(socklen_t*)&addr_len);
  if(accepted_soc < 0)
  {
    printf("[HBServer] [ERROR] Can't accept connection\n");
    return 1;
  }

  // read payload length 
  read_b = read(accepted_soc, &pload_len, sizeof(size_t));
  if(read_b == sizeof(size_t))
  {
    //printf("[HBServer] Size of incoming payload: %d\n",pload_len);
  } 
  else
  {
    //printf("[HBServer] [ERROR] Invalid data\n");
    return 1;
  } 

  // read payload 
  while((read_b = read(accepted_soc, rdbuff+i, 1024)) < pload_len)
  {
    i+=read_b;  
  }
  ploadptr= (payload_t*)rdbuff;
  /* display data */
  printf("[HBServer]  Message: %s\n",ploadptr->buf);
  
  // send message from server to client 
  //send(accepted_soc , "ACK" , 4, 0);
  //printf("[HBServer] Message sent from Server: ACK\n");
}
  /*close socket */
  close(accepted_soc);

  return 0;
}

int startup_test()
{
	int x=1;

	if(pthread_create(&comm_id, NULL, func_comm, NULL) != 0)
	{
		x=0;
	}

	if(pthread_create(&alert_id, NULL, func_alert, NULL) != 0)
	{
		x=0;
	}

	if(pthread_create(&socket_id, NULL, func_socket, NULL) != 0)
	{
		x=0;
	}

	return x;
}

int main()
{

	int startup_check = startup_test();

	pthread_create(&logger_id, NULL, func_logger, NULL);

	
	if(startup_check == 1)
	{
		printf("Startup Success!!\n\n");
	}
	else if(startup_check == 0)
	{
		printf("\n<<<Startup Test Failed>>>\n\n");
		printf("[Main Task] Killing All Tasks\n");
		pthread_cancel(logger_id);
		pthread_cancel(comm_id);
		pthread_cancel(alert_id);
	}

	check_status();

	pthread_join(logger_id,NULL);
	pthread_join(comm_id,NULL);
	pthread_join(alert_id,NULL);
	pthread_exit(NULL);

	printf("Main Process Terminated\n");
	return 0;
}
	