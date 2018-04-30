#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <unistd.h>



int fd;

char *device = "/dev/spidev1.0";


/*Function to initialize SPI*/
int spi_init(void)
{
   
  fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);// | O_NDELAY);
  if(fd == -1)
  { 
    perror("ERROR opening file descriptor\n");
  }

    

  return fd;
}

typedef struct packet
{
  uint8_t log_id;
  uint8_t log_level;
  float data;
  char timestamp[50];
  char c;
}log_packet;

void main()
{
  spi_init();  

  log_packet rec,recv;

  recv.log_id = 9;
  recv.log_level = 2;
  recv.data = 5.9;

  //read(fd,&rec,sizeof(rec));
  
  //read(fd,&rec,sizeof(rec));
  char x='a';
  volatile char y=5;

  int n;
printf("%d\n",fd );
// printf("Sending Data\n");
//   if((n=write(fd,&x,sizeof(x))) < 0){
//         printf("\nWrite Fail\n");
// }

while(1){
//printf("Waiting for recv data\n");
   if((n=read(fd,&rec,sizeof(rec))) < 0)
   {
        printf("\nRead Fail\n");
	}
else if(n>0)
{
  //printf("y= %c\n", y);
  printf("Log ID = %d\n",rec.log_id );
  printf("Log Level = %d\n",rec.log_level );
  printf("Data = %f\n\n",rec.data );
  printf("Data recv\n");

	if(rec.log_id == 1)
		printf("Altitude = %f\n",rec.data);

	if(rec.log_id == 2)
		printf("Humidity = %f\n\n",rec.data);
}
}
}