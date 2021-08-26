// C library headers
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>

// Linux headers
#include <fcntl.h>	 // Contains file controls like O_RDWR
#include <errno.h>	 // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>	 // write(), read(), close()


using std::string;

string convertToString(char* a, int size)
{	
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

int main()
{
	using namespace std;
	using std::cout;
	using std::endl;

	int serial_port = open("/dev/ttyACM0", O_RDWR);

	// Check for errors
	if (serial_port < 0)
	{
		printf("Error %i from open: %s\n", errno, strerror(errno));
	}
	else
	{
		printf("Connection successful...\n");
	}

	struct termios tty;

	// Read in existing settings, and handle any error
	// NOTE: This is important! POSIX states that the struct passed to tcsetattr()
	// must have been initialized with a call to tcgetattr() overwise behaviour
	// is undefined
	if (tcgetattr(serial_port, &tty) != 0)
	{
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}
	struct termios
	{
		tcflag_t c_iflag; /* input mode flags */
		tcflag_t c_oflag; /* output mode flags */
		tcflag_t c_cflag; /* control mode flags */
		tcflag_t c_lflag; /* local mode flags */
		cc_t c_line;	  /* line discipline */
		cc_t c_cc[NCCS];  /* control characters */
	};

	tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	//tty.c_cflag |= PARENB;	// Set parity bit, enabling parity

	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag |= CSTOPB;	// Set stop field, two stop bits used in communication

	tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
	tty.c_cflag |= CS5;	   // 5 bits per byte
	tty.c_cflag |= CS6;	   // 6 bits per byte
	tty.c_cflag |= CS7;	   // 7 bits per byte
	tty.c_cflag |= CS8;	   // 8 bits per byte (most common)

	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CRTSCTS;	 // Enable RTS/CTS hardware flow control

	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;

	tty.c_lflag &= ~ECHO;	// Disable echo
	tty.c_lflag &= ~ECHOE;	// Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo

	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl

	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

	tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 0;

	// Set in/out baud rate to be 9600
	cfsetspeed(&tty, B9600);

	// Save tty settings, also checking for error
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
	{
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
	else
	{
		/*Reading from device*/

		// Allocate memory for read buffer, set size according to your needs
		char read_buf[4];
		int buf_size = sizeof(read_buf)/sizeof(char);

		// Read bytes. The behaviour of read() (e.g. does it block?,
		// how long does it block for?) depends on the configuration
		// settings above, specifically VMIN and VTIME
		while (true)
		{
			int n = read(serial_port, &read_buf, sizeof(read_buf));
			ssize_t read(int fildes, void *buf, size_t nbyte);

			//printf("Number of bits to read: %d\n", n);
			if (n != 0)
			{	
				/*
				for (int i = 0; i <= sizeof(read_buf) - 1; i++)
				{
					cout << read_buf[i];
				}
				*/
				string data = convertToString(read_buf, buf_size);
				cout << data << endl;
			}
		}
	}
	// n is the number of bytes read. n may be 0 if no bytes were received, and can also be negative to signal an error.
}