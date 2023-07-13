#ifndef MJPEGCONTROLLER_H
#define MJPEGCONTROLLER_H

#include <QMutex>
#include <QThread>
#include <QSettings>
#include "httprequesthandler.h"
#include "mjpegclient.h"

class MjpegController : public HttpRequestHandler {
    Q_OBJECT
private:
    QMap<QString, QString> settings;
    QSettings *iniSettings;
    bool active;
    Logger logger;
    QMutex mutex;
    QByteArray data;
    QThread *thrMJPGC;
    int packageCounter;
    int currentPackage;
    QByteArray getData();
    MjpegClient *mjpgclient;
public:
    MjpegController(QMap<QString, QString> settings, QSettings *iniSettings, QObject* parent=0);
    void service(HttpRequest&, HttpResponse& response);
signals:
    void error(QString);
    void stopMJPGC();
private slots:
    void newData(QByteArray data);
    void errorString(QString error);
public slots:
    void stop();
};

#endif // MJPEGCONTROLLER_H
