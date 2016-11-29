#include "config.h"
#include "QFile"
#include "QTextStream"
config::config()
{
    mycnf = new QSettings(QSettings::IniFormat, QSettings::UserScope, "silverwolf", "QHashKillerGui");
    //QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, "/etc/xdg");
    loadconfig();
}

void config::loadconfig()
{
    hashtype = mycnf->value("hashtype").toInt();
    outputtype = mycnf->value("outputtype").toInt();
    hashcatpath = mycnf->value("hashcatpath").toString();
    separator = mycnf->value("separator").toString();
    lasthashpath = mycnf->value("lasthashpath").toString();
    wordlists = mycnf->value("wordlists").toStringList();
    lastdir = mycnf->value("lastdir").toString();
}

void config::saveconfig()
{
    mycnf->setValue("hashtype", hashtype);
    mycnf->setValue("outputtype", outputtype);
    mycnf->setValue("hashcatpath", hashcatpath);
    mycnf->setValue("separator", separator);
    mycnf->setValue("lasthashpath", lasthashpath);
    mycnf->setValue("wordlists", wordlists);
    mycnf->setValue("lastdir", lastdir);
    mycnf->sync();
}
