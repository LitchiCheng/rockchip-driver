#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>

int main(int argc,char **argv)
{
    int fd; 
    fd = open("/dev/gpiotest",O_RDWR);
    if(fd < 0){   
        perror("");
        exit(1);
    }   
    int output = 0;
    while(1){
        int ret = write(fd, &output, sizeof(int));
        output = (output == 0) ? 1 : 0;
        sleep(3);
    }
    close(fd);
    return 0;
}
