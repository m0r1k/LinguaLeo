#ifndef TRAY_WIDGET_HPP
#define TRAY_WIDGET_HPP

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QIcon>
#include <QUrl>
#include <QMediaPlayer>
#include <QBuffer>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QProgressBar>
#include <QVector>
#include <QtAlgorithms>
#include <QTextEdit>
#include <QDebug>

#include "filedownloader.hpp"

namespace Ui {
    class TrayWidget;
}

class TrayWidget
    :   public QDialog
{
    Q_OBJECT

    public:
        explicit TrayWidget(QWidget *a_parent);
        ~TrayWidget();

        QString     getLogin();
        void        setLogin(const QString &);

        QString     getPassword();
        void        setPassword(const QString &);

		void		clearTranslateResults();

        void        setTranslateText(const QString &);
        void        setTranslateTranscription(const QString &);
        void        setTranslateResults(const QJsonObject &);

        int32_t     getTranslateMode()  const;
        int32_t     getPronVolume()     const;

        void        setTranslateMode(int32_t);
        void        setPronVolume(int32_t);

        void        setTranslateSoundUrl(const QUrl &);

        static void setTextEditRowsCount(
            QTextEdit   *a_text_edit,
            int32_t     a_rows
        );


    signals:
        void        doSave();
        void        requestTranslate(const QString &);
        void        requestAddWord(
            const QString &,    // en
            const QString &     // ru
        );

    public slots:
        void        playSoundSlot();

    protected slots:
        void        cellActivatedSlot(int, int);
        void        translateTextSlot(const QString &);
        void        requestAddWordSlot(
            const QString &,    // en
            const QString &     // ru
        );
        void        loginChangedSlot(const QString &);
        void        passwordChangedSlot(const QString &);
        void        saveSlot(bool);
        void        translateModeChangedSlot(int);
        void        pronValueChangedSlot(int);
        void        loadSoundSlot();

    private:
        Ui::TrayWidget  *m_ui;
        FileDownloader  *m_sound_downloader;
		QMediaPlayer	*m_sound_player;
};

#endif

