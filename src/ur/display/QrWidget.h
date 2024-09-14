// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef QRWIDGET_H
#define QRWIDGET_H

#include <QWidget>

#include "../qrcode/QrCode.h"

class QrWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QrWidget(QWidget *parent = nullptr);
    void setQrCode(QrCode *qrCode);

protected:
    void paintEvent(QPaintEvent *event) override;
    int heightForWidth(int w) const override;
    bool hasHeightForWidth() const override;

private:
    QrCode *m_qrcode = nullptr;
};

#endif //QRWIDGET_H
