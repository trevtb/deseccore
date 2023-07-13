#ifndef LOGINCONTROLLER_H
#define LOGINCONTROLLER_H

#include <QTime>
#include <QSettings>

#include "httprequesthandler.h"

class LoginController : public HttpRequestHandler {
    Q_OBJECT
private:
    bool isValidUser(QString user, QString pass);
    QString userfile;
public:
    explicit LoginController(QSettings *settings, QObject* parent=0);
    void service(HttpRequest& request, HttpResponse& response);
};
#endif // LOGINCONTROLLER_H
