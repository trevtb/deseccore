#include "maincontroller.h"

MainController::MainController(QSettings *settings, QObject* parent) : HttpRequestHandler(parent) {
    this->settings = settings;
} //endconstructor

void MainController::service(HttpRequest &request, HttpResponse &response) {
    DbHelper *dbhelper = new DbHelper(settings);
    if (!dbhelper->connectToDb()) {
        logger.log("HTTP server could not connect to database.", Logger::ERROR);
        QCoreApplication::exit(0);
    } //endif
    QList<QList<QString> > units = dbhelper->getTable("deseccunit");
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.write("<html><body>");

    response.write("<h3>Welcome! You are currently logged in.</h3>");

    response.write("<div>");
    response.write("<h4>Selected stream:</h4>");
    response.write("<p><img id='streamimg' src='/mjpg' width='640' height='480' /></p>");
    response.write("</div>");

    response.write("<div>");
    response.write("<h4>Available streams:</h4>");
    foreach (QList<QString> unit, units) {
        response.write(QString("<span><a href='#' target='_self' class='button-deseccunit' id='btndcc-"+unit[0]+"'>"+unit[1]+"</a></span>").toStdString().c_str());
    } //endforeach
    response.write("</div>");

    response.write("<div>");
    response.write("<h4>Actions:</h4>");
    response.write("<p><a href='/logout' target='_self' title='Logout'>Logout</a></p>");
    response.write("</div>");

    response.write("<script src='/assets/js/jquery-1.11.3.min.js'></script>");
    response.write("<script src='/assets/js/main.js'></script>");
    response.write("</body></html>",true);
} //endfunction service
