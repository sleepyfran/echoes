#include <QDesktopServices>
#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QString>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "crypto_provider.h"
#include "entities/provider.h"
#include "providers/provider_factory.h"

#include <memory>
#include <openssl/sha.h>
#include <stop_token>

class LinuxCryptoProvider final : public crypto::CryptoProvider
{
  public:
    std::vector<uint8_t> sha256(std::string_view input) override
    {
        std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
        SHA256(reinterpret_cast<const unsigned char*>(input.data()), input.size(), hash.data());
        return hash;
    }
};

class ProviderBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAuthenticating READ isAuthenticating NOTIFY isAuthenticatingChanged)
    Q_PROPERTY(QString authErrorMessage READ authErrorMessage NOTIFY authErrorMessageChanged)

  public:
    explicit ProviderBridge(QObject* parent = nullptr) : QObject(parent) {}

    ~ProviderBridge() override
    {
        activeAuthStopSource.request_stop();
    }

    bool isAuthenticating() const
    {
        return m_isAuthenticating;
    }
    QString authErrorMessage() const
    {
        return m_authErrorMessage;
    }

    Q_INVOKABLE void startAuthentication()
    {
        if (m_isAuthenticating)
        {
            return;
        }

        setIsAuthenticating(true);
        setAuthErrorMessage({});

        providers::GlobalDependencies deps{
            .crypto_provider = std::make_unique<LinuxCryptoProvider>(),
        };

        activeAuthStopSource = std::stop_source();
        activeAuthProvider =
            providers::create_auth_provider(entities::ProviderId::OneDrive, std::move(deps));

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

        if (!QDesktopServices::openUrl(QUrl(QString::fromStdString(startUrl))))
        {
            activeAuthStopSource.request_stop();
            setIsAuthenticating(false);
            setAuthErrorMessage(i18nc("@info", "Unable to open the authentication URL."));
        }
    }

  Q_SIGNALS:
    void isAuthenticatingChanged();
    void authErrorMessageChanged();

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

    std::unique_ptr<AuthProvider> activeAuthProvider;
    std::stop_source activeAuthStopSource;
    bool m_isAuthenticating = false;
    QString m_authErrorMessage;
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
