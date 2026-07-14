#include <QClipboard>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QString>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "auth_store.h"
#include "entities/provider.h"
#include "providers/provider_factory.h"
#include "wallet_auth_store.h"

#include <memory>
#include <qguiapplication.h>
#include <qwindowdefs.h>
#include <stop_token>

class ProviderBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAuthenticating READ isAuthenticating NOTIFY isAuthenticatingChanged)
    Q_PROPERTY(QString authErrorMessage READ authErrorMessage NOTIFY authErrorMessageChanged)
    Q_PROPERTY(QString authStartUrl READ authStartUrl NOTIFY authStartUrlChanged)

    std::unique_ptr<AuthStore> auth_store = std::make_unique<wallet::WalletAuthStore>();

  public:
    explicit ProviderBridge(QObject* parent = nullptr) : QObject(parent) {}

    ~ProviderBridge() override
    {
        activeAuthStopSource.request_stop();
    }

    [[nodiscard]] bool isAuthenticating() const
    {
        return m_isAuthenticating;
    }
    [[nodiscard]] QString authErrorMessage() const
    {
        return m_authErrorMessage;
    }
    [[nodiscard]] QString authStartUrl() const
    {
        return m_authStartUrl;
    }

    Q_INVOKABLE void startAuthentication()
    {
        if (m_isAuthenticating)
        {
            return;
        }

        setIsAuthenticating(true);
        setAuthErrorMessage({});
        setAuthStartUrl({});

        providers::GlobalDependencies deps{
            .auth_store = auth_store.get(),
        };

        activeAuthStopSource = std::stop_source();
        activeAuthProvider = providers::create_auth_provider(entities::ProviderId::OneDrive, deps);

        auto startUrl = activeAuthProvider->connect(
            activeAuthStopSource.get_token(),
            [this](AuthConnectResult result)
            {
                const auto success = result.status == AuthConnectStatus::Ok;
                auto errorMessage = result.error_msg ? QString::fromStdString(*result.error_msg)
                                                     : i18nc("@info", "Unknown error");

                QMetaObject::invokeMethod(
                    this,
                    [this, success, errorMessage]()
                    {
                        activeAuthStopSource.request_stop();
                        setIsAuthenticating(false);
                        setAuthErrorMessage(success ? QString() : errorMessage);
                    },
                    Qt::QueuedConnection);
            });

        setAuthStartUrl(QString::fromStdString(startUrl));

        if (!QDesktopServices::openUrl(QUrl(QString::fromStdString(startUrl))))
        {
            activeAuthStopSource.request_stop();
            setIsAuthenticating(false);
            setAuthErrorMessage(i18nc("@info", "Unable to open the authentication URL."));
        }
    }

    Q_INVOKABLE void copyAuthenticationUrl() const
    {
        QGuiApplication::clipboard()->setText(m_authStartUrl);
    }

  Q_SIGNALS:
    void isAuthenticatingChanged();
    void authErrorMessageChanged();
    void authStartUrlChanged();

  private:
    void setIsAuthenticating(bool isAuthenticating)
    {
        if (m_isAuthenticating == isAuthenticating)
        {
            return;
        }

        m_isAuthenticating = isAuthenticating;
        Q_EMIT isAuthenticatingChanged();
    }

    void setAuthErrorMessage(const QString& authErrorMessage)
    {
        if (m_authErrorMessage == authErrorMessage)
        {
            return;
        }

        m_authErrorMessage = authErrorMessage;
        Q_EMIT authErrorMessageChanged();
    }

    void setAuthStartUrl(const QString& authStartUrl)
    {
        if (m_authStartUrl == authStartUrl)
        {
            return;
        }

        m_authStartUrl = authStartUrl;
        Q_EMIT authStartUrlChanged();
    }

    std::unique_ptr<AuthProvider> activeAuthProvider;
    std::stop_source activeAuthStopSource;
    bool m_isAuthenticating = false;
    QString m_authErrorMessage;
    QString m_authStartUrl;
};

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("echoes");

    QCoreApplication::setOrganizationName(QStringLiteral("sleepyfran"));
    QCoreApplication::setApplicationName(QStringLiteral("echoes"));

    QQmlApplicationEngine engine;
    ProviderBridge providerBridge;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.rootContext()->setContextProperty(QStringLiteral("authBridge"), &providerBridge);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("me.sleepyfran.echoes"), QStringLiteral("Main"));

    return app.exec();
}

#include "main.moc"
