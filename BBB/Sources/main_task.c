#include "main_task.h"
#include <mqueue.h>
#include <float.h>

pthread_t comm_id, logger_id, alert_id;

void *func_comm()
{
	int i;
	int n,fd;
	mqd_t mq1;
	fd = uart_init();
	printf("Communication Task Started\n");
        mq1 = mq_open("/my_queue",O_RDWR | O_CREAT, 0666, NULL);
	while(1)
	{
		// printf("Signal from Comm Task\n");
		// for(i=0;i<1000000000;i++);
		if((n=read(fd,&rec,sizeof(rec))) < 0)
	    {
    	         //printf("\nRead Fail\n");
		}
		else if(n>0)
		{
  			//printf("y= %c\n", y);
  			//printf("Log ID = %d  ",rec.log_id );
  			//printf("%d\n",rec.log_level );
  			//printf("Data = %f\n\n",rec.data );
  			//printf("Data recv\n");

			if(rec.log_id == 1)
			{
				alti=rec.data;	
				printf("Altitude = %f\n",rec.data);
				printf("Alti = %f\n",alti);
			}

			if(rec.log_id == 2)
			{
				humid=rec.data;
				printf("Humidity = %f\n",rec.data);
				printf("Humid = %f\n\n",humid);
			}
                        mq_send(mq1,(char *)&rec,sizeof(rec),1);
		}

	 }

}

float get_altitude()
{
	printf("Alti req = %f\n",alti);
	return alti;
}

void *func_logger()
{
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
	else if (given.log_id == 2);
	{
	
         fprintf(fptr,"Timestamp:%s, Log level:%d, Log ID:%d, Humidity is: %f\n",given.timestamp,given.log_level,given.log_id,given.data);
	}
        fclose(fptr);
}
        printf("[Logger Thread] Terminating message queue\n");
        return fptr;
}

void *func_alert()
{
	socket_task();
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


	pthread_join(logger_id,NULL);
	pthread_join(comm_id,NULL);
	pthread_join(alert_id,NULL);
	pthread_exit(NULL);

	printf("Main Process Terminated\n");
	return 0;
}
