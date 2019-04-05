#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QProcess>
#include "config.h"
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
    QString getHashcatVer();
    QStringList createCommand();
    QStringList commandV2();
    QStringList commandV3();
    QMap<int,QString> hashTypeMap;
    QMap<int,QString> outputTypeMap;
    config *lastcnf;
    QString createActionText(QString state);
    QString lastdir;
protected:
    void closeEvent(QCloseEvent * event);
	bool eventFilter(QObject *obj, QEvent *ev);
public slots:
	void handleKeyPress(QKeyEvent *e);
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
