#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include <QSettings>

#include "httpsessionstore.h"
#include "httprequesthandler.h"
#include "staticfilecontroller.h"
#include "maincontroller.h"
#include "logincontroller.h"
#include "mjpegcontroller.h"
#include "dbhelper.h"

class RequestMapper : public HttpRequestHandler {
    Q_OBJECT

private:
    QSettings *settings;
    HttpSession session;
    Logger logger;

public:
    RequestMapper(QSettings *settings, QObject* parent=0);
    void service(HttpRequest& request, HttpResponse& response);
    static HttpSessionStore *sessionStore;
    static LoginController *loginController;
    static MainController *mainController;
    static StaticFileController *fileController;
};

#endif // REQUESTMAPPER_H
