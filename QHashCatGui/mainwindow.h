#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QProcess>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QProcess *crackingProcess;
private:
    Ui::MainWindow *ui;
    void initializeHashType(QComboBox *com);
    void initializeOutputType(QComboBox *com);
    QMap<int,QString> hashTypeMap;
    QMap<int,QString> outputTypeMap;
public slots:
    void handleClipboardHash();
    void handleOpenInput();
    void handleOpenOutput();
    void processStarted();
    void startCracking();
    void showLog();
    void processDone(int,QProcess::ExitStatus);
};

#endif // MAINWINDOW_H
