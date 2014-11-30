#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QHash>
#include <QUuid>
#include <QFileSystemWatcher>
#include <Log4Qt/Logger>
#include "appinfo.h"

class AppManager : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER

public:
    explicit AppManager(QObject *parent = 0);

    QStringList appPaths() const;

    AppInfo info(const QUuid &uuid) const;
    AppInfo info(const QString &shortName) const;

public slots:
    void rescan();

private:
    void scanApp(const QString &path);

private:
    QFileSystemWatcher *_watcher;
    QHash<QUuid, AppInfo> _apps;
    QHash<QString, QUuid> _names;
};

#endif // APPMANAGER_H
