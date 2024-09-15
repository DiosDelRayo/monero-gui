// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "UrSettingsDialog.h"
#include "ui_UrSettingsDialog.h"
#include <QSpinBox>

UrSettingsDialog::UrSettingsDialog(QWidget *parent, int fragmentLength, int speed, bool fountainCodeEnabled)
        : QDialog(parent)
        , ui(new Ui::UrSettingsDialog)
{
#ifndef Q_OS_MACOS
    this->setWindowModality(Qt::WindowModal);
#endif

    ui->setupUi(this);

    this->updateSettings(fragmentLength, speed, fountainCodeEnabled);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(ui->spin_fragmentLength, &QSpinBox::valueChanged, [this](int value){
	emit this->fragmentLengthChanged(value);
	this->onUpdate();
    });
    connect(ui->spin_speed, &QSpinBox::valueChanged, [this](int value){
    emit this->fragmentLengthChanged(value);
    this->onUpdate();
    });
#else
    connect(ui->spin_speed, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this](int value) {
                emit this->speedChanged(value);
        this->onUpdate();
    });
    connect(ui->spin_speed, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int value){
        emit this->fragmentLengthChanged(value);
        this->onUpdate();
    });
#endif
    connect(ui->check_fountainCode, &QCheckBox::toggled, [this](bool toggled){
	emit this->fountainCodeEnabledChanged(toggled);
	this->onUpdate();
    });
    
    connect(ui->btn_reset, &QPushButton::clicked, [this]{
	this->updateSettings(this->defaultFragmentLength, this->defaultSpeed, this->defaultFountainCodeEnabled);
	this->onUpdate();
    });
   
    this->adjustSize();
}

void UrSettingsDialog::setDefault(int fragmentLength, int speed, bool fountainCodeEnabled) {
	this->defaultFragmentLength = fragmentLength;
	this->defaultSpeed = speed;
	this->defaultFountainCodeEnabled = fountainCodeEnabled;
}

int UrSettingsDialog::getFragmentLength() const {
    return ui->spin_fragmentLength->value();
}

int UrSettingsDialog::getSpeed() const {
        return ui->spin_fragmentLength->value();
}

bool UrSettingsDialog::getFountainCodeEnabled() const {
        return ui->check_fountainCode->isChecked();
}

void UrSettingsDialog::onUpdate() {
	emit this->settingsChanged(this->getFragmentLength(), this->getSpeed(), this->getFountainCodeEnabled());
}

void UrSettingsDialog::updateSettings(int fragmentLength, int speed, bool fountainCodeEnabled) {
	this->setFragmentLength(fragmentLength);
	this->setSpeed(speed);
	this->setFountainCodeEnabled(fountainCodeEnabled);
}

void UrSettingsDialog::setFragmentLength(int value) {
    ui->spin_fragmentLength->setValue(value);
}

void UrSettingsDialog::setSpeed(int value) {
    ui->spin_speed->setValue(value);
}

void UrSettingsDialog::setFountainCodeEnabled(bool enabled) {
    ui->check_fountainCode->setChecked(enabled);
}

UrSettingsDialog::~UrSettingsDialog() = default;
