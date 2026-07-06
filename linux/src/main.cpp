#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "echoes/greeting.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("echoes");

    QCoreApplication::setOrganizationName(QStringLiteral("sleepyfran"));
    QCoreApplication::setApplicationName(QStringLiteral("echoes"));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("coreGreeting"),
                                             QString::fromStdString(echoes::greeting()));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("me.sleepyfran.echoes"), QStringLiteral("Main"));

    return app.exec();
}
