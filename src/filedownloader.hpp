#ifndef FILEDOWNLOADER_HPP
#define FILEDOWNLOADER_HPP

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader
    :   public QObject
{
    Q_OBJECT

    public:
        explicit FileDownloader(
            QUrl    a_url,
            QObject *a_parent = 0
        );

        virtual ~FileDownloader();
        QByteArray downloadedData() const;

    signals:
        void    downloaded();

    protected slots:
        void    fileDownloaded(QNetworkReply *a_reply);

    private:
        QNetworkAccessManager   m_web_ctrl;
        QByteArray              m_downloaded_data;
};

#endif

