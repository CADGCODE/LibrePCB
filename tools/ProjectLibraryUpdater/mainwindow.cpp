#include <QtCore>
#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <librepcbcommon/fileio/smartxmlfile.h>
#include <librepcbcommon/fileio/xmldomdocument.h>
#include <librepcbcommon/fileio/xmldomelement.h>
#include <librepcblibrary/library.h>
#include <librepcblibrary/cmp/component.h>
#include <librepcblibrary/sym/symbol.h>
#include <librepcblibrary/dev/device.h>
#include <librepcblibrary/pkg/footprint.h>
#include <librepcblibrary/pkg/package.h>

using namespace library;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings s;
    restoreGeometry(s.value("mainwindow/geometry").toByteArray());
    restoreState(s.value("mainwindow/state").toByteArray());
    ui->workspacelibrarypath->setText(s.value("mainwindow/library_directory").toString());
    ui->projectfiles->addItems(s.value("mainwindow/projects").toStringList());
}

MainWindow::~MainWindow()
{
    QStringList projectList;
    for (int i = 0; i < ui->projectfiles->count(); i++)
        projectList.append(ui->projectfiles->item(i)->text());

    QSettings s;
    s.setValue("mainwindow/geometry", saveGeometry());
    s.setValue("mainwindow/state", saveState());
    s.setValue("mainwindow/library_directory", ui->workspacelibrarypath->text());
    s.setValue("mainwindow/projects", QVariant::fromValue(projectList));

    delete ui;
}

void MainWindow::on_libBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Workspace Library Folder",
                                                    ui->workspacelibrarypath->text());
    if (dir.isEmpty()) return;
    ui->workspacelibrarypath->setText(dir);
}

void MainWindow::on_addProjectBtn_clicked()
{
    ui->projectfiles->addItems(QFileDialog::getOpenFileNames(this, "Select Project File",
                                                             QString(), "*.lpp"));
}

void MainWindow::on_removeProjectBtn_clicked()
{
    qDeleteAll(ui->projectfiles->selectedItems());
}

void MainWindow::on_clrProjectBtn_clicked()
{
    ui->projectfiles->clear();
}

void MainWindow::on_pushButton_2_clicked()
{
    if (ui->workspacelibrarypath->text().isEmpty()) return;
    if (ui->projectfiles->count() == 0) return;
    ui->log->clear();

    try
    {
        FilePath libDir(ui->workspacelibrarypath->text());
        Library lib(libDir, libDir.getPathTo(QString("../.metadata/v%1/library_cache.sqlite").arg(APP_VERSION_MAJOR)));

        for (int i = 0; i < ui->projectfiles->count(); i++)
        {
            FilePath projectFilepath(ui->projectfiles->item(i)->text());
            SmartXmlFile projectFile(projectFilepath, false, true);
            QSharedPointer<XmlDomDocument> projectDoc = projectFile.parseFileAndBuildDomTree(true);

            // components & symbols
            SmartXmlFile circuitFile(projectFilepath.getParentDir().getPathTo("core/circuit.xml"), false, true);
            QSharedPointer<XmlDomDocument> circuitDoc = circuitFile.parseFileAndBuildDomTree(true);
            for (XmlDomElement* node = circuitDoc->getRoot().getFirstChild("component_instances/*", true, false);
                 node; node = node->getNextSibling())
            {
                QUuid compUuid = node->getAttribute<QUuid>("component", true);
                FilePath filepath = lib.getLatestComponent(compUuid);
                if (!filepath.isValid())
                {
                    throw RuntimeError(__FILE__, __LINE__, projectFilepath.toStr(),
                        QString("missing component: %1").arg(compUuid.toString()));
                }
                // copy component
                Component latestComp(filepath);
                FilePath dest = projectFilepath.getParentDir().getPathTo("library/cmp");
                latestComp.saveTo(dest);
                ui->log->addItem(latestComp.getDirectory().toNative());

                // search all required symbols
                foreach (const ComponentSymbolVariant* symbvar, latestComp.getSymbolVariants())
                {
                    foreach (const ComponentSymbolVariantItem* item, symbvar->getItems())
                    {
                        QUuid symbolUuid = item->getSymbolUuid();
                        FilePath filepath = lib.getLatestSymbol(symbolUuid);
                        if (!filepath.isValid())
                        {
                            throw RuntimeError(__FILE__, __LINE__, projectFilepath.toStr(),
                                QString("missing symbol: %1").arg(symbolUuid.toString()));
                        }
                        Symbol latestSymbol(filepath);
                        FilePath dest = projectFilepath.getParentDir().getPathTo("library/sym");
                        latestSymbol.saveTo(dest);
                        ui->log->addItem(latestSymbol.getDirectory().toNative());
                    }
                }
            }


            // devices & footprints
            for (XmlDomElement* node = projectDoc->getRoot().getFirstChild("boards/*", true, false);
                 node; node = node->getNextSibling())
            {
                FilePath boardFilePath = projectFilepath.getParentDir().getPathTo("boards/" % node->getText(true));
                SmartXmlFile boardFile(boardFilePath, false, true);
                QSharedPointer<XmlDomDocument> boardDoc = boardFile.parseFileAndBuildDomTree(true);
                for (XmlDomElement* node = boardDoc->getRoot().getFirstChild("device_instances/*", true, false);
                     node; node = node->getNextSibling())
                {
                    QUuid deviceUuid = node->getAttribute<QUuid>("device", true);
                    FilePath filepath = lib.getLatestDevice(deviceUuid);
                    if (!filepath.isValid())
                    {
                        throw RuntimeError(__FILE__, __LINE__, projectFilepath.toStr(),
                            QString("missing device: %1").arg(deviceUuid.toString()));
                    }
                    // copy device
                    Device latestDevice(filepath);
                    FilePath dest = projectFilepath.getParentDir().getPathTo("library/dev");
                    latestDevice.saveTo(dest);
                    ui->log->addItem(latestDevice.getDirectory().toNative());

                    // get package
                    QUuid packUuid = latestDevice.getPackageUuid();
                    filepath = lib.getLatestPackage(packUuid);
                    if (!filepath.isValid())
                    {
                        throw RuntimeError(__FILE__, __LINE__, projectFilepath.toStr(),
                            QString("missing package: %1").arg(packUuid.toString()));
                    }
                    // copy package
                    Package latestPackage(filepath);
                    dest = projectFilepath.getParentDir().getPathTo("library/pkg");
                    latestPackage.saveTo(dest);
                    ui->log->addItem(latestPackage.getDirectory().toNative());
                }
            }
        }
    }
    catch (Exception& e)
    {
        ui->log->addItem("ERROR: " % e.getUserMsg());
    }

    ui->log->addItem("FINISHED");
    ui->log->setCurrentRow(ui->log->count()-1);
}

void MainWindow::on_rescanlib_clicked()
{
    if (ui->workspacelibrarypath->text().isEmpty()) return;

    try
    {
        FilePath libDir(ui->workspacelibrarypath->text());
        Library lib(libDir, libDir.getPathTo(QString("../.metadata/v%1/library_cache.sqlite").arg(APP_VERSION_MAJOR)));
        lib.rescan();
        QMessageBox::information(this, tr("Library Rescan"), tr("Successfully"));
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.getUserMsg());
    }
}
