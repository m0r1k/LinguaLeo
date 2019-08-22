#include "filedownloader.hpp"

FileDownloader::FileDownloader(
    QUrl    a_url,
    QObject *a_parent)
        :   QObject(a_parent)
{
    connect(
        &m_web_ctrl,
        SIGNAL(finished(QNetworkReply*)),
        this,
        SLOT(fileDownloaded(QNetworkReply*))
    );

    QNetworkRequest request(a_url);
    m_web_ctrl.get(request);
}

FileDownloader::~FileDownloader()
{
}

void FileDownloader::fileDownloaded(
    QNetworkReply   *a_reply)
{
    m_downloaded_data = a_reply->readAll();

    //emit a signal
    a_reply->deleteLater();
    emit downloaded();
}

QByteArray FileDownloader::downloadedData() const
{
    return m_downloaded_data;
}

