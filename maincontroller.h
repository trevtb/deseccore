#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QCoreApplication>
#include <QSettings>
#include "httprequesthandler.h"
#include "dbhelper.h"
#include "logger.h"

class MainController : public HttpRequestHandler {
    Q_OBJECT
private:
    QSettings *settings;
    Logger logger;
    int cwidth;
    int cheight;
public:
    explicit MainController(QSettings *settings, QObject* parent=0);
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // MAINCONTROLLER_H
