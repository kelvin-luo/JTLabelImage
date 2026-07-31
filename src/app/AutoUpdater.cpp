#include "AutoUpdater.h"

#include <QDateTime>
#include <QDir>
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

namespace {

constexpr const char* kUserAgent =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) JTLabelImage/1.0";

QString httpErrorText(QNetworkReply* reply) {
    QString reason = reply->errorString();
    const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (http > 0)
        reason += QStringLiteral(" (HTTP %1)").arg(http);
    const QByteArray ct = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    if (!ct.isEmpty())
        reason += QStringLiteral(" [%1]").arg(QString::fromUtf8(ct));
    return reason;
}

QString guessFileName(QNetworkReply* reply, const QString& fallbackUrl) {
    QString fname;
    const QByteArray cd = reply->rawHeader("Content-Disposition");
    const int idx = cd.toLower().indexOf("filename=");
    if (idx >= 0) {
        fname = QString::fromUtf8(cd.mid(idx + 9)).trimmed();
        if (fname.startsWith(QLatin1Char('"'))) {
            const int end = fname.indexOf(QLatin1Char('"'), 1);
            if (end > 1) fname = fname.mid(1, end - 1);
        } else {
            const int semi = fname.indexOf(QLatin1Char(';'));
            if (semi > 0) fname = fname.left(semi).trimmed();
        }
        const int star = fname.indexOf(QStringLiteral("''"));
        if (star >= 0)
            fname = QUrl::fromPercentEncoding(fname.mid(star + 2).toUtf8());
    }
    if (fname.isEmpty()) {
        const QUrl finalUrl = reply->url().isValid() ? reply->url() : QUrl(fallbackUrl);
        fname = QFileInfo(finalUrl.path()).fileName();
    }
    if (fname.isEmpty())
        fname = QStringLiteral("JTLabelImage-update.bin");
    return QFileInfo(fname).fileName();
}

QNetworkRequest makeRequest(const QUrl& url) {
    QNetworkRequest req{url};
    req.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setMaximumRedirectsAllowed(15);
    // 0 = no transfer timeout (large release packages).
    req.setTransferTimeout(0);
    return req;
}

}  // namespace

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
    QNetworkRequest req = makeRequest(QUrl(m_versionUrl));
    req.setRawHeader("Accept", "application/json,text/plain,*/*");
    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QUrl failed = reply->url();
            if (http == 404
                && failed.host().contains(QStringLiteral("kelvin-luo.github.io"))
                && failed.path().endsWith(QStringLiteral("version.json"))) {
                const QString fallback =
                    QStringLiteral("https://raw.githubusercontent.com/kelvin-luo/kelvin-luo.github.io/master/version.json");
                emit logMessage(QStringLiteral("Pages 不可用，回退: %1").arg(fallback));
                QNetworkReply* reply2 = m_nam->get(makeRequest(QUrl(fallback)));
                connect(reply2, &QNetworkReply::finished, this, [this, reply2]() {
                    reply2->deleteLater();
                    handleVersionReply(reply2);
                });
                return;
            }
            emit checkFailed(httpErrorText(reply));
            return;
        }
        handleVersionReply(reply);
    });
}

void AutoUpdater::handleVersionReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit checkFailed(httpErrorText(reply));
        return;
    }
    const QByteArray body = reply->readAll();
    const QJsonObject root = QJsonDocument::fromJson(body).object();
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
}

void AutoUpdater::downloadUpdate(const QString& downloadUrl) {
    const QString url = downloadUrl.isEmpty() ? m_pendingDownloadUrl : downloadUrl;
    if (url.isEmpty()) {
        emit downloadFailed(QStringLiteral("下载地址为空"));
        return;
    }
    if (m_activeDownload) {
        emit downloadFailed(QStringLiteral("已有下载任务进行中"));
        return;
    }

    emit logMessage(QStringLiteral("开始下载: %1").arg(url));
    m_lastLoggedPct = -1;

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (baseDir.isEmpty())
        baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString outDir = QDir(baseDir).filePath(QStringLiteral("JTLabelImage_updates"));
    if (!QDir().mkpath(outDir)) {
        emit downloadFailed(QStringLiteral("无法创建目录: %1").arg(outDir));
        return;
    }

    // Temporary name until final filename is known from headers / URL.
    const QString tmpPath = QDir(outDir).filePath(
        QStringLiteral("JTLabelImage-download-%1.part")
            .arg(QDateTime::currentMSecsSinceEpoch()));

    auto* file = new QFile(tmpPath);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString err = file->errorString();
        delete file;
        emit downloadFailed(QStringLiteral("无法写入文件: %1 (%2)").arg(tmpPath, err));
        return;
    }

    QNetworkRequest req = makeRequest(QUrl(url));
    req.setRawHeader("Accept", "application/octet-stream,*/*");
    QNetworkReply* reply = m_nam->get(req);
    m_activeDownload = reply;
    file->setParent(reply);  // auto-delete with reply

    connect(reply, &QNetworkReply::readyRead, this, [reply, file]() {
        if (file->isOpen())
            file->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                emit downloadProgress(received, total);
                if (total > 0) {
                    const int pct = int(received * 100 / total);
                    if (pct != m_lastLoggedPct && (pct % 10 == 0 || pct == 100)) {
                        m_lastLoggedPct = pct;
                        emit logMessage(QStringLiteral("下载进度: %1% (%2 / %3 字节)")
                                            .arg(pct)
                                            .arg(received)
                                            .arg(total));
                    }
                }
            });
    connect(reply, &QNetworkReply::finished, this, [this, reply, file, url, outDir, tmpPath]() {
        m_activeDownload = nullptr;
        reply->deleteLater();

        if (file->isOpen()) {
            if (reply->bytesAvailable() > 0)
                file->write(reply->readAll());
            file->flush();
            file->close();
        }

        const qint64 size = QFileInfo(tmpPath).size();
        if (reply->error() != QNetworkReply::NoError) {
            QFile::remove(tmpPath);
            emit downloadFailed(httpErrorText(reply));
            return;
        }

        const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (http >= 400) {
            QFile::remove(tmpPath);
            emit downloadFailed(QStringLiteral("下载失败 HTTP %1").arg(http));
            return;
        }

        // Reject tiny responses that are likely HTML error pages.
        if (size < 1024) {
            QFile::remove(tmpPath);
            emit downloadFailed(
                QStringLiteral("下载内容过小 (%1 字节)，可能不是安装包。最终 URL: %2")
                    .arg(size)
                    .arg(reply->url().toString()));
            return;
        }

        const QByteArray ct = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray().toLower();
        if (ct.contains("text/html")) {
            QFile::remove(tmpPath);
            emit downloadFailed(QStringLiteral("服务器返回 HTML 而非安装包: %1")
                                    .arg(reply->url().toString()));
            return;
        }

        const QString fname = guessFileName(reply, url);
        const QString outPath = QDir::cleanPath(QDir(outDir).filePath(fname));
        if (QFile::exists(outPath))
            QFile::remove(outPath);
        if (!QFile::rename(tmpPath, outPath)) {
            // Cross-device rename can fail; fall back to copy.
            if (!QFile::copy(tmpPath, outPath)) {
                QFile::remove(tmpPath);
                emit downloadFailed(QStringLiteral("无法保存到: %1").arg(outPath));
                return;
            }
            QFile::remove(tmpPath);
        }

        emit logMessage(QStringLiteral("下载完成: %1 (%2 字节)")
                            .arg(QDir::toNativeSeparators(outPath))
                            .arg(QFileInfo(outPath).size()));
        emit downloadFinished(outPath);
    });
}
