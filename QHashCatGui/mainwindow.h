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
    void loadWordList();
    QMap<int,QString> hashTypeMap;
    QMap<int,QString> outputTypeMap;
protected:
    void closeEvent(QCloseEvent * event);
public slots:
    void handleClipboardHash();
    void handleOpenInput();
    void handleOpenOutput();
    void handleOpenSalt();
    void handleOpenProg();
    void processStarted();
    void startCracking();
    void showLog();
    void processDone(int,QProcess::ExitStatus);
    void createHashFile();
    void addWordList();
    void deleteWordList();
    void saveWordList(QObject* = 0);
    void moveWordUp();
    void moveWordDown();
};

#endif // MAINWINDOW_H
