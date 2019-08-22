// vim: ts=4:expandtab

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "leo.hpp"

Leo::Leo(QObject *a_parent)
    :   QObject(a_parent)
{
    m_net_manager = new QNetworkAccessManager(this);

    connect(
        m_net_manager,
        SIGNAL(finished(QNetworkReply*)),
        this,
        SLOT(onFinishSlot(QNetworkReply*))
    );
}

Leo::~Leo()
{
}

QString Leo::getApiUrl() const
{
    return m_api_url;
}

QString Leo::getApiLoginUri() const
{
    return m_api_login_uri;
}

QString Leo::getApiTranslateUri() const
{
    return m_api_translate_uri;
}

QString Leo::getApiAddWordUri() const
{
    return m_api_add_word_uri;
}

void Leo::setApiUrl(
    const QString &a_val)
{
    m_api_url = a_val;
}

void Leo::setApiLoginUri(
    const QString &a_val)
{
    m_api_login_uri = a_val;
}

void Leo::setApiTranslateUri(
    const QString &a_val)
{
    m_api_translate_uri = a_val;
}

void Leo::setApiAddWordUri(
    const QString &a_val)
{
    m_api_add_word_uri = a_val;
}

void Leo::doAuth(
    const QString   &a_email,
    const QString   &a_password)
{
    QUrlQuery           post_data;
    QByteArray          post_data_ba;
    QUrl                url;
    QString             api_url         = getApiUrl();
    QString             api_login_uri   = getApiLoginUri();

    if (!api_url.size()){
        QString error_str = tr("empty API URL");
        emit error(error_str);
        goto fail;
    }

    if (!api_login_uri.size()){
        QString error_str = tr("empty API login URI");
        emit error(error_str);
        goto fail;
    }

    url = QUrl(api_url + api_login_uri);

    post_data.addQueryItem("email",     a_email);
    post_data.addQueryItem("password",  a_password);
    post_data.addQueryItem("source",    "landing_modal");

    qInfo() << "POST URL: "  << url;
    qInfo() << "POST DATA: " << post_data.toString();

    post_data_ba = post_data.toString().toUtf8();

    {
        QNetworkRequest     req(url);

        // content type
        req.setHeader(
            QNetworkRequest::ContentTypeHeader,
            "application/x-www-form-urlencoded"
        );

        // content length
        req.setHeader(
            QNetworkRequest::ContentLengthHeader,
            post_data_ba.size()
        );

        m_net_manager->post(req, post_data_ba);
    }

out:
    return;
fail:
    goto out;
}

void Leo::doTranslate(
    const QString    &a_word)
{
    QUrl        url;
    QString     api_url             = getApiUrl();
    QString     api_translate_uri   = getApiTranslateUri();
    QUrlQuery   query;

    if (!api_url.size()){
        QString error_str = tr("empty API URL");
        emit error(error_str);
        goto fail;
    }

    if (!api_translate_uri.size()){
        QString error_str = tr("empty API translate URI");
        emit error(error_str);
        goto fail;
    }

    url = QUrl(api_url + api_translate_uri);

    query.addQueryItem("word", a_word);
    url.setQuery(query);

    {
        QNetworkRequest req(url);
        qInfo() << "GET URL: " << url.toString();

        // cookie
        req.setHeader(
            QNetworkRequest::SetCookieHeader,
            m_cookie
        );

        m_net_manager->get(req);
    }

out:
    return;
fail:
    goto out;
}

void Leo::doAddWord(
    const QString &a_en_word,                       // en
    const QString &a_ru_word __attribute((unused))) // ru
{
    QUrl        url;
    QString     api_url             = getApiUrl();
    QString     api_add_word_uri    = getApiAddWordUri();
    QUrlQuery   query;

    if (!api_url.size()){
        QString error_str = tr("empty API URL");
        emit error(error_str);
        goto fail;
    }

    if (!api_add_word_uri.size()){
        QString error_str = tr("empty API add word URI");
        emit error(error_str);
        goto fail;
    }

    url = QUrl(api_url + api_add_word_uri);

    query.addQueryItem("word",  a_en_word);
    // TODO query.addQueryItem("tword", a_ru_word);

    url.setQuery(query);

    {
        QNetworkRequest req(url);
        qInfo() << "GET URL: " << url.toString();

        // cookie
        req.setHeader(
            QNetworkRequest::SetCookieHeader,
            m_cookie
        );

        m_net_manager->get(req);
    }

out:
    return;
fail:
    goto out;
}

void Leo::onFinishSlot(
    QNetworkReply *a_reply)
{
    qInfo() << "MORIK end of request: " << a_reply->url();

    int32_t     err     = a_reply->error();
    QByteArray  content = a_reply->readAll();
    QString     data    = QString::fromUtf8(content.data());
    QUrl        url     = a_reply->url();
    QString     path    = url.path();

    qInfo() << "MORIK path: " << path;

    // headers
    if (1){
        QList<QByteArray> headerList = a_reply->rawHeaderList();
        foreach(QByteArray head, headerList){
            qInfo() << head << ":" << a_reply->rawHeader(head);
        }
    }

    // content
    if (1){
        qInfo() << data;
    }

    if (getApiLoginUri() == path){
        if (QNetworkReply::NoError != err){
            qInfo() << "network error: "    << err;
            qInfo() << "content: "          << data;
            QString error_str = tr("auth failed");
            emit loginFailed();
            emit error(error_str);
            goto fail;
        }

        QJsonDocument   json_response   = QJsonDocument::fromJson(content);
        QJsonObject     json_object     = json_response.object();
        QString         error_msg       = json_object["error_msg"].toString();

        if (error_msg.size()){
            QString error_str = tr("the answer from Leo: '%1'")
                .arg(error_msg);
            emit loginFailed();
            emit error(error_str);
            goto fail;
        }

        m_cookie = a_reply->header(QNetworkRequest::SetCookieHeader);

        emit loginSuccess();
    } else if (getApiTranslateUri() == path){
        if (QNetworkReply::NoError != err){
            qInfo() << "network error: "    << err;
            qInfo() << "content: "          << data;
            QString error_str = tr("translate failed");
            emit translateFailed();
            emit error(error_str);
            goto fail;
        }

        QJsonDocument   json_response   = QJsonDocument::fromJson(content);
        QJsonObject     json_object     = json_response.object();
        QString         error_msg       = json_object["error_msg"].toString();

        if (error_msg.size()){
            QString error_str = tr("the answer from Leo: '%1'")
                .arg(error_msg);

            qInfo() << "MORIK error_str: " << error_str;

            emit translateFailed();
            emit error(error_str);
            goto fail;
        }

        emit translateSuccess(json_object);
    } else if (getApiAddWordUri() == path){
        if (QNetworkReply::NoError != err){
            qInfo() << "network error: "    << err;
            qInfo() << "content: "          << data;
            QString error_str = tr("do add word failed");
            emit translateFailed();
            emit error(error_str);
            goto fail;
        }

        QJsonDocument   json_response   = QJsonDocument::fromJson(content);
        QJsonObject     json_object     = json_response.object();
        QString         error_msg       = json_object["error_msg"].toString();

        if (error_msg.size()){
            QString error_str = tr("the answer from Leo: '%1'")
                .arg(error_msg);

            qInfo() << "MORIK error_str: " << error_str;

            emit translateFailed();
            emit error(error_str);
            goto fail;
        }

        emit doAddWordSuccess(json_object);
    } else {
        QString error_str = tr("unsupported API answer");
        emit error(error_str);
        goto fail;
    }

    // all ok

out:
    a_reply->deleteLater();
    return;
fail:
    goto out;
}

