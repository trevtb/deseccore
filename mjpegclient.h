#ifndef MjpegClient_H
#define MjpegClient_H

#include <QObject>
#include <QMutex>
#include <QSettings>
#include <QByteArray>
#include <QSslSocket>
#include <QtNetwork>
#include <QImage>

#include "logger.h"

class MjpegClient: public QObject {
        Q_OBJECT
public:
    MjpegClient(QMap<QString, QString> *settings, QSettings *iniSettings, QObject *parent = 0);
    ~MjpegClient();
signals:
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();
    void newData(QByteArray);
private slots:
    void start();
    void dataReady();
    void processBlock();
    void lostConnection();
    void lostConnection(QAbstractSocket::SocketError);
    void reconnect();
    void connectionReady();
    void emitStdImg();
public slots:
    void stop();
private:
    QMap<QString, QString> *settings;
    QSettings *iniSettings;
    Logger logger;
    QSslSocket *m_socket;
    QMutex mutex;

    QString m_boundary;
    bool m_firstBlock;

    QByteArray m_dataBlock;

    QSize m_resolution;
    bool m_resize;
    bool m_autoReconnect;

    QString m_host;
    int m_port;
    QString m_url;
    QString m_user;
    QString m_pass;

    bool m_flipImage;
    int  m_delay;
};

#endif // MjpegClient_H
