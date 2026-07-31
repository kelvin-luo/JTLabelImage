#pragma once

#include <QWidget>

class QTextEdit;

class LogPanel : public QWidget {
    Q_OBJECT
public:
    explicit LogPanel(QWidget* parent = nullptr);

    void append(const QString& text);
    void clear();

private:
    QTextEdit* m_edit{nullptr};
};
