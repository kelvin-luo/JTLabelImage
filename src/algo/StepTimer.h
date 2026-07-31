#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

class StepTimer {
public:
    void reset();
    void start(const QString& stepName);
    void stop();
    QString dumpSummary() const;
    bool isRunning() const { return m_running; }

private:
    struct Entry {
        QString name;
        qint64  ms{0};
    };

    QVector<Entry> m_entries;
    QString m_currentName;
    qint64  m_startMs{0};
    bool    m_running{false};
};
