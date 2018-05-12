#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "../version.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent, const librepcb::Version& appVersion, const QString& gitVersion);
    ~AboutDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
