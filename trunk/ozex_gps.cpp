
#include "ozex_gps.h"

#include <time.h>

FILE* track = NULL;
/** do something with the GGA data */
static void print_gga(nmeap_gga_t *gga)
{
    printf("found GPGGA message %.6f %.6f %.0f %lu %d %d %f %f\n",
            gga->latitude  ,
            gga->longitude, 
            gga->altitude , 
            gga->time     , 
            gga->satellites,
            gga->quality   ,
            gga->hdop      ,
            gga->geoid     
            );
}

/** called when a gpgga message is received and parsed */
static void gpgga_callout(nmeap_context_t *context,void *data,void *user_data)
{
    nmeap_gga_t *gga = (nmeap_gga_t *)data;
    
//    printf("-------------callout\n");
//    print_gga(gga);
}


/** do something with the RMC data */
static void print_rmc(nmeap_rmc_t *rmc)
{
    printf("found GPRMC Message %lu %c %.6f %.6f %f %f %lu %f\n",
            rmc->time,
            rmc->warn,
            rmc->latitude,
            rmc->longitude,
            rmc->speed,
            rmc->course,
            rmc->date,
            rmc->magvar
            );
}

/** called when a gprmc message is received and parsed */
static void gprmc_callout(nmeap_context_t *context,void *data,void *user_data)
{
    nmeap_rmc_t *rmc = (nmeap_rmc_t *)data;
    
//    printf("-------------callout\n");
//    print_rmc(rmc);

	if (track)
	{
		fprintf(track, "  %2.6f,  %2.6f,0,-777,,,\r\n",
				rmc->latitude, rmc->longitude);
	}
}


/*************************************************************************
 * LINUX IO
 */
 
/**
 * open the specified serial port for read/write
 * @return port file descriptor or -1
 */
int openPort(const char *tty,int baud)
{
	int     status;
	int     fd;
	struct termios newtio;
    
    // open the tty
    fd = open(tty,O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return fd;
    }

    // flush serial port
    status = tcflush(fd, TCIFLUSH);
    if (status < 0) {
        perror("tcflush");
        close(fd);
        return -1;
    }
    
    /* get current terminal state */
    tcgetattr(fd,&newtio);

    // set to raw terminal type
    newtio.c_cflag = baud | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNBRK | IGNPAR;
    newtio.c_oflag = 0;    
    
    // control parameters
    newtio.c_cc[VMIN]     = 1;   // block for at least one charater 
    
    // set its new attrigutes
    status = tcsetattr(fd,TCSANOW,&newtio);
    if (status < 0) {
        perror("tcsetattr");
        close(fd);
        fd = -1;
        return fd;
    }

	return fd;
}


/* ---------------------------------------------------------------------------------------*/
/* STEP 1 : allocate the data structures. be careful if you put them on the stack because */
/*          they need to be live for the duration of the parser                           */
/* ---------------------------------------------------------------------------------------*/
static nmeap_context_t nmea;	   /* parser context */
static nmeap_gga_t     gga;		   /* this is where the data from GGA messages will show up */
static nmeap_rmc_t     rmc;		   /* this is where the data from RMC messages will show up */
static int             user_data; /* user can pass in anything. typically it will be a pointer to some user data */
static int fd = 0;


double ozex_gps_lat()
{
	return rmc.latitude;
}

double ozex_gps_lon()
{
	return rmc.longitude;
}



static void ozex_gps_init_internals()
{
	int status;
	
    status = nmeap_init(&nmea,(void *)&user_data);
    if (status != 0) {
        printf("nmeap_init %d\n",status);
        exit(1);
    }
    
	/* ---------------------------------------*/
	/*STEP 3 : add standard GPGGA parser      */                                                
	/* -------------------------------------- */
    status = nmeap_addParser(&nmea,"GPGGA",nmeap_gpgga,gpgga_callout,&gga);
    if (status != 0) {
        printf("nmeap_add %d\n",status);
        exit(1);
    }

	/* ---------------------------------------*/
	/*STEP 4 : add standard GPRMC parser      */                                                
	/* -------------------------------------- */
    status = nmeap_addParser(&nmea,"GPRMC",nmeap_gprmc,gprmc_callout,&rmc);
    if (status != 0) {
        printf("nmeap_add %d\n",status);
        exit(1);
    }


	struct tm *time_date_struct_ptr;
	time_t timer;

	time( &timer );
	time_date_struct_ptr = localtime( &timer );

	char filename[128];

	strcpy(filename, asctime( time_date_struct_ptr ));
	char *s = strchr(filename, '\n');
	*s = 0; 
	strcat(filename, ".plt");

	track = fopen(filename, "wt");

	fprintf(track, "OziExplorer Track Point File Version 2.1\r\n");
	fprintf(track, "WGS 84\r\n");
	fprintf(track, "Altitude is in Feet\r\n");
	fprintf(track, "Reserved 3\r\n");
	fprintf(track, "0,2,255,Track Log,0,0,2,8421376\r\n");
	fprintf(track, "100\r\n");

}

void ozex_gps_start(const char *port, int bauds)
{
	int baudrate = B9600;
	
	switch (bauds)
	{
		case 4800: baudrate = B4800; break;
		case 9600: baudrate = B9600; break;
		case 19200: baudrate = B19200; break;
		case 38400: baudrate = B38400; break;
		case 57600: baudrate = B57600; break;
		case 115200: baudrate = B115200; break;
		default:
			break;
	}

    fd = openPort(port, baudrate);

    if (fd < 0) 
    {
        printf("openPort %d\n",fd);
    }
    
    ozex_gps_init_internals();

}

void ozex_gps_start_fromfile(const char *port)
{
    fd = open(port, O_RDONLY);

    if (fd < 0) 
    {
        printf("openPort %d\n",fd);
    }
    
    ozex_gps_init_internals();
}


void ozex_gps_update()
{
    int  status;
    int  rem;
	int  offset;
	int  len;
	static char buffer[4096];

    len = rem = read(fd,buffer,sizeof(buffer));
    if (len <= 0) 
    {
        perror("read");
        return;
    }
    
    
	offset = 0;
    while(rem > 0) 
    {
        status = nmeap_parseBuffer(&nmea,&buffer[offset],&rem);
		offset += (len - rem); 

/*        
        switch(status) 
        {
		    case NMEAP_GPGGA:
		        printf("-------------switch\n");
		        print_gga(&gga);
		        printf("-------------\n");
		        break;
		    case NMEAP_GPRMC:
		        printf("-------------switch\n");
		        print_rmc(&rmc);
		        printf("-------------\n");
		        break;
		    default:
		        break;
        }
*/
    }
}

void ozex_gps_close()
{
	if (track)
	{
		fclose(track);
	}
	close(fd);
}

#if 0
int main(int argc,char *argv[])
{
    int  status;
    int  rem;
	int  offset;
	int  len;
	char buffer[32];
    int  fd;
    const char *port = "/dev/ttyS0";
    
    // default to ttyS0 or invoke with 'linux_nmeap <other serial device>' 
    if (argc == 2) {
        port = argv[1];
    }
    
    /* --------------------------------------- */
    /* open the serial port device             */
    /* using default 4800 baud for most GPS    */
    /* --------------------------------------- */
    fd = openPort(port,B4800);
    if (fd < 0) {
        /* open failed */
        printf("openPort %d\n",fd);
        return fd;
    }
    
    status = nmeap_init(&nmea,(void *)&user_data);
    if (status != 0) {
        printf("nmeap_init %d\n",status);
        exit(1);
    }
    
	/* ---------------------------------------*/
	/*STEP 3 : add standard GPGGA parser      */                                                
	/* -------------------------------------- */
    status = nmeap_addParser(&nmea,"GPGGA",nmeap_gpgga,gpgga_callout,&gga);
    if (status != 0) {
        printf("nmeap_add %d\n",status);
        exit(1);
    }

	/* ---------------------------------------*/
	/*STEP 4 : add standard GPRMC parser      */                                                
	/* -------------------------------------- */
    status = nmeap_addParser(&nmea,"GPRMC",nmeap_gprmc,gprmc_callout,&rmc);
    if (status != 0) {
        printf("nmeap_add %d\n",status);
        exit(1);
    }
    
	/* ---------------------------------------*/
	/*STEP 5 : process input until done       */                                                
	/* -------------------------------------- */
    for(;;) {
		/* ---------------------------------------*/
		/*STEP 6 : get a buffer of input          */                                                
		/* -------------------------------------- */
        len = rem = read(fd,buffer,sizeof(buffer));
        if (len <= 0) {
            perror("read");
            break;
        }
        
        
		/* ----------------------------------------------*/
		/*STEP 7 : process input until buffer is used up */                                                
		/* --------------------------------------------- */
		offset = 0;
        while(rem > 0) {
			/* --------------------------------------- */
			/*STEP 8 : pass it to the parser           */
			/* status indicates whether a complete msg */
			/* arrived for this byte                   */
			/* NOTE : in addition to the return status */
			/* the message callout will be fired when  */
			/* a complete message is processed         */
			/* --------------------------------------- */
            status = nmeap_parseBuffer(&nmea,&buffer[offset],&rem);
			offset += (len - rem); 
            
			/* ---------------------------------------*/
			/*STEP 9 : process the return code        */
            /* DON"T NEED THIS IF USING CALLOUTS      */
            /* PICK ONE OR THE OTHER                  */
			/* -------------------------------------- */
            switch(status) {
            case NMEAP_GPGGA:
                printf("-------------switch\n");
                print_gga(&gga);
                printf("-------------\n");
                break;
            case NMEAP_GPRMC:
                printf("-------------switch\n");
                print_rmc(&rmc);
                printf("-------------\n");
                break;
            default:
                break;
            }
        }
    }
    
    /* close the serial port */
    close(fd);
    
    return 0;
}
#endif
