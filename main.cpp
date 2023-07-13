#include <QCoreApplication>
#include <QSettings>

#include <signal.h>

#include "httplistener.h"
#include "httpsessionstore.h"
#include "requestmapper.h"
#include "staticfilecontroller.h"
#include "logger.h"

#define RESTART_CODE 1000

static void signalHandler (int signo) {
    Logger *logger = new Logger();

    if (signo == SIGINT) {
        logger->log("DESEC core recieved SIGINT, exiting.", Logger::OK);
        QCoreApplication::exit(0);
    } else if (signo == SIGTERM) {
        logger->log("DESEC core recieved SIGTERM, exiting.", Logger::OK);
        QCoreApplication::exit(0);
    } else if (signo == SIGHUP) {
        logger->log("DESEC core recieved SIGHUP, restarting.", Logger::OK);
        QCoreApplication::exit(1000);
    } //endif
} //endfunction signalHandler

int main(int argc, char *argv[]) {
    int return_from_event_loop_code;

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, signalHandler);

    do {
        QCoreApplication a(argc, argv);
        Logger logger(&a);
        QSettings *settings = new QSettings("/home/pi/settings.ini", QSettings::IniFormat, &a);

        logger.log("Starting DESEC core.", Logger::OK);

        RequestMapper::sessionStore = new HttpSessionStore(settings, &a);
        RequestMapper::loginController = new LoginController(settings, &a);
        RequestMapper::mainController = new MainController(settings, &a);
        RequestMapper::fileController = new StaticFileController(settings, &a);
        new HttpListener(settings, new RequestMapper(settings, &a), &a);
        logger.log("HTTP server started successfully.", Logger::OK);

        return_from_event_loop_code = a.exec();
    } while( return_from_event_loop_code == RESTART_CODE);

    return return_from_event_loop_code;
} //endfunction main
