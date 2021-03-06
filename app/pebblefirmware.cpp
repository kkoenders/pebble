#include "pebblefirmware.h"
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>

const QString PebbleFirmware::firmwareURL("http://pebblefw.s3.amazonaws.com/pebble/%1/%2/latest.json");

PebbleFirmware::PebbleFirmware(QObject *parent) :
    QObject(parent), nm(new QNetworkAccessManager(this))
{
    connect(nm, SIGNAL(finished(QNetworkReply*)), SLOT(onNetworkReplyFinished(QNetworkReply*)));
}

void PebbleFirmware::updateLatest(QString hw)
{
    QNetworkRequest req;
    req.setUrl(firmwareURL.arg(hw).arg(hw.startsWith("snowy_") ? "release-v3" : "release-v2"));
    req.setRawHeader("Cache-Control", "no-cache");
    qDebug() << "Getting latest firmware" << req.url();
    nm->get(req);
}

void PebbleFirmware::fetchFirmware(QString type)
{
    QNetworkRequest req;
    req.setUrl(_latest.value(type).toObject().value("url").toString());
    req.setRawHeader("Cache-Control", "no-cache");
    qDebug() << "Fetching firmware" << req.url();
    nm->get(req);
}

void PebbleFirmware::onNetworkReplyFinished(QNetworkReply* rep)
{
    qDebug() << "Download finished" << rep->url();

    if (rep->request().url().toString().endsWith("/latest.json")) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(rep->readAll());
        _latest = jsonResponse.object();
        if (!_latest.isEmpty()) {
            qDebug() << "Latest firmware" << _latest;
            emit latestChanged();
        }
    } else if (rep->url().toString().endsWith(".pbz")) {
        QDir downDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        QFileInfo name(rep->url().toString(QUrl::FullyDecoded|QUrl::NormalizePathSegments|
                                           QUrl::RemoveQuery|QUrl::RemoveFragment));
        QFile file(downDir.absoluteFilePath(name.fileName()));
        file.open(QIODevice::WriteOnly|QIODevice::Truncate);
        if (file.write(rep->readAll()) == -1) {
            qWarning() << "Error writing" << file.fileName() << file.errorString();
        } else {
            file.close();
            qDebug() << "Downloaded" << file.fileName();
            emit firmwareFetched(file.fileName());
        }
    } else {
        qWarning() << "Unrequested download" << rep->request().url();
    }
}
