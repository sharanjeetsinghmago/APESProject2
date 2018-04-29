#include "main_task.h"


pthread_t comm_id, logger_id, alert_id;

void *func_comm()
{
	int i;
	int n,fd;
	log_packet rec,recv;
	fd = uart_init();
	printf("Communication Task Started\n");
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
				printf("Altitude = %f\n",rec.data);
			}

			if(rec.log_id == 2)
			{	
				printf("Humidity = %f\n\n",rec.data);
			}
		}

	 }

}

void *func_logger()
{

}

void *func_alert()
{

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