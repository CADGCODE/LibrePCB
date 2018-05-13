/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2018 LibrePCB Developers, see AUTHORS.md for contributors.
 * http://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <librepcb/common/application.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

AboutDialog::AboutDialog(QWidget* parent) noexcept :
    QDialog(parent),
    mUi(new Ui::AboutDialog)
{
    // Set up base dialog
    mUi->setupUi(this);

    // Content alignment
    mUi->horizontalLayout->setAlignment(mUi->logo, Qt::AlignTop);
    mUi->aboutContentLayout->setAlignment(Qt::AlignTop);

    // Get some version information
    const Version& appVersion = qApp->getAppVersion();
    const QString& gitVersion = qApp->getGitVersion();
    const QString& buildDate = qApp->getBuildDate().toString("yyyy-MM-dd hh:mm:ss (t)");

    // Add content to about dialog
    QLabel *textIntro = new QLabel(tr("LibrePCB is a free & open source schematic/layout-editor."));
    QLabel *headerVersion = new QLabel("<b>" + tr("Version Info") + "</b>");
    QLabel *textVersion = new QLabel(QString("%1 (%2)<br>Build date: %3").arg(appVersion.toPrettyStr(3), gitVersion, buildDate));
    QLabel *headerLinks = new QLabel("<b>" + tr("Links") + "</b>");
    QLabel *textLinks = new QLabel(tr("For more information, please check out <a href='%1'>librepcb.org</a><br>or our <a href='%2'>GitHub repository</a>.").arg("http://librepcb.org/", "https://github.com/LibrePCB/LibrePCB"));
    QLabel *headerLicense = new QLabel("<b>" + tr("License") + "</b>");
    QLabel *textLicense = new QLabel(tr("LibrePCB is free software, released under the GNU General<br>Public License (GPL) version 3 or later. You can find the full<br>license text <a href='https://github.com/LibrePCB/LibrePCB/blob/master/LICENSE.txt'>in our source code</a>."));
    formatLabelHeading(headerVersion);
    formatLabelHeading(headerLinks);
    formatLabelHeading(headerLicense);
    formatLabelText(textIntro);
    formatLabelText(textVersion);
    formatLabelText(textLinks);
    formatLabelText(textLicense);
    mUi->aboutContentLayout->addWidget(textIntro);
    mUi->aboutContentLayout->addWidget(headerVersion);
    mUi->aboutContentLayout->addWidget(textVersion);
    mUi->aboutContentLayout->addWidget(headerLinks);
    mUi->aboutContentLayout->addWidget(textLinks);
    mUi->aboutContentLayout->addWidget(headerLicense);
    mUi->aboutContentLayout->addWidget(textLicense);
}

/**
 * @brief Format a heading label in the about dialog.
 * @param label Pointer to the QLabel instance
 */
void AboutDialog::formatLabelHeading(QLabel* label) noexcept
{
    int headerMarginTop = 8;
    label->setContentsMargins(0, headerMarginTop, 0, 0);
}

/**
 * @brief Format a text label in the about dialog.
 * @param label Pointer to the QLabel instance
 */
void AboutDialog::formatLabelText(QLabel* label) noexcept
{
    label->setOpenExternalLinks(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
}

AboutDialog::~AboutDialog() noexcept
{
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void AboutDialog::on_buttonBox_clicked(QAbstractButton *button) noexcept
{
    Q_UNUSED(button);
    this->close();
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
