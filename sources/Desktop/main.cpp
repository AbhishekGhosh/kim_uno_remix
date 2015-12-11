#include "mainwindow.h"
#include <QApplication>

#if defined( __APPLE__ ) || defined( __linux__ )
#include <signal.h>
#include <unistd.h>
#endif

#if defined( __APPLE__ ) || defined( __linux__ )
void handler( int sig )
{
    sig = messageFlags = kMessage_Reload;
}
#endif

int main(int argc, char *argv[])
{
#if defined( __APPLE__ ) || defined( __linux__ )
    // set up our signal handler (for code reload)
    //for ( int sig : ignoreSignals )
    //    signal(sig, SIG_IGN);

    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);
#endif

    // now start up the application.
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
