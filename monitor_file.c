#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>

#include <linux/inotify.h>

#define EVENT_SIZE (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))


static void skeleton_daemon() {
	pid_t pid;

	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0){
		//printf("pid<0 error");
		exit(EXIT_FAILURE);
	}
	/* Success: Let the parent terminate */
	if (pid > 0){
		//printf("pid>0 parent terminate");
		exit(EXIT_SUCCESS);
	}

	/* On success: The child process becomes session leader */
	if (setsid() < 0){
		//printf("setsid <0");
		exit(EXIT_FAILURE);
	}

	/* Catch, ignore and handle signals */
	//TODO: Implement a working signal handler */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0){
		//printf("pid<0");
		exit(EXIT_FAILURE);
	}
	/* Success: Let the parent terminate */
	if (pid > 0){
		//printf("pid > 0 ");
		exit(EXIT_SUCCESS);
	}
	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");

	/* Close all open file descriptors */
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
	{
		close (x);
	}

	/* Open the log file */
	openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}

int main(int argc, char *argv[]) {
	
	int length, i = 0;

	int fd;

	int wd;

	char buffer[EVENT_BUF_LEN];

	skeleton_daemon();

	//printf("the argv[] after daemon is %s",argv[1]);

	syslog (LOG_NOTICE, "------------  First testing daemon started.----------");

	/*creating the INOTIFY instance*/
	fd = inotify_init();

	/*checking for error*/
	if ( fd < 0 ) {
	  perror( "inotify_init" );
	}

	/*adding the directory into watch list. */
	wd = inotify_add_watch( fd,  "/home/temp/test"    , IN_ALL_EVENTS  /* IN_IGNORED | IN_OPEN | IN_CLOSE_WRITE | IN_ATTRIB | IN_CREATE | IN_DELETE |IN_CLOSE | IN_MODIFY | IN_ACCESS */ );

	if(wd<0){
	syslog(LOG_NOTICE, "wd < 0");
	perror("inotify_add_watch");    
	}

	//syslog(LOG_NOTICE, "THE FILE WE ARE MONITORING IS: %s",argv[1]);

	while (1)
	{
		//TODO: Insert daemon code here.

	syslog (LOG_NOTICE, "~~~~~~~ One new testing daemon started.~~~~~~~~~");    

	/*read to determine the event change happens on directory. Actually this read blocks until the change event occurs*/
		syslog (LOG_NOTICE, "BEFORE READ IN BUFFER");
		length = read( fd, buffer, EVENT_BUF_LEN );

	syslog (LOG_NOTICE, "AFTER READ IN BUFFER.");

		/*checking for error*/
		if ( length < 0 ) {
		  perror( "read" );
		}else if(length == 0){
		syslog(LOG_NOTICE," length =0 " );
		continue;
	}

	/*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
	while ( i < length ) {     
		struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];     
		if ( event->len ) {

		if (event->mask & IN_IGNORED) {
				inotify_rm_watch(fd,wd);
				inotify_add_watch(fd,"/home/test", IN_ALL_EVENTS );
			}       

		if ( event->mask & IN_ATTRIB ) {
					if ( event->mask & IN_ISDIR ) {
						//printf( " directory %s ATTRIB .\n", event->name );
			syslog (LOG_NOTICE, " directory:chaning     ATTRIB .");
						syslog (LOG_NOTICE, " directory:%s ATTRIB .", event->name);
					}   
					else {
			syslog (LOG_NOTICE, " directory:chaning     ATTRIB .");

						syslog (LOG_NOTICE, " file:%s ATTRIB .", event->name);                            
					}   
				}  

		if ( event->mask & IN_MODIFY ) {
				   if ( event->mask & IN_ISDIR ) {
						//printf( " directory %s modified .\n", event->name );
			syslog (LOG_NOTICE, " dirtory:chaning    !!!!!!! .");
						syslog (LOG_NOTICE, " file:%s modify.", event->name);
					}
					else {
			syslog (LOG_NOTICE, " file  modify:chaning!!! .");

						syslog (LOG_NOTICE, " file:%s modify.", event->name);
					}
				}

		if ( event->mask & IN_CLOSE_WRITE ) {
					if ( event->mask & IN_ISDIR ) {
						//printf( " directory %s modified .\n", event->name );
						syslog (LOG_NOTICE, " CLSOE WRITE    !!!!!!! .");
						//syslog (LOG_NOTICE, " file:%s modify.", event->name);
					}
					else {
						//syslog (LOG_NOTICE, " file modify:changing!!! .");
					syslog (LOG_NOTICE, " CLSOE WRITE    !!!!!!! .");                   
						//syslog (LOG_NOTICE, " file:%s modify.", event->name);
					}
				}

		if ( event->mask & IN_OPEN ) {
					if ( event->mask & IN_ISDIR ) {
						//printf( "New directory %s opened.\n", event->name );
						syslog (LOG_NOTICE, " directory:%s open.", event->name);
					}
					else {
			syslog (LOG_NOTICE, " file:opening.");

						syslog (LOG_NOTICE, " file:%s open.", event->name);
						//printf( "New file %s open.\n", event->name );

					}
				}

			if ( event->mask & IN_CREATE ) {
			if ( event->mask & IN_ISDIR ) {
				//printf( "New directory %s created.\n", event->name );
				syslog (LOG_NOTICE, "new directory:%s created.", event->name);
			}
			else {
				syslog (LOG_NOTICE, "new file:%s created.", event->name);
				//printf( "New file %s created.\n", event->name );

				}
			}
			else if ( event->mask & IN_DELETE ) {
			if ( event->mask & IN_ISDIR ) {
				syslog (LOG_NOTICE, "dir:%s is deleted.", event->name);
				//printf( "Directory %s deleted.\n", event->name );
			}
			else {
				syslog (LOG_NOTICE, "file:%s is deleted.", event->name);
				//printf( "File %s deleted.\n", event->name );
			}
			}

		else if ( event->mask & IN_ACCESS ) {
					if ( event->mask & IN_ISDIR ) {
						syslog(LOG_NOTICE, "dir:%s is accessed.", event->name);
						//printf( "The directory %s was accessed.\n", event->name );
					}
					else {
						syslog(LOG_NOTICE, "file:%s is accessed.", event->name);
						//printf( "The file %s was accessed.\n", event->name );
					}   
				} 

		if ( event->mask & IN_CLOSE ) {
					if ( event->mask & IN_ISDIR ) {
						//printf( "New directory %s created.\n", event->name );
						syslog (LOG_NOTICE, " directory:%s closed.", event->name);
					}
					else {
						syslog (LOG_NOTICE, " file:%s closed.", event->name);
						//printf( "New file %s created.\n", event->name );

					}
				}

			}
		i += EVENT_SIZE + event->len;
	}

		i=0;    
	syslog (LOG_NOTICE, "~~~~~~one testing daemon ended.~~~~~~~~~~");
		sleep (20);
	}

	/*removing the directory from the watch list.*/
	inotify_rm_watch( fd, wd );

	/*closing the INOTIFY instance*/
	close( fd );

	syslog (LOG_NOTICE, "--------------- first testing daemon terminated.----------------");
	closelog();

	return EXIT_SUCCESS;
	
}