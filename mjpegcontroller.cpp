#include "mjpegcontroller.h"

MjpegController::MjpegController(QMap<QString, QString> settings, QSettings *iniSettings, QObject *parent) : HttpRequestHandler(parent) {
    this->settings = settings;
    this->iniSettings = iniSettings;
    active = false;
    packageCounter = 0;
    currentPackage = 0;
    thrMJPGC = new QThread();
} //endconstructor

void MjpegController::service(HttpRequest&, HttpResponse &response) {
    active = true;

    if (thrMJPGC->isRunning()) {
        thrMJPGC->quit();
        thrMJPGC->deleteLater();
        thrMJPGC = new QThread();
    } //endif

    mjpgclient = new MjpegClient(&settings, iniSettings);
    mjpgclient->moveToThread(thrMJPGC);

    connect(thrMJPGC, SIGNAL(started()), mjpgclient, SLOT(start()));
    connect(this, SIGNAL(stopMJPGC()), mjpgclient, SLOT(stop()), Qt::DirectConnection);
    connect(mjpgclient, SIGNAL(newData(QByteArray)), this, SLOT(newData(QByteArray)), Qt::DirectConnection);

    connect(mjpgclient, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    thrMJPGC->start();

    logger.log("MJPG streamer streaming.", Logger::OK);
    response.setHeader("Cache-Control", "no-cache");
    response.setHeader("Pragma", "no-cache");
    response.setHeader("Expires", "Thu, 01 Dec 1994 16:00:00 GMT");
    response.setHeader("Connection", "close");
    response.setHeader("Content-Type", "multipart/x-mixed-replace; boundary=--jbound");
    response.write(" ");

    while (active) {
        QByteArray rawImage = getData();
        response.write("--jbound\r\n", false, true);
        response.resetSentHeaders();
        response.setHeader("Content-Type", "image/jpeg");
        response.setHeader("Content-Length", rawImage.size());
        response.write(rawImage, false, true);
    } //endwhile

    stopMJPGC();
    //mjpgclient->deleteLater();
    //mjpgclient = 0;
    thrMJPGC->quit();
    thrMJPGC->deleteLater();
    response.write("\r\n", true, false);
    usleep(4000000);
} //endfunction service

void MjpegController::newData(QByteArray data) {
    QMutexLocker locker(&mutex);
    this->data = data;
    if (packageCounter > 10000) {
        packageCounter = 3;
    } //endif
    packageCounter++;
} //endfunction newData

QByteArray MjpegController::getData() {
    while (currentPackage == packageCounter) {
        usleep(1);
    } //endwhile
    QMutexLocker locker(&mutex);
    QByteArray retVal = data;
    currentPackage = packageCounter;
    return retVal;
} //endfunction getData

void MjpegController::stop() {
    active = false;
} //endfunction stop

void MjpegController::errorString(QString error) {
    logger.log("MJPG streamer: "+error, Logger::ERROR);
} //endfunction errorString
