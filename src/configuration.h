#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include <QHostAddress>
#include <QTextStream>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QJsonValue>
#include <string>
#include <array>
#include <vector>
#include <cpptoml.h>
#include "semester.h"
#include "plan.h"
#include "plancsvhelper.h"

#ifndef DEFAULT_CONFIG_PATH
#define DEFAULT_CONFIG_PATH "/etc/pruefungsplaner-backend"
#endif
#ifndef DEFAULT_KEYS_PATH
#define DEFAULT_KEYS_PATH "/usr/share/pruefungsplaner-backend/keys"
#endif
#ifndef DEFAULT_STORAGE_PATH
#define DEFAULT_STORAGE_PATH "/usr/share/pruefungsplaner-backend/data"
#endif

class Configuration : public QObject
{
    Q_OBJECT
private:
    static constexpr std::array defaultConfigurationFiles{DEFAULT_CONFIG_PATH "/config.toml", "/dev/null"};
    static constexpr auto defaultAddress = "0.0.0.0";
    static constexpr uint32_t defaultPort = 80;
    static constexpr auto defaultPublicKey = DEFAULT_KEYS_PATH "/public_key.pem";
    static constexpr auto defaultIssuer = "pruefungsplaner-auth";
    static constexpr auto defaultAuthUrl = "ws://pruefungsplaner-auth:80";
    static constexpr std::array defaultRequiredClaims{"pruefungsplanerStartSchedule","pruefungsplanerRetrieveSchedule"};
    static constexpr auto defaultStoragePath = DEFAULT_STORAGE_PATH;
    static constexpr auto defaultRetrieveSettings = false;
    static constexpr auto defaultCheckSettings = false;
    static constexpr int defaultJobLifetime = 86400;
    static constexpr auto defaultDefaultScheduler = "legacy-fast";
    static constexpr auto defaultLegacySchedulerAlgorithmBinary = "/usr/bin/SPA-algorithmus";
    static constexpr auto defaultLegacySchedulerPrintLog = false;
    QString address;
    quint16 port;
    QString publicKey;
    QString issuer;
    QList<QString> requiredClaims;
    QScopedPointer<QDir> storagePath;
    QScopedPointer<int> jobLifetime;
    QString defaultSchedulingAlgorithm;
    QString legacySchedulerAlgorithmBinary;
    QScopedPointer<bool> legacySchedulerPrintLog;

    //These are only used internally
    QString authUrl;
    QScopedPointer<bool> check;
    QScopedPointer<bool> retrieve;

public:
    explicit Configuration(const QList<QString>& args, QObject *parent = nullptr);
    QString getAddress() const;
    quint16 getPort() const;
    QString getPublicKey() const;
    QString getIssuer() const;
    QList<QString> getClaims() const;
    QDir getStoragePath() const;
    int getJobLifetime() const;
    QString getDefaultSchedulingAlgorithm() const;
    QString getLegacySchedulerAlgorithmBinary() const;
    bool getLegacySchedulerPrintLog() const;

private:
    void loadConfiguration(const QFile& configuration);
    void readPublicKey(QFile& publicKeyFile);
    void checkPublicKey(const QString& publicKey);
    void retrieveSettings(const QString& authServerUrl);
    void checkKeyAndIssuer(const QString& authServerUrl);
    void loadStoragePath(const QString& storagePath);
    void checkConfiguration();
    [[ noreturn ]] void failConfiguration(const QString& message) const;
    void warnConfiguration(const QString& message) const;

signals:

};

#endif // CONFIGURATION_H
