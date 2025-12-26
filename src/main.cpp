#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QQmlContext>
#include <QDebug>
#include "backend.h"
#include "platform.h"

//font loading function
static QString registerAppFont(const QString& qrcPath)
{
    const int id = QFontDatabase::addApplicationFont(qrcPath);
    if (id < 0) {
        qWarning() << "[FONT] Failed to load:" << qrcPath;
        return {};
    }

    const QStringList families = QFontDatabase::applicationFontFamilies(id);
    qDebug() << "[FONT] Loaded" << qrcPath << "families =" << families;

    return families.isEmpty() ? QString{} : families.first();
}

//main function
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Basic");//designate theme

    //register Backend as usable type
    qmlRegisterType<Backend>("CppComm", 1, 0, "Backend");

    //not required, but precaution
    qmlRegisterUncreatableType<ExifModel>("CppComm", 1, 0, "ExifModel", "C++ only");
    qmlRegisterUncreatableType<FileListModel>("CppComm", 1, 0, "FileListModel", "C++ only");
    qmlRegisterUncreatableType<ExifGroupsModel>("CppComm", 1, 0, "ExifGroupsModel", "C++ only");
    qmlRegisterUncreatableType<EntryListModel>("CppComm", 1, 0, "EntryListModel", "C++ only");

    //register fonts
    const QString LatinFamily =
        registerAppFont(":/fonts/resources/Roboto-VariableFont_wdth,wght.ttf");
    const QString cjkFamily =
        registerAppFont(":/fonts/resources/SourceHanSansSC-Normal-2.otf");
    const QString SegoeFamily =
        registerAppFont(":/fonts/resources/Segoe UI.ttf");
    if (!LatinFamily.isEmpty()) {
        QFont f;
        if (!cjkFamily.isEmpty())
            f.setFamilies({ LatinFamily, cjkFamily });
        else
            f.setFamily(LatinFamily);

        f.setPointSize(10);
        app.setFont(f);

        qDebug() << "[FONT] App font families set to:" << f.families();
    }

    QQmlApplicationEngine engine;

    //pass UI scaling factor to QML
    engine.rootContext()->setContextProperty("FontScale", UiScale::FontScale);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ZViewerCMake1", "Main");

    return app.exec();
}
