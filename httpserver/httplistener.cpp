#include "httplistener.h"

HttpListener::HttpListener(QSettings* settings, HttpRequestHandler* requestHandler, QObject *parent) : QTcpServer(parent) {
    // Reqister type of socketDescriptor for signal/slot handling
    qRegisterMetaType<tSocketDescriptor>("tSocketDescriptor");
    // Create connection handler pool
    this->settings = settings;
    pool=new HttpConnectionHandlerPool(settings, requestHandler);
    // Start listening
    QString host = settings->value("HttpServer/host", "").toString();
    int port=settings->value("HttpServer/port", 8080).toInt();
    listen(host.isEmpty() ? QHostAddress::Any : QHostAddress(host), port);
    if (!isListening()) {
        logger.log("HTTP server can't bind to desired port.", Logger::ERROR);
    } //endif
} //endconstructor

HttpListener::~HttpListener() {
    close();
    pool->deleteLater();
} //enddestructor

void HttpListener::incomingConnection(tSocketDescriptor socketDescriptor) {
    HttpConnectionHandler* freeHandler=pool->getConnectionHandler();

    // Let the handler process the new connection.
    if (freeHandler) {
        // The descriptor is passed via signal/slot because the handler lives in another
        // thread and cannot open the socket when called by another thread.
        connect(this,SIGNAL(handleConnection(tSocketDescriptor)),freeHandler,SLOT(handleConnection(tSocketDescriptor)));
        emit handleConnection(socketDescriptor);
        disconnect(this,SIGNAL(handleConnection(tSocketDescriptor)),freeHandler,SLOT(handleConnection(tSocketDescriptor)));
    } else {
        // Reject the connection
        logger.log("HTTP server too many incoming connections.", Logger::WARNING);
        QTcpSocket* socket=new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        socket->write("HTTP/1.1 503 too many connections\r\nConnection: close\r\n\r\nToo many connections\r\n");
        socket->disconnectFromHost();
    } //endif
} //endfunction socketDescriptor
