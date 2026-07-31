#include "AutoUpdater.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>
#include <QVersionNumber>

AutoUpdater::AutoUpdater(QObject* parent) : QObject(parent) {
    m_nam = new QNetworkAccessManager(this);
}

void AutoUpdater::setVersionUrl(const QString& url) {
    m_versionUrl = url;
}

void AutoUpdater::setLocalVersion(const QString& version) {
    m_localVersion = version;
}

int AutoUpdater::compareVersions(const QString& a, const QString& b) {
    const QVersionNumber va = QVersionNumber::fromString(a);
    const QVersionNumber vb = QVersionNumber::fromString(b);
    return QVersionNumber::compare(va, vb);
}

void AutoUpdater::checkForUpdates() {
    if (m_versionUrl.isEmpty()) {
        emit checkFailed(QStringLiteral("更新 URL 为空"));
        return;
    }
    emit logMessage(QStringLiteral("检查更新: %1").arg(m_versionUrl));
    QNetworkRequest req{QUrl(m_versionUrl)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "JTLabelImage");
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit checkFailed(reply->errorString());
            return;
        }
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        const QString latest = root.value("latest_version").toString();
        const QString url = root.value("download_url").toString();
        const QString notes = root.value("notes").toString();
        if (latest.isEmpty() || url.isEmpty()) {
            emit checkFailed(QStringLiteral("version.json 缺少 latest_version / download_url"));
            return;
        }
        emit logMessage(QStringLiteral("云端版本 %1，本地版本 %2")
                            .arg(latest, m_localVersion));
        if (compareVersions(latest, m_localVersion) > 0) {
            m_pendingDownloadUrl = url;
            emit updateAvailable(latest, url, notes);
        } else {
            emit logMessage(QStringLiteral("已是最新版本"));
            emit upToDate(latest);
        }
    });
}

void AutoUpdater::downloadUpdate(const QString& downloadUrl) {
    const QString url = downloadUrl.isEmpty() ? m_pendingDownloadUrl : downloadUrl;
    if (url.isEmpty()) {
        emit downloadFailed(QStringLiteral("下载地址为空"));
        return;
    }
    emit logMessage(QStringLiteral("开始下载: %1").arg(url));
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "JTLabelImage");
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                emit downloadProgress(received, total);
            });
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit downloadFailed(reply->errorString());
            return;
        }
        const QUrl qu(url);
        QString fname = QFileInfo(qu.path()).fileName();
        if (fname.isEmpty()) fname = QStringLiteral("JTLabelImage-update.bin");
        const QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        const QString outPath = tmpDir + "/" + fname;
        QFile f(outPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            emit downloadFailed(QStringLiteral("无法写入临时文件: %1").arg(outPath));
            return;
        }
        f.write(reply->readAll());
        f.close();
        emit logMessage(QStringLiteral("下载完成: %1").arg(outPath));
        emit downloadFinished(outPath);
    });
}
