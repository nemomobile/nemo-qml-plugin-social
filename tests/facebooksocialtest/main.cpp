#include <QtCore/QDir>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QtQml>
# include <QQuickItem>
# include <QQuickView>
# include <QGuiApplication>
# define QDeclarativeView QQuickView
#else
# include <QtDeclarative>
# include <QtDeclarative/QDeclarativeView>
# include <QtGui/QApplication>
#endif
#include <QDebug>

//static const char *IMPORT_PATH = "/opt/sdk/tests/nemo-qml-plugins/social/imports";

int main(int argc, char *argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    QDeclarativeView view;

    if (argc != 2) {
        qWarning() << "usage: facebooktest [facebook_access_token]";
        return 1;
    }

    // TODO: manage better the difference between desktop and device
    view.engine()->addImportPath(PLUGIN_PATH);
    view.setSource(QUrl::fromLocalFile(QLatin1String("share/facebooksocialtest.qml")));

    if (view.status() == QDeclarativeView::Error) {
        qWarning() << "Unable to read main qml file";
        return 1;
    }

    view.rootObject()->setProperty("accessToken", QLatin1String(argv[1]));
    view.rootObject()->setProperty("_desktop", true);
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);

    view.show();

    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    return app.exec();
}

