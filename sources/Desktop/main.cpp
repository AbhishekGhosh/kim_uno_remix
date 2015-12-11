#include "mainwindow.h"
#include <QApplication>

#include <signal.h>
#include <unistd.h>

void handler( int sig )
{
    messageFlags = kMessage_Reload;
}

int main(int argc, char *argv[])
{
    // set up our signal handler (for code reload)
    //for ( int sig : ignoreSignals )
    //    signal(sig, SIG_IGN);

    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);

    // now start up the application.
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
