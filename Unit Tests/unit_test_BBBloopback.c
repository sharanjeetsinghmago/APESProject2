#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <linux/ioctl.h>

struct termios *configure;

int fd;

char *device = "/dev/ttyO4";

/*Function to configure UART*/
void tty_config(struct termios *con, int descriptor)
{
  tcgetattr(descriptor, con);
  con->c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
  con->c_oflag = 0;
  con->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
  con->c_cc[VMIN] = 1;
  con->c_cc[VTIME] = 0;
  if(cfsetispeed(con, B9600) || cfsetospeed(con, B9600))
  {
    perror("ERROR in baud set\n");
  }


  if(tcsetattr(descriptor, TCSAFLUSH, con) < 0)
  {
    perror("ERROR in set attr\n");
  }
}

/*Function to initialize UART*/
int uart_init(void)
{
   
  fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);// | O_NDELAY);
  if(fd == -1)
  { 
    perror("ERROR opening file descriptor\n");
  }

  configure = (struct termios*)malloc(sizeof(struct termios));
  tty_config(configure, fd);

  if(tcsetattr(fd,TCSAFLUSH, configure) < 0)
  {
    printf("\nERROR: TC Set Attr\n");
  }

  return fd;
}

typedef struct packet
{
  uint8_t log_id;
  uint8_t log_level;
  float data;
  char timestamp[25];
  char c;
}log_packet;

void main()
{
  uart_init();  

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
printf("Sending Data\n");
  if((n=write(fd,&x,sizeof(x))) < 0){
        printf("\nWrite Fail\n");
}


//printf("Waiting for recv data\n");
if((n=read(fd,&y,sizeof(y))) < 0)
{
    printf("\nRead Fail\n");
}

if( y == x )
  printf("[Success] UART Loopback \n");
else
  printf("[Failure] UART Loopback \n");

}