// vim: ts=4:expandtab

#ifndef LEO_HPP
#define LEO_HPP

#include <QObject>
#include <QVariant>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QUrlQuery>
#include <QJsonObject>
#include <QDebug>

#include "main.hpp"

class Leo
    :   public QObject
{
    Q_OBJECT

    public:
        Leo(QObject *a_parent = 0);
        ~Leo();

        void    setApiUrl(const QString &);
        void    setApiLoginUri(const QString &);
        void    setApiTranslateUri(const QString &);
        void    setApiAddWordUri(const QString &);

        QString getApiUrl()             const;
        QString getApiLoginUri()        const;
        QString getApiTranslateUri()    const;
        QString getApiAddWordUri()      const;

        void    doAuth(
            const QString &a_email,
            const QString &a_password
        );

        void    doTranslate(
            const QString &a_word
        );

        void    doAddWord(
            const QString &,    // en
            const QString &     // ru
        );

	signals:
		void	translateSuccess(const QJsonObject &);
        void    doAddWordSuccess(const QJsonObject &);
		void	translateFailed();
        void    loginSuccess();
        void    loginFailed();
        void    error(const QString &);

    protected slots:
        void    onFinishSlot(QNetworkReply *);

    private:
        QNetworkAccessManager	*m_net_manager;
        QVariant                m_cookie;

        QString     m_api_url;
        QString     m_api_login_uri;
        QString     m_api_translate_uri;
        QString     m_api_add_word_uri;
};

#endif

