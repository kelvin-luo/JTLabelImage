#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class AutoUpdater : public QObject {
    Q_OBJECT
public:
    explicit AutoUpdater(QObject* parent = nullptr);

    void setVersionUrl(const QString& url);
    void setLocalVersion(const QString& version);
    void checkForUpdates();

signals:
    void logMessage(const QString& msg);
    void updateAvailable(const QString& latestVersion,
                         const QString& downloadUrl,
                         const QString& notes);
    void upToDate(const QString& latestVersion);
    void checkFailed(const QString& reason);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(const QString& localPath);
    void downloadFailed(const QString& reason);

public slots:
    void downloadUpdate(const QString& downloadUrl);

private:
    static int compareVersions(const QString& a, const QString& b);
    void handleVersionReply(QNetworkReply* reply);

    QNetworkAccessManager* m_nam{nullptr};
    QNetworkReply* m_activeDownload{nullptr};
    QString m_versionUrl;
    QString m_localVersion;
    QString m_pendingDownloadUrl;
    int m_lastLoggedPct{-1};
};
