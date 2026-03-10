#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "xmlparser.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("XML Graph Visualizer");
    resize(1200, 800);

    setupMenu();

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(10, 10, 10, 10);

    graphWidget = new GraphWidget(this);
    layout->addWidget(graphWidget);

    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupMenu() {
    QMenuBar *menuBar = this->menuBar();

    QMenu *fileMenu = menuBar->addMenu("Файл");

    QAction *openAction = new QAction("Открыть XML...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    fileMenu->addAction(openAction);
    fileMenu->addSeparator();

    QAction *exitAction = new QAction("Выход", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    fileMenu->addAction(exitAction);

    QMenu *viewMenu = menuBar->addMenu("Вид");

    QAction *zoomInAction = new QAction("Увеличить", this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, [this]() {
        graphWidget->scale(1.2, 1.2);
    });
    viewMenu->addAction(zoomInAction);

    QAction *zoomOutAction = new QAction("Уменьшить", this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, [this]() {
        graphWidget->scale(0.8, 0.8);
    });
    viewMenu->addAction(zoomOutAction);

    QAction *fitAction = new QAction("По размеру окна", this);
    fitAction->setShortcut(Qt::Key_F);
    connect(fitAction, &QAction::triggered, this, [this]() {
        graphWidget->fitGraph();
    });
    viewMenu->addAction(fitAction);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Открыть XML файл",
        "",
        "XML файлы (*.xml);;Все файлы (*.*)"
        );

    if (!fileName.isEmpty()) {
        GraphData graph = parseXmlGraph(fileName);
        graphWidget->loadGraph(graph);
    }
}
