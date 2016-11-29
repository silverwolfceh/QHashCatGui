#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>
class config
{
private:
    QSettings *mycnf;
public:
    config();
    void loadconfig();
    void saveconfig();
    int hashtype;
    int outputtype;
    QString hashcatpath;
    QString separator;
    QString lasthashpath;
    QStringList wordlists;
    QString lastdir;
};

#endif // CONFIG_H
