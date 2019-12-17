#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#include <mutex>
#include <thread>
#include <iostream>

// using namespace std;

// start daemon via: https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149

std::mutex mut;

void func() {
    std::lock_guard<std::mutex> lk( mut );

    std::cout << "It is function of " << std::this_thread::get_id() << std::endl;
}

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}

int main()
{
/*
    std::unique_ptr<int> p( new int( 32 ) );
    // p = std::unique_ptr<int>( new 32 );
    std::thread thr = std::thread(&func);

    {
       std::lock_guard<std::mutex> lk( mut );
       std::cout << "Hello World of: " << std::this_thread::get_id() << " with val: " << *p << std::endl;
    }
*/
    skeleton_daemon();

    while (1)
    {
       //TODO: Insert daemon code here.
       syslog (LOG_NOTICE, "First daemon started.");
       sleep (20);
       break;
    }

    syslog (LOG_NOTICE, "First daemon terminated.");
    closelog();

    return EXIT_SUCCESS;

//    thr.join();

    return 0;
}

