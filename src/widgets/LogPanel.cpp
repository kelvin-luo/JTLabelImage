#include "LogPanel.h"

#include <QDateTime>
#include <QVBoxLayout>
#include <QTextEdit>

LogPanel::LogPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_edit = new QTextEdit(this);
    m_edit->setReadOnly(true);
    m_edit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_edit->setMinimumHeight(120);
    layout->addWidget(m_edit);
}

void LogPanel::append(const QString& text) {
    const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_edit->append(QString("[%1] %2").arg(ts, text));
}

void LogPanel::clear() {
    m_edit->clear();
}
