#include "httpconnectionhandler.h"

HttpConnectionHandler::HttpConnectionHandler(QSettings* settings, HttpRequestHandler* requestHandler) : QThread() {
    this->settings = settings;
    this->requestHandler = requestHandler;
    currentRequest = 0;
    busy = false;
    useSsl = false;
    // Load SSL configuration
    loadSslConfig();
    // execute signals in my own thread
    moveToThread(this);
    socket.moveToThread(this);
    readTimer.moveToThread(this);
    connect(&socket, SIGNAL(readyRead()), SLOT(read()));
    connect(&socket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));
    readTimer.setSingleShot(true);
    this->start();
} //endconstructor

HttpConnectionHandler::~HttpConnectionHandler() {
    socket.close();
    quit();
    wait();
} //enddestructor

void HttpConnectionHandler::loadSslConfig() {
    QString sslKeyFileName = settings->value("HttpServer/sslkeyfile", "").toString();
    QString sslCertFileName = settings->value("HttpServer/sslcertfile", "").toString();
    if (!sslKeyFileName.isEmpty() && !sslCertFileName.isEmpty()) {
        // Convert relative fileNames to absolute, based on the directory of the config file.
        QFileInfo configFile(settings->fileName());
        if (QDir::isRelativePath(sslKeyFileName)) {
            sslKeyFileName=QFileInfo(configFile.absolutePath(),sslKeyFileName).absoluteFilePath();
        } //endif
        if (QDir::isRelativePath(sslCertFileName)) {
            sslCertFileName=QFileInfo(configFile.absolutePath(),sslCertFileName).absoluteFilePath();
        } //endif
        // Load the two files
        QFile certFile(sslCertFileName);
        if (!certFile.open(QIODevice::ReadOnly)) {
            logger.log("HTTP server can't open sslCertFile.", Logger::ERROR);
            return;
        } //endif
        QSslCertificate certificate(&certFile, QSsl::Pem);
        certFile.close();
        QFile keyFile(sslKeyFileName);
        if (!keyFile.open(QIODevice::ReadOnly)) {
            logger.log("HTTP server can't open sslKeyFile.", Logger::ERROR);
            return;
        } //endif
        QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
        keyFile.close();
        sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
        sslConfiguration.setLocalCertificate(certificate);
        sslConfiguration.setPrivateKey(sslKey);
        sslConfiguration.setProtocol(QSsl::TlsV1SslV3);
        useSsl=true;
    } //endif
} //endfunction loadSslConfig

void HttpConnectionHandler::run() {
    try {
        exec();
    } catch (...) {
        logger.log("HTTP server encountered an uncaught exception in a child thread.", Logger::ERROR);
    } //endtry
} //endfunction run

void HttpConnectionHandler::handleConnection(tSocketDescriptor socketDescriptor) {
    busy = true;

    //UGLY workaround - we need to clear writebuffer before reusing this socket
    //https://bugreports.qt-project.org/browse/QTBUG-28914
    socket.connectToHost("",0);
    socket.abort();

    if (!socket.setSocketDescriptor(socketDescriptor)) {
        logger.log("HTTP server can't initialize the socket: "+socket.errorString()+".", Logger::ERROR);
        return;
    } //endif

    // Switch on encryption, if enabled
    if (useSsl) {
        socket.setSslConfiguration(sslConfiguration);
        socket.startServerEncryption();
    } //endif

    // Start timer for read timeout
    int readTimeout=settings->value("HttpServer/readtimeout", 10000).toInt();
    readTimer.start(readTimeout);
    // delete previous request
    delete currentRequest;
    currentRequest=0;
} //endfunction handleConnection

bool HttpConnectionHandler::isBusy() {
    return busy;
} //endfunction isBusy

void HttpConnectionHandler::setBusy() {
    this->busy = true;
} //endfunction setBusy

void HttpConnectionHandler::readTimeout() {
    socket.disconnectFromHost();
    delete currentRequest;
    currentRequest=0;
} //endfunction readTimeout

void HttpConnectionHandler::disconnected() {
    logger.log("HTTP server disconnected client "+socket.peerAddress().toString()+".", Logger::OK);
    requestHandler->stop();
    socket.close();
    readTimer.stop();
    busy = false;
} //endfunction disconnected

void HttpConnectionHandler::read() {
    // The loop adds support for HTTP pipelinig
    while (socket.bytesAvailable()) {
        // Create new HttpRequest object if necessary
        if (!currentRequest) {
            currentRequest = new HttpRequest(settings);
        } //endif

        // Collect data for the request object
        while (socket.bytesAvailable() && currentRequest->getStatus()!=HttpRequest::complete && currentRequest->getStatus()!=HttpRequest::abort) {
            currentRequest->readFromSocket(socket);
            if (currentRequest->getStatus()==HttpRequest::waitForBody) {
                // Restart timer for read timeout, otherwise it would
                // expire during large file uploads.
                int readTimeout = settings->value("HttpServer/readtimeout", 10000).toInt();
                readTimer.start(readTimeout);
            } //endif
        } //endwhile

        // If the request is aborted, return error message and close the connection
        if (currentRequest->getStatus() == HttpRequest::abort) {
            socket.write("HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n");
            socket.disconnectFromHost();
            delete currentRequest;
            currentRequest=0;
            return;
        } //endif

        // If the request is complete, let the request mapper dispatch it
        if (currentRequest->getStatus() == HttpRequest::complete) {
            readTimer.stop();
            HttpResponse response(&socket);
            try {
                requestHandler->service(*currentRequest, response);
            } catch (...) {
                logger.log("HTTP server encountered an uncaught exception in the request handler.", Logger::ERROR);
            } //endtry

            // Finalize sending the response if not already done
            if (!response.hasSentLastPart()) {
                response.write(QByteArray(),true);
            } //endif
            // Close the connection after delivering the response, if requested
            if (QString::compare(currentRequest->getHeader("Connection"),"close", Qt::CaseInsensitive)==0) {
                socket.disconnectFromHost();
            } else {
                // Start timer for next request
                int readTimeout=settings->value("HttpServer/readtimeout", 10000).toInt();
                readTimer.start(readTimeout);
            } //endif
            // Prepare for next request
            delete currentRequest;
            currentRequest = 0;
        } //endif
    } //endwhile
} //endfunction read
