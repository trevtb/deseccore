#include "requestmapper.h"

HttpSessionStore* RequestMapper::sessionStore = 0;
LoginController* RequestMapper::loginController=0;
MainController* RequestMapper::mainController=0;
StaticFileController* RequestMapper::fileController=0;

RequestMapper::RequestMapper(QSettings *settings, QObject* parent) : HttpRequestHandler(parent) {
    this->settings = settings;
} //endconstructor

void RequestMapper::service(HttpRequest& request, HttpResponse& response) {
    QString path = request.getPath();
    /**QByteArray sessionId = sessionStore->getSessionId(request, response);
    logger.log("Session id: "+QString(sessionId), Logger::OK);
    if (sessionId.isEmpty() && path != "/login") {
       response.redirect("/login");
       return;
    } //endif**/

    if (path == "/") {
        mainController->service(request, response);
    } else if (path == "/login") {
        loginController->service(request, response);
    } else if (path == "/logout") {
        //sessionStore->removeSession(sessionStore->getSession(sessionId));
        response.redirect("/");
    } else if (path.startsWith("/mjpg")) {
        int id = 1;
        QStringList parts = path.split("/");
        if (parts.length() == 3) {
            bool valid = false;
            int temp = parts[parts.length() - 1].toInt(&valid);
            if (valid) {
                id = temp;
            } //endif
        } //endif
        DbHelper *helper = new DbHelper(settings);
        helper->connectToDb();
        QMap<QString, QString> unitSettings = helper->getUnit(id);
        helper->closeDbConnection();
        MjpegController *mjpgController = new MjpegController(unitSettings, settings);
        connect(this, SIGNAL(doStop()), mjpgController, SLOT(stop()));
        mjpgController->service(request, response);
    } else if (path.startsWith("/assets/")) {
        QString docroot = settings->value("HttpServer/docroot", ".").toString();
        if (docroot[docroot.length()-1] != '/') {
            docroot = docroot.mid(0, docroot.length());
        } //endif
        QString file = docroot+path;
        if (0 == access(file.toStdString().c_str(), 0)) {
            fileController->service(request, response);
        } //endif
    } else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.", true);
    } //endif
} //endfunction service
