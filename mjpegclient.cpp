#include "mjpegclient.h"
#include <QCoreApplication>
#include <QTimer>

MjpegClient::MjpegClient(QMap<QString, QString> *settings, QSettings *iniSettings, QObject *parent)
        : QObject(parent)
        , m_socket(0)
        , m_boundary("")
        , m_firstBlock(true)
        , m_dataBlock("")
        , m_resolution(-1,-1)
        , m_autoReconnect(true)
        , m_flipImage(false)
        , m_delay(0) {
    this->settings = settings;
    this->iniSettings = iniSettings;
} //endconstructor

MjpegClient::~MjpegClient() {
} //enddestructor

void MjpegClient::start() {
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = 0;
    } //endif

    m_host = settings->value("host");
    m_port = settings->value("port").toInt();
    m_url = settings->value("streampath");
    m_user = "";
    m_pass = "";
    if (settings->value("useauth").toInt() == 1) {
        m_user = settings->value("user");
        m_pass = settings->value("pass");
    } //endif
    if (settings->value("autoreconnect").toInt() == 1) {
        m_autoReconnect = true;
    } else {
        m_autoReconnect = false;
    } //endif
    m_delay = settings->value("delay").toInt();

    if (settings->value("resize").toInt() == 1) {
        m_resize = true;
    } //endif

    QString tsize = settings->value("resolution");
    QStringList slist = tsize.split("x");
    if (slist.length() != 2) {
        logger.log("MJPG client settings contain invalid size parameter. Disable resize or provide a proper value, using defaults.", Logger::ERROR);
        slist.clear();
        slist.append("320");
        slist.append("240");
    } //endif
    bool validwidth = false;
    bool validheight = false;
    int width = slist.at(0).toInt(&validwidth);
    int height = slist.at(1).toInt(&validheight);
    if (validwidth && validheight) {
        m_resolution = QSize(width, height);
    } else {
        logger.log("MJPG client settings contain invalid size parameter. Disable resize or provide a proper value, using defaults.", Logger::ERROR);
        m_resolution = QSize(320, 240);
    } //endif

    m_socket = new QSslSocket(this);
    connect(m_socket, SIGNAL(readyRead()),    this,   SLOT(dataReady()));
    connect(m_socket, SIGNAL(disconnected()), this,   SLOT(lostConnection()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(socketDisconnected()));
    connect(m_socket, SIGNAL(connected()),    this, SIGNAL(socketConnected()));
    connect(m_socket, SIGNAL(connected()),    this,   SLOT(connectionReady()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(socketError(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(lostConnection(QAbstractSocket::SocketError)));

    if (settings->value("usessl").toInt() == 1) {
        QList<QSslCertificate> cert = QSslCertificate::fromPath(settings->value("sslcert"));
        QSslError error(QSslError::SelfSignedCertificate, cert.at(0));
        QList<QSslError> expectedSslErrors;
        expectedSslErrors.append(error);
        m_socket->ignoreSslErrors(expectedSslErrors);
        m_socket->connectToHostEncrypted(m_host, m_port);
    } else {
       m_socket->connectToHost(m_host, m_port);
    } //endif
    m_socket->setReadBufferSize(1024 * 1024);
} //endfunction start

void MjpegClient::connectionReady() {
    char data[1024];
    sprintf(data, "GET %s HTTP/1.0\r\n",qPrintable(m_url));
    m_socket->write((const char*)&data,strlen((const char*)data));

    sprintf(data, "Host: %s\r\n",qPrintable(m_host));
    m_socket->write((const char*)&data,strlen((const char*)data));

    if (!m_user.isEmpty() || !m_pass.isEmpty()) {
        logger.log("MJPG client using authentication.", Logger::OK);
        sprintf(data, "%s:%s",qPrintable(m_user),qPrintable(m_pass));
        QByteArray auth = QByteArray((const char*)&data).toBase64();
        sprintf(data, "Authorization: Basic %s\r\n",auth.constData());
        m_socket->write((const char*)&data,strlen((const char*)data));
    } //endif

    sprintf(data, "\r\n");
    m_socket->write((const char*)&data,strlen((const char*)data));
} //endfunction connectionReady

void MjpegClient::lostConnection() {
    logger.log("lostConnection() called.", Logger::OK);
    emitStdImg();
    if (m_autoReconnect) {
        logger.log("Firing reconnect timer.", Logger::OK);
        QTimer::singleShot(2000,this,SLOT(reconnect()));
    } //endif
} //endfunction lostConnection

void MjpegClient::lostConnection(QAbstractSocket::SocketError error) {
    logger.log("lostConnection() called with socket error.", Logger::OK);
    QMutexLocker locker(&mutex);
    logger.log("MJPG client lost the connection to the current unit, reason: "+m_socket->errorString()+".", Logger::WARNING);
    if (error == QAbstractSocket::ConnectionRefusedError) {
        lostConnection();
    } else {
        emitStdImg();
    } //endif
} //endif

void MjpegClient::reconnect() {
    logger.log("MJPG client trying to reconnect to http://"+m_host+":"+QString::number(m_port)+m_url, Logger::OK);
    start();
} //endfunction reconnect

void MjpegClient::dataReady() {
    QByteArray bytes = m_socket->readAll();
    if (bytes.size() > 0) {
        m_dataBlock.append(bytes);
        processBlock();
        if (m_delay > 0) {
            usleep(m_delay*1000);
        } //endif
    } //endif
} //endfunction dataReady

void MjpegClient::processBlock() {
    if (m_boundary.isEmpty()) {
        // still waiting for boundary string defenition, check for content type in data block
        const char * ctypeString;
        if (m_dataBlock.contains("Content-Type:")) {
            ctypeString = "Content-Type:";
        } else if(m_dataBlock.contains("content-type:")) {
            // allow for buggy servers (some IP cameras - trendnet, I'm looking at you!)
            // sometimes dont use proper case in their headers.
            ctypeString = "content-type:";
        } else {
            ctypeString = 0;
        } //endif

        if (ctypeString) {
            int ctypeIdx = m_dataBlock.indexOf(ctypeString);
            if (ctypeIdx < 0) {
                logger.log("MJPG client can't find the content-type index in the data block, exiting.", Logger::ERROR);
                stop();
                return;
            } //endif

            static QString boundaryMarker = "boundary=";
            int boundaryStartIdx = m_dataBlock.indexOf(boundaryMarker, ctypeIdx);
            if (boundaryStartIdx < 0) {
                logger.log("MJPG client can't find the boundary index definition in the data block, exiting.", Logger::ERROR);
                stop();
                return;
            } //endif

            int eolIdx = m_dataBlock.indexOf("\n", boundaryStartIdx);
            int pos = boundaryStartIdx + boundaryMarker.length();
            m_boundary = m_dataBlock.mid(pos, eolIdx - pos);
            m_boundary.replace("\r","");
        } //endif
    } else {
        // we have the boundary string defenition, check for the boundary string in the data block.
        // If found, grab the data from the start of the block up to and including the boundary, leaving any data after the boundary in the block.
        // What we then have to process could look:
        // Content-Type.....\r\n(data)--(boundary)

        // If its the first block, we wait for the boundary, but discard it since the first block is the one that contains the server headers
        // like the boundary definition, Server:, Connection:, etc.
        int idx = m_dataBlock.indexOf(m_boundary);

        while (idx > 0 && m_socket->isOpen()) {
            QByteArray block = m_dataBlock.left(idx);

            int blockAndBoundaryLength = idx + m_boundary.length();
            m_dataBlock.remove(0,blockAndBoundaryLength);

            if (m_firstBlock) {
                m_firstBlock = false;
            } else {
                static const char * eol1 = "\n\n";
                static const char * eol2 = "\r\n\r\n";
                int headerLength = 0;
                if (block.contains(eol2)) {
                    headerLength = block.indexOf(eol2,0) + 4;
                } else if(block.contains(eol1)) {
                        headerLength = block.indexOf(eol1,0) + 2;
                } //endif

                if (headerLength) {
                    block.remove(0,headerLength);
                    // Block should now be just data

                    if (block.length() > 1) {
                        // THIS COSTS SPEED!! BETTER PREVENT IT
                        if (m_flipImage || m_resize) {
                            QImage frame = QImage::fromData(block);
                            if (!frame.isNull()) {
                                if (m_flipImage) {
                                    frame = frame.mirrored(true, true);
                                } //endif
                                if (frame.size().width() > m_resolution.width() && frame.size().height() > m_resolution.height()
                                        && m_resolution.width() > 0 && m_resolution.height() > 0) {
                                    frame = frame.scaled(m_resolution);
                                } //endif

                                block.clear();
                                QBuffer buffer(&block);
                                buffer.open(QIODevice::WriteOnly);
                                frame.save(&buffer, "JPEG");
                            } //endif
                        } //endif
                        emit newData(block);
                    } //endif
                } //endif
            } //endif
            // check for another boundary string in the data before exiting from processBlock()
            idx = m_dataBlock.indexOf(m_boundary);
        } //endwhile
    } //endif
} //endfunction processBlock

void MjpegClient::stop() {
    if (m_socket->isOpen()) {
        //m_socket->abort();
        //m_socket->disconnectFromHost();
        //m_socket->waitForDisconnected();
        m_socket->close();
        m_socket->deleteLater();
        //delete m_socket;
        m_socket = 0;
    } //endif
} //endfunction stop

void MjpegClient::emitStdImg() {
    QString filename;
    if (m_resolution.width() <= 320) {
        filename = "nocon_320.jpg";
    } else if (m_resolution.width() > 320 && m_resolution.width() <= 640) {
        filename = "nocon_640.jpg";
    } else if (m_resolution.width() > 640) {
        filename = "nocon_1280.jpg";
    } //endif

    QString docroot = iniSettings->value("HttpServer/docroot", ".").toString();
    if (docroot[docroot.length()-1] != '/') {
        docroot += "/";
    } //endif
    QFile file(docroot+"assets/images/"+filename);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray imgData = file.readAll();
        file.close();
        emit newData(imgData);
    } //endif
} //endfunction emitStdImg
