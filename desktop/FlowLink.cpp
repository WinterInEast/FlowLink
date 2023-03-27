#include <QLabel>
#include <QInputDialog>
#include <QTableWidget>
#include <QDebug>

#include "FlowLink.h"
#include "./ui_FlowLink.h"

using namespace ads;

FlowLink::FlowLink(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::FlowLink),
      receiver(new Receiver)
{
    ui->setupUi(this);

    // dock manager
    CDockManager::setConfigFlag(CDockManager::OpaqueSplitterResize, true);
    CDockManager::setConfigFlag(CDockManager::XmlCompressionEnabled, false);
    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setAutoHideConfigFlags(CDockManager::DefaultAutoHideConfig);
    dockManager = new CDockManager(this);

    // toolbar
    createConnectionUi();
    createPerspectiveUi();

    // central widget
    QLabel *l = new QLabel();
    l->setText("Welcome");
    l->setAlignment(Qt::AlignCenter);
    CDockWidget *centralDockWidget = new CDockWidget("Central Widget");
    centralDockWidget->setWidget(l);
    auto *centralDockArea = dockManager->setCentralWidget(centralDockWidget);
    centralDockArea->setAllowedAreas(DockWidgetArea::OuterDockAreas);

    // device list widget
    lv = new QListView();
    CDockWidget *lvDockWidget = new CDockWidget("Device List");
    lvDockWidget->setWidget(lv);
    lvDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
    lvDockWidget->setMinimumSize(200, 150);
    dockManager->addDockWidget(DockWidgetArea::LeftDockWidgetArea, lvDockWidget, centralDockArea);
    ui->menuView->addAction(lvDockWidget->toggleViewAction());

    // properties table widget
    QTableWidget *propertiesTable = new QTableWidget();
    propertiesTable->setColumnCount(3);
    propertiesTable->setRowCount(10);
    CDockWidget *propertiesDockWidget = new CDockWidget("Properties");
    propertiesDockWidget->setWidget(propertiesTable);
    propertiesDockWidget->setMinimumSizeHintMode(CDockWidget::MinimumSizeHintFromDockWidget);
    propertiesDockWidget->resize(200, 150);
    propertiesDockWidget->setMinimumSize(200, 150);
    dockManager->addDockWidget(DockWidgetArea::RightDockWidgetArea, propertiesDockWidget, centralDockArea);
    ui->menuView->addAction(propertiesDockWidget->toggleViewAction());
}

FlowLink::~FlowLink()
{
    delete ui;
}

void FlowLink::createConnectionUi()
{
    connectAction = new QAction("Connect", this);
    connect(connectAction, SIGNAL(triggered()), SLOT(receiver->createConnection()));
    connect(receiver, &Receiver::sendHostInfo, this, &FlowLink::addDevice);
    ui->toolBar->addAction(connectAction);
}

void FlowLink::createPerspectiveUi()
{
    savePerspectiveAction = new QAction("Create Perspective", this);
    connect(savePerspectiveAction, SIGNAL(triggered()), SLOT(savePerspective()));
    perspectiveListAction = new QWidgetAction(this);
    perspectiveComboBox = new QComboBox(this);
    perspectiveComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    perspectiveComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(perspectiveComboBox, SIGNAL(currentTextChanged(const QString &)),
            dockManager, SLOT(openPerspective(const QString &)));
    perspectiveListAction->setDefaultWidget(perspectiveComboBox);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(perspectiveListAction);
    ui->toolBar->addAction(savePerspectiveAction);
}

void FlowLink::addDevice()
{
    QStringListModel *model = new QStringListModel(this);
    QStringList list;
    // TODO: add the real host name and address
    list << "Host";
    model->setStringList(list);
    lv->setModel(model);
    delete model;
}

void FlowLink::savePerspective()
{
    QString perspectiveName = QInputDialog::getText(this, "Save Perspective", "Perspective Name:");
    if (perspectiveName.isEmpty())
    {
        return;
    }

    dockManager->addPerspective(perspectiveName);
    QSignalBlocker Blocker(perspectiveComboBox);
    perspectiveComboBox->clear();
    perspectiveComboBox->addItems(dockManager->perspectiveNames());
    perspectiveComboBox->setCurrentText(perspectiveName);
}

void FlowLink::closeEvent(QCloseEvent *event)
{
    // To delete all floating widgets, remove the dock manager.
    // This will ensure that all top-level windows associated with the dock manager are closed correctly.
    dockManager->deleteLater();
    QMainWindow::closeEvent(event);
}