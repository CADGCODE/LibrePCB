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

#ifndef LIBREPCB_ABOUTDIALOG_H
#define LIBREPCB_ABOUTDIALOG_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QDialog>
#include <QAbstractButton>
#include "../version.h"

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

namespace Ui {
class AboutDialog;
}

/*****************************************************************************************
 *  Class AboutDialog
 ****************************************************************************************/

/**
 * @brief The AboutDialog class
 */
class AboutDialog final : public QDialog
{
        Q_OBJECT

    public:

        // Constructors / Destructor
        AboutDialog() = delete;
        AboutDialog(const AboutDialog& other) = delete;
        AboutDialog(const Version& appVersion, const QString& gitVersion,
                    const QString& buildDate, QWidget* parent = nullptr) noexcept;
        ~AboutDialog() noexcept;

        // Operator Overloadings
        AboutDialog& operator=(const AboutDialog& rhs) = delete;

    private slots:
        void on_buttonBox_clicked(QAbstractButton *button) noexcept;

    private: // Data
        QScopedPointer<Ui::AboutDialog> mUi;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb

#endif // LIBREPCB_ABOUTDIALOG_H
