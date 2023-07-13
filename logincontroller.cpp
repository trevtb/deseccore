#include "logincontroller.h"
#include "requestmapper.h"

LoginController::LoginController(QSettings *settings, QObject* parent) : HttpRequestHandler(parent) {
    this->userfile = settings->value("HttpServer/userfile", "").toString();
} //endconstructor

void LoginController::service(HttpRequest &request, HttpResponse &response) {
    HttpSession session = RequestMapper::sessionStore->getSession(request,response,true);
    QByteArray username = request.getParameter("username");
    QByteArray password = request.getParameter("password");

    if (session.contains("username")) {
        username = session.get("username").toByteArray();
        QTime logintime = session.get("logintime").toTime();
        response.redirect("/");
    } else {
        if (isValidUser(username, password)) {
            session.set("username", username);
            session.set("logintime", QTime::currentTime());
            response.redirect("/");
        } else {
            response.setHeader("Content-Type", "text/html; charset=UTF-8");
            response.write("<html><body>");
            response.write("<form method='POST' action='/login'>");
            if (!username.isEmpty()) {
                response.write("Login failed, please try again.<br><br>");
            } //endif
            response.write("Please log in:<br>");
            response.write("Name:  <input type='text' name='username'><br>");
            response.write("Password: <input type='password' name='password'><br>");
            response.write("<input type='submit'>");
            response.write("</form");
        } //endif
    } //endif
    response.write("</body></html>",true);
} //endfunction service

bool LoginController::isValidUser(QString user, QString pass) {
    QSettings *users = new QSettings(userfile, QSettings::IniFormat);
    if (users->contains("Users/"+user) && users->value("Users/"+user, "").toString() == pass) {
        return true;
    } else {
        return false;
    } //endif
} //endfunction isValiduser
