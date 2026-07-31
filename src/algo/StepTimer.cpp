#include "StepTimer.h"

#include <QDateTime>

void StepTimer::reset() {
    if (m_running) stop();
    m_entries.clear();
}

void StepTimer::start(const QString& stepName) {
    if (m_running) stop();
    m_currentName = stepName;
    m_startMs = QDateTime::currentMSecsSinceEpoch();
    m_running = true;
}

void StepTimer::stop() {
    if (!m_running) return;
    const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_startMs;
    m_entries.append({m_currentName, elapsed});
    m_running = false;
    m_currentName.clear();
}

QString StepTimer::dumpSummary() const {
    if (m_entries.isEmpty())
        return QStringLiteral("[计时] 无记录步骤");

    qint64 total = 0;
    QStringList lines;
    lines << QStringLiteral("[计时汇总]");
    for (int i = 0; i < m_entries.size(); ++i) {
        const auto& e = m_entries[i];
        total += e.ms;
        lines << QStringLiteral("  %1. %2 — %3 ms").arg(i + 1).arg(e.name).arg(e.ms);
    }
    lines << QStringLiteral("  合计: %1 ms").arg(total);
    return lines.join('\n');
}
