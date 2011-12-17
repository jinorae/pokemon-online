#include <QtDeclarative/QDeclarativeView>
#include <QDeclarativeError>

#include "spectatorwindow.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battledata.h"
#include "../BattleManager/battleinput.h"
#include "../BattleManager/battlescene.h"
#include "../BattleManager/regularbattlescene.h"
#include "../BattleManager/battledatatypes.h"
#include "poketextedit.h"
#include "theme.h"

SpectatorWindow::SpectatorWindow(const BattleConfiguration &conf, QString name1, QString name2)
{
    data = new battledata_basic(&conf);
    data2 = new advbattledata_proxy(&conf);

    data->team(0).name() = data2->team(0).name() = name1;
    data->team(1).name() = data2->team(1).name() = name2;

    log = new BattleClientLog(data, Theme::getBattleTheme());
    input = new BattleInput(&conf);

    logWidget = new PokeTextEdit();
    connect(log, SIGNAL(lineToBePrinted(QString)), logWidget, SLOT(insertHtml(QString)));

    QSettings s;

    bool qml = !(s.value("old_battle_window", false).toBool() || conf.mode != ChallengeInfo::Singles);

    if (qml) {
        BattleScene *scene = new BattleScene(data2);

        input->addOutput(scene);
        scene->addOutput(data);
        scene->addOutput(log);
        scene->addOutput(data2);


        connect(scene, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));

        scene->launch();

        foreach(QDeclarativeError error, scene->getWidget()->errors()) {
            log->printLine("Error", error.toString());
        }

        battleView = scene->getWidget();
    } else {
        RegularBattleScene *battle = new RegularBattleScene(data2, Theme::getBattleTheme());

        input->addOutput(data);
        input->addOutput(log);
        input->addOutput(data2);
        input->addOutput(battle);

        connect(battle, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));

        battleView = battle;
    }
}

void SpectatorWindow::reloadTeam(int player)
{
    data->reloadTeam(player);
    data2->reloadTeam(player);
}

PokeTextEdit *SpectatorWindow::getLogWidget()
{
    return logWidget;
}

QWidget *SpectatorWindow::getSceneWidget()
{
    return battleView;
}

QWidget *SpectatorWindow::getSampleWidget()
{
    QWidget *ret = new QWidget();

    QHBoxLayout *layout = new QHBoxLayout(ret);
    layout->addWidget(getSceneWidget());
    layout->addWidget(getLogWidget());

    return ret;
}

SpectatorWindow::~SpectatorWindow()
{
    input->deleteTree();
    delete input;
}

void SpectatorWindow::receiveData(const QByteArray &data)
{
    input->receiveData(data);
}
