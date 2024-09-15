// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef INFO_FRAME_H
#define INFO_FRAME_H

#include <QPushButton>
#include <QHBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QListWidget>


class InfoFrame : public QFrame {
    Q_OBJECT

public:
    explicit InfoFrame(QWidget *parent);
    void setInfo(const QIcon &icon, const QString &text);
    void setText(const QString &text);

private:
    QPushButton *m_icon;
    QLabel *m_infoLabel;
};

#endif // INFO_FRAME_H
