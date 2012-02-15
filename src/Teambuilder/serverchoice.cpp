#include "serverchoice.h"
#include "../Utilities/functions.h"
#include "analyze.h"
#include "poketextedit.h"

ServerChoice::ServerChoice(const QString &nick)
{
    QSettings settings;

    registry_connection = new Analyzer(true);
    registry_connection->connectTo(
            settings.value("registry_server", "pokemon-online.dynalias.net").toString(),
            settings.value("registry_port", 8080).toUInt()
    );
    registry_connection->setParent(this);

    connect(registry_connection, SIGNAL(connectionError(int,QString)), SLOT(connectionError(int , QString)));
    connect(registry_connection, SIGNAL(regAnnouncementReceived(QString)), SLOT(setRegistryAnnouncement(QString)));

    connect(registry_connection, SIGNAL(serverReceived(QString, QString, quint16,QString,quint16,quint16, bool)), SLOT(addServer(QString, QString, quint16, QString,quint16,quint16, bool)));

    // Someone make this a little better, though i suck at UI design :( - Latios / Forgive
    QVBoxLayout *l = new QVBoxLayout(this);

    announcement = new PokeTextEdit();
    announcement->setVisible(false);
    announcement->setOpenLinks(false);

    connect(announcement, SIGNAL(anchorClicked(QUrl)), SLOT(openURL(QUrl)));

    l->addWidget(announcement);

    mylist = new QCompactTable(0,4);

    QStringList horHeaders;
    horHeaders << tr("") << tr("Server Name") << tr("Players / Max") << tr("Advanced connection");

    mylist->setHorizontalHeaderLabels(horHeaders);
    mylist->setSelectionBehavior(QAbstractItemView::SelectRows);
    mylist->setSelectionMode(QAbstractItemView::SingleSelection);
    mylist->setShowGrid(false);
    mylist->verticalHeader()->hide();
    mylist->horizontalHeader()->resizeSection(0, 150);
    mylist->horizontalHeader()->setStretchLastSection(true);
    mylist->setMinimumHeight(200);

    //TO-DO: Make  the item 0 un-resizable and unselectable - Latios

    mylist->setColumnWidth(0, settings.value("ServerChoice/PasswordProtectedWidth", 26).toInt());
    mylist->setColumnWidth(1, settings.value("ServerChoice/ServerNameWidth", 152).toInt());
    if (settings.contains("ServerChoice/PlayersInfoWidth")) {
        mylist->setColumnWidth(2, settings.value("ServerChoice/PlayersInfoWidth").toInt());
    }
    if (settings.contains("ServerChoice/ServerIPWidth")) {
        mylist->setColumnWidth(3, settings.value("ServerChoice/ServerIPWidth").toInt());
    }
    mylist->horizontalHeaderItem(0)->setIcon(QIcon("db/mixed-lock.png"));
    mylist->horizontalHeaderItem(0)->setToolTip(tr("This is to check if the server is password protected"));

    connect(mylist, SIGNAL(cellActivated(int,int)), SLOT(regServerChosen(int)));
    connect(mylist, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(showDetails(int)));

    l->addWidget(mylist, 100);

    myDesc = new PokeTextEdit();
    myDesc->setOpenExternalLinks(true);
    myDesc->setFixedHeight(100);

    myDesc->setOpenLinks(false);
    l->addWidget(new QEntitled(tr("Server Description"), myDesc));
    connect(myDesc, SIGNAL(anchorClicked(QUrl)), SLOT(openURL(QUrl)));

    myName = new QLineEdit(nick);
    l->addWidget(new QEntitled(tr("Trainer Name"), myName));

    myAdvServer = new QLineEdit(settings.value("default_server").toString());
    connect(myAdvServer, SIGNAL(returnPressed()), SLOT(advServerChosen()));

    l->addWidget(new QEntitled(tr("&Advanced Connection"), myAdvServer));

    QHBoxLayout *hl= new QHBoxLayout();
    l->addLayout(hl);

    QPushButton *cancel = new QPushButton(tr("&Go Back"));
    QPushButton *ok = new QPushButton(tr("Advanced &Connection"));
    QPushButton *localhost = new QPushButton(tr("Connect to own server"));

    connect(cancel, SIGNAL(clicked()), SIGNAL(rejected()));
    connect(ok, SIGNAL(clicked()), SLOT(advServerChosen()));
    connect(localhost, SIGNAL(clicked()), SLOT(connectToLocalhost()));

    hl->addWidget(cancel);
    hl->addWidget(ok);
    hl->addWidget(localhost);
}

ServerChoice::~ServerChoice()
{
    writeSettings(this);
    saveSettings();
}

void ServerChoice::regServerChosen(int row)
{
    QString ip = mylist->item(row, 3)->text();

    QSettings settings;
    settings.setValue("default_server", ip);
    if(ip.contains(":")){
        quint16 port = ip.section(":",1,1).toInt(); //Gets port from IP:PORT
        QString fIp = ip.section(":",0,0);  //Gets IP from IP:PORT
        emit serverChosen(fIp,port, myName->text());
    }
    else
        emit serverChosen(ip,5080, myName->text());
}

void ServerChoice::advServerChosen()
{
    QString ip = myAdvServer->text().trimmed();

    QSettings settings;
    settings.setValue("default_server", ip);
    if(ip.contains(":")){
        quint16 port = ip.section(":",1,1).toInt(); //Gets port from IP:PORT
        QString fIp = ip.section(":",0,0);  //Gets IP from IP:PORT
        emit serverChosen(fIp,port, myName->text());
    }
    else
        emit serverChosen(ip,5080, myName->text());

}
void ServerChoice::connectToLocalhost()
{
    emit serverChosen("localhost", 5080, myName->text());
}

void ServerChoice::setRegistryAnnouncement(const QString &sannouncement) {
    announcement->insertHtml(sannouncement);
    announcement->show();
}

void ServerChoice::addServer(const QString &name, const QString &desc, quint16 num, const QString &ip, quint16 max, quint16 port, bool passwordProtected)
{
    mylist->setSortingEnabled(false);

    QString playerStr;
    if(max == 0)
        playerStr = QString::number(num).rightJustified(3);
    else
        playerStr = QString::number(num).rightJustified(3) + " / " + QString::number(max);
    int row = mylist->rowCount();
    mylist->setRowCount(row+1);

    QTableWidgetItem *witem;

    witem = new QTableWidgetItem();
    if(passwordProtected) witem->setIcon(QIcon("db/locked.png"));
    else witem->setIcon(QIcon("db/unlocked.png"));

    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 0, witem);

    witem = new QTableWidgetItem(name);
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 1, witem);

    witem = new QTableWidgetItem(playerStr);
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 2, witem);

    witem = new QTableWidgetItem(ip + ":" + QString::number(port == 0 ? 5080 : port));
    witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
    mylist->setItem(row, 3, witem);

    descriptionsPerIp.insert(ip + ":" + QString::number(port == 0 ? 5080 : port), desc);
    /*This needed to be changed because the showDescription function was looking for a ip and port,
      while only the IP was in the list, and in the end, the description wouldn't be displayed. */

    mylist->setSortingEnabled(true);
    mylist->sortByColumn(2);

    if (mylist->currentRow() != -1)
        showDetails(mylist->currentRow());
}

void ServerChoice::showDetails(int row)
{
    if (row < 0)
        return;
    myDesc->clear();
    myDesc->insertHtml(descriptionsPerIp[mylist->item(row,3)->text()]);

    QString ip = mylist->item(row, 3)->text();
    myAdvServer->setText(ip);

}

void ServerChoice::connectionError(int, const QString &mess)
{
    mylist->setCurrentCell(-1,-1);
    myDesc->clear();
    myDesc->insertPlainText(tr("Disconnected from the registry: %1").arg(mess));
}

void ServerChoice::saveSettings() {
    QSettings settings;
    settings.setValue("ServerChoice/PasswordProtectedWidth", mylist->columnWidth(0));
    settings.setValue("ServerChoice/ServerNameWidth", mylist->columnWidth(1));
    settings.setValue("ServerChoice/PlayersInfoWidth", mylist->columnWidth(2));
    settings.setValue("ServerChoice/ServerIPWidth", mylist->columnWidth(3));
}

void ServerChoice::openURL(QUrl url) {
    QDesktopServices::openUrl(url);
}
