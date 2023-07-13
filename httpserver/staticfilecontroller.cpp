#include "staticfilecontroller.h"

StaticFileController::StaticFileController(QSettings *settings, QObject* parent) :HttpRequestHandler(parent) {
    maxAge = settings->value("HttpServer/filemaxage", 60000).toInt();
    encoding = settings->value("HttpServer/fileencoding","UTF-8").toString();
    docroot = settings->value("HttpServer/docroot",".").toString();
    // Convert relative path to absolute, based on the directory of the config file.
    if (QDir::isRelativePath(docroot)) {
        QFileInfo configFile(settings->fileName());
        docroot = QFileInfo(configFile.absolutePath(),docroot).absoluteFilePath();
    } //endif
    maxCachedFileSize = settings->value("HttpServer/maxcachedfilesize", 65536).toInt();
    cache.setMaxCost(settings->value("HttpServer/cachesize", 1000000).toInt());
    cacheTimeout = settings->value("HttpServer/cachetime", 60000).toInt();
} //endconstructor

void StaticFileController::service(HttpRequest& request, HttpResponse& response) {
    QByteArray path = request.getPath();
    // Check if we have the file in cache
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    mutex.lock();
    CacheEntry* entry = cache.object(path);
    if (entry && (cacheTimeout == 0 || entry->created>now-cacheTimeout)) {
        QByteArray document = entry->document; //copy the cached document, because other threads may destroy the cached entry immediately after mutex unlock.
        QByteArray filename = entry->filename;
        mutex.unlock();
        setContentType(filename,response);
        response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
        response.write(document);
    } else {
        mutex.unlock();
        // The file is not in cache.
        // Forbid access to files outside the docroot directory
        if (path.contains("/..")) {
            response.setStatus(403,"forbidden");
            response.write("403 forbidden", true);
            return;
        } //endif
        // If the filename is a directory, append index.html.
        if (QFileInfo(docroot+path).isDir()) {
            path += "/index.html";
        } //endif
        // Try to open the file
        QFile file(docroot+path);
        if (file.open(QIODevice::ReadOnly)) {
            setContentType(path,response);
            response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
            if (file.size() <= maxCachedFileSize) {
                // Return the file content and store it also in the cache
                entry = new CacheEntry();
                while (!file.atEnd() && !file.error()) {
                    QByteArray buffer = file.read(65536);
                    response.write(buffer);
                    entry->document.append(buffer);
                } //endwhile
                entry->created = now;
                entry->filename = path;
                mutex.lock();
                cache.insert(request.getPath(),entry, entry->document.size());
                mutex.unlock();
            } else {
                // Return the file content, do not store in cache
                while (!file.atEnd() && !file.error()) {
                    response.write(file.read(65536));
                } //endwhile
            } //endif
            file.close();
        } else {
            if (file.exists()) {
                logger.log("HTTP server can't open file "+file.fileName()+".", Logger::WARNING);
                response.setStatus(403,"forbidden");
                response.write("403 forbidden",true);
            } else {
                response.setStatus(404,"not found");
                response.write("404 not found",true);
            } //endif
        } //endif
    } //endif
} //endfunction service

void StaticFileController::setContentType(QString fileName, HttpResponse& response) const {
    if (fileName.endsWith(".png")) {
        response.setHeader("Content-Type", "image/png");
    } else if (fileName.endsWith(".jpg")) {
        response.setHeader("Content-Type", "image/jpeg");
    } else if (fileName.endsWith(".gif")) {
        response.setHeader("Content-Type", "image/gif");
    } else if (fileName.endsWith(".pdf")) {
        response.setHeader("Content-Type", "application/pdf");
    } else if (fileName.endsWith(".txt")) {
        response.setHeader("Content-Type", qPrintable("text/plain; charset="+encoding));
    } else if (fileName.endsWith(".html") || fileName.endsWith(".htm")) {
        response.setHeader("Content-Type", qPrintable("text/html; charset="+encoding));
    } else if (fileName.endsWith(".css")) {
        response.setHeader("Content-Type", "text/css");
    } else if (fileName.endsWith(".js")) {
        response.setHeader("Content-Type", "text/javascript");
    } //endif
    // Todo: add all of your content types
} //endfunction setContentType
