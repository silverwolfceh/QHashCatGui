#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include <QUuid>
#include <QFile>
#include <QClipboard>
#include <QMimeData>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    lastcnf = new config();
    crackingProcess = new QProcess(this);
    initializeHashType(ui->comboBox);
    initializeOutputType(ui->comboBox_2);
    loadWordList();
    ui->txtSalt->setDisabled(true);
	ui->txtLog->installEventFilter(this);
	//connect(ui->txtLog,SIGNAL(keyPressEvent()),this,SLOT(handleKeyPress()));
    connect(ui->btnclipboard,SIGNAL(clicked()),this,SLOT(handleClipboardHash()));
    connect(ui->btnBrowseInput,SIGNAL(clicked()),this,SLOT(handleOpenInput()));
    connect(ui->btnBrowseOutput,SIGNAL(clicked()),this,SLOT(handleOpenOutput()));
    connect(crackingProcess,SIGNAL(started()),this,SLOT(processStarted()));
    connect(crackingProcess,SIGNAL(finished(int, QProcess::ExitStatus)),this,SLOT(processDone(int,QProcess::ExitStatus)));
    connect(crackingProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(showLog()));
    connect(ui->btnAction,SIGNAL(clicked()),this,SLOT(startCracking()));
    connect(ui->btnCreate,SIGNAL(clicked()),this,SLOT(createHashFile()));
    connect(ui->btnAdd,SIGNAL(clicked()),this,SLOT(addWordList()));
    connect(ui->btnRemove,SIGNAL(clicked()),this,SLOT(deleteWordList()));
    connect(ui->btnBrowseSalt,SIGNAL(clicked()),this,SLOT(handleOpenSalt()));
    connect(ui->btnBrowseProg,SIGNAL(clicked()),this,SLOT(handleOpenProg()));
    connect(ui->btnUp,SIGNAL(clicked()),this,SLOT(moveWordUp()));
    connect(ui->btnDown,SIGNAL(clicked()),this,SLOT(moveWordDown()));
    ui->txtInputFile->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/input.txt");
    ui->txtOutputFile->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/output.txt");
#ifdef Q_OS_WIN32
    ui->txtProg->setText("hashcat-cli32.exe");
#endif
#ifdef Q_OS_DARWIN
    ui->txtProg->setText("/usr/local/bin/hashcat");
#endif
    if(lastcnf->lasthashpath != "")
        ui->txtInputFile->setText(lastcnf->lasthashpath);
    if(lastcnf->hashcatpath != "")
        ui->txtProg->setText(lastcnf->hashcatpath);
    if(lastcnf->separator != "")
        ui->sep->setText(lastcnf->separator);
    ui->tab_3->setFocus();
    createActionText("idle");
    if(lastcnf->lastdir != "")
        lastdir = lastcnf->lastdir;
    else
        lastdir = QDir::currentPath();
}

QString MainWindow::createActionText(QString state)
{
    if(state == "idle")
    {
        QStringList caption;
        caption << "Kill the bom";
        caption << "Powered by ANBU!";
        caption << "Expect us";
        caption << "Design by SilverWolf";
        caption << "Crack me please";
        caption << "Power of ATOM";
        caption << "..............Huhm";
        int captNum = qrand() % caption.count();
        ui->btnAction->setProperty("state", "idle");
        ui->btnAction->setText(caption[captNum]);
        return caption[captNum];
    }
    else
    {
        ui->btnAction->setProperty("state", "cracking");
        ui->btnAction->setText("Stop");
        return "Stop";
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->txtLog) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Q)
            {
				crackingProcess->terminate();
				return true;
			}
            return false;
        } else {
            return false;
        }
    } else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}

void MainWindow::handleKeyPress(QKeyEvent* e)
{
	if (e->key()==Qt::Key_Q)
	{
		qDebug() << "Q key pressed";
	}
	//QTextEdit::keyPressEvent(e);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    saveWordList();
}

QString MainWindow::getHashcatVer()
{
    return QString::fromStdString("1.0");
}

QStringList MainWindow::commandV2()
{
    QStringList args;
    args << "--hash-type=" + ui->comboBox->currentData().toString(); //hashtype
    args << "--outfile=" + ui->txtOutputFile->text(); //output
    args << "--outfile-format=" + ui->comboBox_2->currentData().toString();
    args << "--threads=" + ui->txtThread->text();
    if(ui->txtSalt->text().simplified() != "")
        args << "--salt-file=" + ui->txtSalt->text();
    args << ui->txtInputFile->text(); //hash file
    for(int i = 0; i < ui->listWidget->count(); i++)
        args << ui->listWidget->item(i)->text();
    return args;
}

QStringList MainWindow::commandV3()
{
    QStringList args;
    args << "--hash-type=" + ui->comboBox->currentData().toString(); //hashtype
    args << "--outfile=" + ui->txtOutputFile->text(); //output
    args << "--outfile-format=" + ui->comboBox_2->currentData().toString();
    args << "-a 0"; //attack mode
    args << "--separator=" + ui->sep->text(); //separator
    args << "--workload-profile=3"; //work load high
    args << "--status-timer=1"; //update timer
    args << "--status";
    args << "--force";
    args << "--potfile-disable";
    args << "--restore-disable";
    args << ui->txtInputFile->text(); //hash file
    for(int i = 0; i < ui->listWidget->count(); i++)
        args <<  "\"" + ui->listWidget->item(i)->text() + "\"" ;
    return args;
}

QStringList MainWindow::createCommand()
{
    return commandV3();
}

void MainWindow::startCracking()
{
    QString curstate = ui->btnAction->property("state").toString();
    if(curstate == "cracking")
    {
        createActionText("idle");
        crackingProcess->terminate();
    }
    else
    {
        QFileInfo file(ui->txtProg->text());
        if(!file.exists())
        {
            QMessageBox msgBox(this);
            msgBox.setText("Hashcat not found! Please download it");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            QDesktopServices::openUrl(QUrl("http://hashcat.net/hashcat/"));
            return;
        }

        QDir dir = file.absoluteDir();
        if(dir.entryList(QStringList() << "eula.accepted").count() == 0)
        {
            QFile file(dir.path() + QDir::separator() + "eula.accepted");
            file.open(QIODevice::ReadWrite | QIODevice::Text);
            QTextStream out(&file);
            out << "1\0";
            file.close();
        }

        QStringList args = createCommand();
        ui->txtLog->append("COMMAND: " + ui->txtProg->text() + " " + args.join(" "));
        createActionText("cracking");
        ui->tabWidget->setCurrentWidget(ui->tab_3);

		QString completeCmd = ui->txtProg->text();
		foreach (QString arg, args)
		{
			completeCmd += " " + arg;
		}
        crackingProcess->start(completeCmd);

        if(!crackingProcess->waitForStarted(60000))
        {
            createActionText("idle");
            ui->txtLog->append("Start failed");
            return;
        }
    }

}

void MainWindow::moveWordDown()
{
    QList<QListWidgetItem *> listItems = ui->listWidget->selectedItems();
    if(listItems.count() == 0) return;
    if(listItems.count() > 1)
    {
        QMessageBox msgBox(this);
        msgBox.setText("Move one item one time");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }
    if(ui->listWidget->row(listItems[0]) == ui->listWidget->count() - 1) return;
    int cur = ui->listWidget->row(listItems[0]);
    QString below = ui->listWidget->item(cur + 1)->text();
    QString current = ui->listWidget->item(cur)->text();
    ui->listWidget->takeItem(cur);
    ui->listWidget->insertItem(cur,below);
    ui->listWidget->takeItem(cur+1);
    ui->listWidget->insertItem(cur+1,current);

}

void MainWindow::moveWordUp()
{
    QList<QListWidgetItem *> listItems = ui->listWidget->selectedItems();
    if(listItems.count() == 0) return;
    if(listItems.count() > 1)
    {
        QMessageBox msgBox(this);
        msgBox.setText("Move one item one time");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }
    if(ui->listWidget->row(listItems[0]) == 0) return;
    int cur = ui->listWidget->row(listItems[0]);
    QString above = ui->listWidget->item(cur - 1)->text();
    QString current = ui->listWidget->item(cur)->text();
    ui->listWidget->takeItem(cur);
    ui->listWidget->insertItem(cur,above);
    ui->listWidget->takeItem(cur-1);
    ui->listWidget->insertItem(cur-1,current);
}

void MainWindow::saveWordList(QObject*)
{
    QStringList wl;
    for(int i = 0; i < ui->listWidget->count();i++)
        wl << ui->listWidget->item(i)->text();
    lastcnf->wordlists = wl;
}

void MainWindow::loadWordList()
{
    ui->listWidget->clear();
    QStringList wl = lastcnf->wordlists;
    for(int i =0; i < wl.count(); i++)
        ui->listWidget->addItem(wl[i].trimmed().simplified());
}

void MainWindow::addWordList()
{
    QStringList fnames = QFileDialog::getOpenFileNames(this,tr("Open hash file..."),lastdir, tr("Text Files (*.txt);; All Files (*.*)"));
	foreach (QString fname, fnames)
	{
		QFile file(fname);
    	if(!file.exists())
    	{
        	QMessageBox msgBox(this);
        	msgBox.setText("File " + fname + " is not exist!");
        	msgBox.setIcon(QMessageBox::Warning);
        	msgBox.exec();
        	lastdir = QFileInfo(fname).absoluteDir().absolutePath();
        	return;
    	}
    	lastdir = QFileInfo(fname).absoluteDir().absolutePath();
    	for(int i = 0; i < ui->listWidget->count();i++)
    	{
        	if(ui->listWidget->item(i)->text().compare(fname) == 0)
        	{
            	QMessageBox msgBox(this);
            	msgBox.setText("Wordlist " + fname + " is already exist!");
            	msgBox.setIcon(QMessageBox::Warning);
            	msgBox.exec();
            	return;
        	}
    	}
    	ui->listWidget->addItem(fname);
	}
}

void MainWindow::deleteWordList()
{
    QList<QListWidgetItem *> listItem = ui->listWidget->selectedItems();
    for(int i = 0; i < listItem.count(); i++)
        ui->listWidget->takeItem(ui->listWidget->row(listItem[i]));
}

void MainWindow::showLog()
{
    ui->txtLog->append(crackingProcess->readAllStandardOutput());
	// scroll to bottom
	QTextCursor c = ui->txtLog->textCursor();
	c.movePosition(QTextCursor::End);
	ui->txtLog->setTextCursor(c);
}


void MainWindow::processStarted()
{
    ui->tab_3->setFocus();
}

void MainWindow::processDone(int code, QProcess::ExitStatus status)
{
    ui->txtLog->append("-----Finished-----");
    ui->txtLog->append(QString::number(code));
    ui->btnAction->setEnabled(true);
    createActionText("idle");
}

void MainWindow::handleOpenSalt()
{
    QString fname = QFileDialog::getOpenFileName(this,tr("Open salt file..."),lastdir, tr("Text Files (*.txt);; All Files (*.*)"));
    if(fname.simplified() == "") return;
    QFile file(fname);
    if(!file.exists())
    {
        QMessageBox msgBox(this);
        msgBox.setText("File " + fname + " is not exist!");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        lastdir = QFileInfo(fname).absoluteDir().absolutePath();
        return;
    }
    ui->txtSalt->setText(fname);
    lastdir = QFileInfo(fname).absoluteDir().absolutePath();
}

void MainWindow::handleOpenProg()
{
    QString fname = QFileDialog::getOpenFileName(this,tr("Open hashcat client file..."),lastdir , tr("All Files (*.*)"));
    if(fname.simplified() == "") return;
    QFile file(fname);
    if(!file.exists())
    {
        QMessageBox msgBox(this);
        msgBox.setText("File " + fname + " is not exist!");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        lastdir = QFileInfo(fname).absoluteDir().absolutePath();
        return;
    }
    ui->txtProg->setText(fname);
    lastdir = QFileInfo(fname).absoluteDir().absolutePath();
}

void MainWindow::handleOpenInput()
{
    QString fname = QFileDialog::getOpenFileName(this,tr("Open hash file..."),lastdir , tr("Text Files (*.txt);; All Files (*.*)"));
    QFile file(fname);
    if(!file.exists())
    {
        QMessageBox msgBox(this);
        msgBox.setText("File " + fname + " is not exist!");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        lastdir = QFileInfo(fname).absoluteDir().absolutePath();
        return;
    }
    ui->txtInputFile->setText(fname);
    lastdir = QFileInfo(fname).absoluteDir().absolutePath();
}

void MainWindow::handleOpenOutput()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Output File..."),lastdir, tr("Text Files (*.txt);;All Files (*)"));
    QFile file(fname);
    if(file.exists())
    {
        QMessageBox msgBox(this);
        msgBox.setText("File " + fname + " will be override");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        lastdir = QFileInfo(fname).absoluteDir().absolutePath();
    }
    ui->txtOutputFile->setText(fname);
    lastdir = QFileInfo(fname).absoluteDir().absolutePath();
}

void MainWindow::handleClipboardHash()
{
    QString hashs = QApplication::clipboard()->mimeData()->text();
    QString filename = QUuid::createUuid().toString().left(5).right(4);
#ifdef Q_OS_WIN32
    QFile file("%TEMP%/hash_" + filename);
#else
    QFile file("/tmp/hash_" + filename);
#endif
    while(file.exists())
    {
        filename = QUuid::createUuid().toString();
#ifdef Q_OS_WIN32
        file.setFileName("%TEMP%/hash_" + filename);
#else
        file.setFileName("/tmp/hash_" + filename);
#endif
    }
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&file);
    out << hashs;
    file.close();
    ui->txtInputFile->setText(file.fileName());
}

void MainWindow::createHashFile()
{
    QApplication::clipboard()->setText(ui->txtHashEdit->toPlainText());
    handleClipboardHash();
    QMessageBox msgBox(this);
    msgBox.setText("Hash file created in: " + ui->txtInputFile->text());
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
    ui->tabWidget->setCurrentWidget(ui->tab);
}

void MainWindow::initializeOutputType(QComboBox *com)
{
    bool setindex = false;
    outputTypeMap.insert(1,"hash[:salt]");
    outputTypeMap.insert(2,"plain");
    outputTypeMap.insert(3,"hash[:salt]:plain");
    outputTypeMap.insert(4,"hex_plain");
    outputTypeMap.insert(5,"hash[:salt]:hex_plain");
    outputTypeMap.insert(6,"plain:hex_plain");
    outputTypeMap.insert(7,"hash[:salt]:plain:hex_plain");
    outputTypeMap.insert(8,"crackpos");
    outputTypeMap.insert(9,"hash[:salt]:crackpos");
    outputTypeMap.insert(10,"plain:crackpos");
    outputTypeMap.insert(11,"hash[:salt]:plain:crackpos");
    outputTypeMap.insert(12,"hex_plain:crackpos");
    outputTypeMap.insert(13,"hash[:salt]:hex_plain:crackpos");
    outputTypeMap.insert(14,"plain:hex_plain:crackpos");
    outputTypeMap.insert(15,"hash[:salt]:plain:hex_plain:crackpos");
    QList<QString> Values = outputTypeMap.values();
    for(int i = 0; i < Values.count(); i++)
    {
        com->addItem(Values[i],outputTypeMap.key(Values[i]));
        if(outputTypeMap.key(Values[i]) == lastcnf->outputtype)
        {
            setindex = true;
            com->setCurrentIndex(i);
        }
    }
    if(!setindex)
        com->setCurrentIndex(2);

}

void MainWindow::initializeHashType(QComboBox *com)
{
    hashTypeMap.insert(0,"MD5");
    hashTypeMap.insert(10,"md5($pass.$salt)");
    hashTypeMap.insert(20,"md5($salt.$pass)");
    hashTypeMap.insert(30,"md5(unicode($pass).$salt)");
    hashTypeMap.insert(40,"md5($salt.unicode($pass))");
    hashTypeMap.insert(50,"HMAC-MD5 (key= $pass)");
    hashTypeMap.insert(60,"HMAC-MD5 (key= $salt)");
    hashTypeMap.insert(100,"SHA1");
    hashTypeMap.insert(110,"sha1($pass.$salt)");
    hashTypeMap.insert(120,"sha1($salt.$pass)");
    hashTypeMap.insert(130,"sha1(unicode($pass).$salt)");
    hashTypeMap.insert(140,"sha1($salt.unicode($pass))");
    hashTypeMap.insert(150,"HMAC-SHA1 (key= $pass)");
    hashTypeMap.insert(160,"HMAC-SHA1 (key= $salt)");
    hashTypeMap.insert(200,"MySQL323");
    hashTypeMap.insert(300,"MySQL4.1/MySQL5");
    hashTypeMap.insert(400,"phpass, MD5(Wordpress), MD5(phpBB3), MD5(Joomla)");
    hashTypeMap.insert(500,"md5crypt, MD5(Unix), FreeBSD MD5, Cisco-IOS MD5");
    hashTypeMap.insert(900,"MD4");
    hashTypeMap.insert(1000,"NTLM");
    hashTypeMap.insert(1100,"Domain Cached Credentials, mscash");
    hashTypeMap.insert(1400,"SHA256");
    hashTypeMap.insert(1410,"sha256($pass.$salt)");
    hashTypeMap.insert(1420,"sha256($salt.$pass)");
    hashTypeMap.insert(1430,"sha256(unicode($pass).$salt)");
    hashTypeMap.insert(1440,"sha256($salt.unicode($pass))");
    hashTypeMap.insert(1450,"HMAC-SHA256 (key=$pass)");
    hashTypeMap.insert(1460,"HMAC-SHA256 (key=$salt)");
    hashTypeMap.insert(1600,"md5apr1, MD5(APR), Apache MD5");
    hashTypeMap.insert(1700,"SHA512");
    hashTypeMap.insert(1710,"sha512($pass.$salt)");
    hashTypeMap.insert(1720,"sha512($salt.$pass)");
    hashTypeMap.insert(1730,"sha512(unicode($pass).$salt)");
    hashTypeMap.insert(1740,"sha512($salt.unicode($pass))");
    hashTypeMap.insert(1750,"HMAC-SHA512 (key= $pass)");
    hashTypeMap.insert(1760,"HMAC-SHA512 (key= $salt)");
    hashTypeMap.insert(1800,"SHA-512(Unix)");
    hashTypeMap.insert(2400,"Cisco-PIX MD5");
    hashTypeMap.insert(2410,"Cisco-ASA MD5");
    hashTypeMap.insert(2500,"WPA/WPA2");
    hashTypeMap.insert(2600,"Double MD5");
    hashTypeMap.insert(3200,"bcrypt, Blowfish(OpenBSD)");
    hashTypeMap.insert(3300,"MD5(Sun)");
    hashTypeMap.insert(3500,"md5(md5(md5($pass)))");
    hashTypeMap.insert(3610,"md5(md5($salt).$pass)");
    hashTypeMap.insert(3710,"md5($salt.md5($pass))");
    hashTypeMap.insert(3720,"md5($pass.md5($salt))");
    hashTypeMap.insert(3810,"md5($salt.$pass.$salt)");
    hashTypeMap.insert(3910,"md5(md5($pass).md5($salt))");
    hashTypeMap.insert(4010,"md5($salt.md5($salt.$pass))");
    hashTypeMap.insert(4110,"md5($salt.md5($pass.$salt))");
    hashTypeMap.insert(4210,"md5($username.0.$pass)");
    hashTypeMap.insert(4300,"md5(strtoupper(md5($pass)))");
    hashTypeMap.insert(4400,"md5(sha1($pass))");
    hashTypeMap.insert(4500,"Double SHA1");
    hashTypeMap.insert(4600,"sha1(sha1(sha1($pass)))");
    hashTypeMap.insert(4700,"sha1(md5($pass))");
    hashTypeMap.insert(4710,"sha1($salt.$pass.$salt)");
    hashTypeMap.insert(4800,"MD5(Chap), iSCSI CHAP authentication");
    hashTypeMap.insert(5000,"SHA-3(Keccak)");
    hashTypeMap.insert(5100,"Half MD5");
    hashTypeMap.insert(5200,"Password Safe SHA-256");
    hashTypeMap.insert(5300,"IKE-PSK MD5");
    hashTypeMap.insert(5400,"IKE-PSK SHA1");
    hashTypeMap.insert(5500,"NetNTLMv1-VANILLA / NetNTLMv1-ESS");
    hashTypeMap.insert(5600,"NetNTLMv2");
    hashTypeMap.insert(5700,"Cisco-IOS SHA256");
    hashTypeMap.insert(5800,"Android PIN");
    hashTypeMap.insert(6300,"AIX {smd5}");
    hashTypeMap.insert(6400,"AIX {ssha256}");
    hashTypeMap.insert(6500,"AIX {ssha512}");
    hashTypeMap.insert(6700,"AIX {ssha1}");
    hashTypeMap.insert(6900,"GOST, GOST R 34.11-94");
    hashTypeMap.insert(7000,"Fortigate (FortiOS)");
    hashTypeMap.insert(7100,"OS X v10.8 / v10.9");
    hashTypeMap.insert(7200,"GRUB 2");
    hashTypeMap.insert(7300,"IPMI2 RAKP HMAC-SHA1");
    hashTypeMap.insert(7400,"sha256crypt, SHA256(Unix)");
    hashTypeMap.insert(7900,"Drupal7");
    hashTypeMap.insert(8400,"WBB3, Woltlab Burning Board 3");
    hashTypeMap.insert(8900,"scrypt");
    hashTypeMap.insert(9200,"Cisco $8$");
    hashTypeMap.insert(9300,"Cisco $9$");
    hashTypeMap.insert(9800,"Radmin2");
    hashTypeMap.insert( 10000,"Django (PBKDF2-SHA256)");
    hashTypeMap.insert( 10200,"Cram MD5");
    hashTypeMap.insert( 10300,"SAP CODVN H (PWDSALTEDHASH) iSSHA-1");
    hashTypeMap.insert( 99999,"Plaintext");
    hashTypeMap.insert(11,"Joomla < 2.5.18");
    hashTypeMap.insert(12,"PostgreSQL");
    hashTypeMap.insert(21,"osCommerce, xt:Commerce");
    hashTypeMap.insert(23,"Skype");
    hashTypeMap.insert(101,"nsldap, SHA-1(Base64), Netscape LDAP SHA");
    hashTypeMap.insert(111,"nsldaps, SSHA-1(Base64), Netscape LDAP SSHA");
    hashTypeMap.insert(112,"Oracle 11g/12c");
    hashTypeMap.insert(121,"SMF > v1.1");
    hashTypeMap.insert(122,"OS X v10.4, v10.5, v10.6");
    hashTypeMap.insert(123,"EPi");
    hashTypeMap.insert(124,"Django (SHA-1)");
    hashTypeMap.insert(131,"MSSQL(2000)");
    hashTypeMap.insert(132,"MSSQL(2005)");
    hashTypeMap.insert(133,"PeopleSoft");
    hashTypeMap.insert(141,"EPiServer 6.x < v4");
    hashTypeMap.insert(1421,"hMailServer");
    hashTypeMap.insert(1441,"EPiServer 6.x > v4");
    hashTypeMap.insert(1711,"SSHA-512(Base64), LDAP {SSHA512}");
    hashTypeMap.insert(1722,"OS X v10.7");
    hashTypeMap.insert(1731,"MSSQL(2012 & 2014)");
    hashTypeMap.insert(2611,"vBulletin < v3.8.5");
    hashTypeMap.insert(2612,"PHPS");
    hashTypeMap.insert(2711,"vBulletin > v3.8.5");
    hashTypeMap.insert(2811,"IPB2+, MyBB1.2+");
    hashTypeMap.insert(3711,"Mediawiki B type");
    hashTypeMap.insert(3721,"WebEdition CMS");
    hashTypeMap.insert(7600,"Redmine Project Management Web App");

    QList<QString> Values = hashTypeMap.values();
    for(int i = 0; i < Values.count(); i++)
    {
        com->addItem(Values[i],hashTypeMap.key(Values[i]));
        if(hashTypeMap.key(Values[i]) == lastcnf->hashtype)
            com->setCurrentIndex(i);
    }
}

MainWindow::~MainWindow()
{
    lastcnf->hashtype = hashTypeMap.key(this->ui->comboBox->currentText());
    lastcnf->outputtype = outputTypeMap.key(this->ui->comboBox_2->currentText());
    lastcnf->hashcatpath = this->ui->txtProg->text();
    lastcnf->lasthashpath = this->ui->txtInputFile->text();
    lastcnf->separator = this->ui->sep->text();
    lastcnf->lastdir = lastdir;
//    qDebug() << lastcnf->hashtype;
//    qDebug() << lastcnf->outputtype;
//    qDebug() << lastcnf->hashcatpath;
    lastcnf->saveconfig();
    delete ui;
}
