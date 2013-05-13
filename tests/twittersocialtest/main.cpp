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
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include "twitter/twitterdatautil_p.h"

static const char *REQUEST = "request_token";
static const char *TWITTER_API_REQUEST_TOKEN = "https://api.twitter.com/oauth/request_token";
static const char *TWITTER_API_REQUEST_TOKEN_PARAM_KEY = "oauth_callback";
static const char *TWITTER_API_REQUEST_TOKEN_PARAM_VALUE = "oob";
static const char *TWITTER_API_AUTHORIZE = "https://api.twitter.com/oauth/authorize";
static const char *TWITTER_API_ACCESS_TOKEN = "https://api.twitter.com/oauth/access_token";
static const char *TWITTER_API_ACCESS_TOKEN_PARAM_KEY = "oauth_verifier";
static const char *TWITTER_API_OAUTH_TOKEN_KEY = "oauth_token";
static const char *TWITTER_API_OAUTH_TOKEN_SECRET_KEY = "oauth_token_secret";
static const char *TWITTER_API_OAUTH_CALLBACK_CONFIRMED_KEY = "oauth_callback_confirmed";
static const char *TWITTER_API_OAUTH_CALLBACK_CONFIRMED_VALUE = "true";
static const char *TWITTER_API_OAUTH_USER_ID_KEY = "user_id";
static const char *TWITTER_API_OAUTH_SCREEN_NAME_KEY = "screen_name";

void displayUsage()
{
    qWarning() << "usage: twittersocialtest [consumer_key] [consumer_secret] [token] [token_secret] [user_id]";
    qWarning() << "       twittersocialtest request_token [consumer_key] [consumer_secret]";
}

struct Data
{
    QByteArray appId;
    QByteArray appSecret;
    QByteArray token;
    QByteArray tokenSecret;
};

class TokenRequestHandler: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pin READ pin WRITE setPin NOTIFY pinChanged)
public:
    explicit TokenRequestHandler(Data *data, QObject *parent = 0);
    int pin() const;
    void setPin(int pin);
    void startRequest();
public slots:
    void continueRequest();
signals:
    void pinChanged();
    void sendAuthorize(const QUrl &url);
private:
    Data *m_data;
    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply *m_startRequestReply;
    QNetworkReply *m_continueRequestReply;
    int m_pin;
private slots:
    void startRequestFinishedHandler();
    void continueRequestFinishedHandler();

};

TokenRequestHandler::TokenRequestHandler(Data *data, QObject *parent)
    : QObject(parent), m_data(data), m_networkAccessManager(new QNetworkAccessManager(this))
    , m_startRequestReply(0), m_continueRequestReply(0), m_pin(-1)
{
}

int TokenRequestHandler::pin() const
{
    return m_pin;
}

void TokenRequestHandler::setPin(int pin)
{
    if (m_pin != pin) {
        m_pin = pin;
        emit pinChanged();
    }
}

void TokenRequestHandler::startRequest()
{
    QList<QPair<QByteArray, QByteArray> > args;
    args.append(qMakePair<QByteArray, QByteArray>(TWITTER_API_REQUEST_TOKEN_PARAM_KEY,
                                                  TWITTER_API_REQUEST_TOKEN_PARAM_VALUE));
    QByteArray header = TwitterDataUtil::authorizationHeader(m_data->appId, m_data->appSecret,
                                                             "POST",
                                                             TWITTER_API_REQUEST_TOKEN,
                                                             args);
    qDebug() << "The authentification header for the start request is:" << header;

    QNetworkRequest request (QUrl(QByteArray(TWITTER_API_REQUEST_TOKEN) + "?"
                                             + TWITTER_API_REQUEST_TOKEN_PARAM_KEY + "="
                                             + TWITTER_API_REQUEST_TOKEN_PARAM_VALUE));
    request.setRawHeader("Authorization", header);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QByteArray("application/x-www-form-urlencoded"));

    m_startRequestReply = m_networkAccessManager->post(request, QByteArray());
    connect(m_startRequestReply, SIGNAL(finished()), this, SLOT(startRequestFinishedHandler()));
}

void TokenRequestHandler::continueRequest()
{
    if (m_data->token.isEmpty() || m_pin == -1) {
        return;
    }

    qDebug() << "Continuing request with pin:" << m_pin;

    QList<QPair<QByteArray, QByteArray> > args;
    args.append(qMakePair<QByteArray, QByteArray>(TWITTER_API_ACCESS_TOKEN_PARAM_KEY,
                                                  QByteArray::number(m_pin)));
    QByteArray header = TwitterDataUtil::authorizationHeader(m_data->appId, m_data->appSecret,
                                                             "POST",
                                                             TWITTER_API_ACCESS_TOKEN,
                                                             args,
                                                             m_data->token, m_data->tokenSecret);
    qDebug() << "The authentification header for the continue request is:" << header;

    QNetworkRequest request (QUrl(QByteArray(TWITTER_API_ACCESS_TOKEN) + "?"
                                             + TWITTER_API_ACCESS_TOKEN_PARAM_KEY + "="
                                             + QByteArray::number(m_pin)));
    request.setRawHeader("Authorization", header);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QByteArray("application/x-www-form-urlencoded"));

    m_continueRequestReply = m_networkAccessManager->post(request, QByteArray());
    connect(m_continueRequestReply, SIGNAL(finished()),
            this, SLOT(continueRequestFinishedHandler()));
}

void TokenRequestHandler::startRequestFinishedHandler()
{
    if (m_startRequestReply->error() != QNetworkReply::NoError) {
        qWarning() << "Error happened. Code:" << m_startRequestReply->error();
        qWarning() << "Error message (Qt):" << m_startRequestReply->errorString();
        qWarning() << "Error message (Twitter):" << m_startRequestReply->readAll();
        return;
    }

    QList<QByteArray> data = m_startRequestReply->readAll().split('&');
    QMap<QByteArray, QByteArray> dataMap;
    foreach (QByteArray dataEntry, data) {
        QList<QByteArray> pair = dataEntry.split('=');
        dataMap.insert(pair.first(), pair.last());
    }

    if (dataMap.value(TWITTER_API_OAUTH_CALLBACK_CONFIRMED_KEY)
        != QByteArray(TWITTER_API_OAUTH_CALLBACK_CONFIRMED_VALUE)) {
        qWarning() << "The callback is not confirmed";
        return;
    }

    m_data->token = dataMap.value(TWITTER_API_OAUTH_TOKEN_KEY);
    m_data->tokenSecret = dataMap.value(TWITTER_API_OAUTH_TOKEN_SECRET_KEY);

    // Now we need to continue the PIN based authentification
    QByteArray url = TWITTER_API_AUTHORIZE;
    emit sendAuthorize(QUrl(url + "?" + TWITTER_API_OAUTH_TOKEN_KEY + "=" + m_data->token));
}

void TokenRequestHandler::continueRequestFinishedHandler()
{
    if (m_continueRequestReply->error() != QNetworkReply::NoError) {
        qWarning() << "Error happened. Code:" << m_continueRequestReply->error();
        qWarning() << "Error message (Qt):" << m_continueRequestReply->errorString();
        qWarning() << "Error message (Twitter):" << m_continueRequestReply->readAll();
        return;
    }

    QList<QByteArray> data = m_continueRequestReply->readAll().split('&');
    QMap<QByteArray, QByteArray> dataMap;
    foreach (QByteArray dataEntry, data) {
        QList<QByteArray> pair = dataEntry.split('=');
        dataMap.insert(pair.first(), pair.last());
    }

    qDebug() << "--------------------------";
    qDebug() << "Login successful";
    qDebug() << "Token:" << dataMap.value(TWITTER_API_OAUTH_TOKEN_KEY);
    qDebug() << "Token secret:" << dataMap.value(TWITTER_API_OAUTH_TOKEN_SECRET_KEY);
    qDebug() << "User id:" << dataMap.value(TWITTER_API_OAUTH_USER_ID_KEY);
    qDebug() << "Screen name:" << dataMap.value(TWITTER_API_OAUTH_SCREEN_NAME_KEY);
    QCoreApplication::quit();
}


int main(int argc, char *argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    QDeclarativeView view;

    QStringList arguments = app.arguments();
    if (arguments.count() != 4 && arguments.count() != 6) {
        displayUsage();
        return 1;
    }

    if (arguments.count() == 4 && arguments.at(1) != QLatin1String(REQUEST)) {
        displayUsage();
        return 1;
    }

    Data data;
    bool request = (arguments.count() == 4 && arguments.at(1) == QLatin1String(REQUEST));
    if (request) {
        data.appId = arguments.at(2).toLatin1();
        data.appSecret = arguments.at(3).toLatin1();

        qDebug() << "Performing token request using PIN";
        qDebug() << "Application id:" << data.appId;
        qDebug() << "Application secret:" << data.appSecret;

        TokenRequestHandler *tokenRequestHandler = new TokenRequestHandler(&data);
        view.rootContext()->setContextProperty("tokenRequestHandler", tokenRequestHandler);
        view.setSource(QUrl::fromLocalFile(QLatin1String("share/twittersocialtesttokenrequest.qml")));

        tokenRequestHandler->startRequest();
    } else {
        view.engine()->addImportPath(PLUGIN_PATH);
        qDebug() << "Consumer key:" << arguments.at(1) << ", consumer secret:" << arguments.at(2);
        qDebug() << "Token:" << arguments.at(3) << ", token secret:" << arguments.at(4);
        qDebug() << "Identifier:" << arguments.at(5);
        view.setSource(QUrl::fromLocalFile(QLatin1String("share/twittersocialtest.qml")));
        view.rootObject()->setProperty("consumerKey", arguments.at(1));
        view.rootObject()->setProperty("consumerSecret", arguments.at(2));
        view.rootObject()->setProperty("token", arguments.at(3));
        view.rootObject()->setProperty("tokenSecret", arguments.at(4));
        view.rootObject()->setProperty("identifier", arguments.at(5));
    }
    if (view.status() == QDeclarativeView::Error) {
        qWarning() << "Unable to read main qml file";
        return 1;
    }
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view.show();
    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    return app.exec();
}

#include "main.moc"
