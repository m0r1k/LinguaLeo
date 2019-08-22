// vim: ts=4:expandtab

#ifndef MAIN_WIDGET_HPP
#define MAIN_WIDGET_HPP

#include <QString>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QSettings>
#include <QJsonArray>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <QLocalServer>
#include <QDebug>

#include "leo.hpp"
#include "trayWidget.hpp"

class MainWidget
    :   public QDialog
{
    Q_OBJECT

    public:
        MainWidget();

        void    setVisible(bool visible) override;

        void    showTrayWidget() const;

        void    setCanTranslate(bool);
        void    loadSettings();
        void    saveSettings();

        QString getApiUrl()             const;
        QString getApiLoginUri()        const;
        QString getApiTranslateUri()    const;
        QString getApiAddWordUri()      const;
        bool    isModifierPressed()     const;

        void    setApiUrl(const QString &, bool a_save_settings);
        void    setApiLoginUri(const QString &, bool a_save_settings);
        void    setApiTranslateUri(const QString &, bool a_save_settings);
        void    setApiAddWordUri(const QString &, bool a_save_settings);
        void    setApiLogin(const QString &, bool a_save_settings);
        void    setApiPassword(const QString &, bool a_save_settings);

        void    setTranslateMode(int32_t, bool a_save_settings);
        void    setPronVolume(int32_t, bool a_save_settings);

    protected slots:
        void    initSlot();
        void    requestTranslateSlot(const QString &);
        void    doTranslateSlot(const QString &);
        void    doAddWordSlot(
            const QString &,    // en
            const QString &     // ru
        );
        void    timerStartTranslateSlot();
		void    loginSuccessSlot();
		void    loginFailedSlot();
		void    translateSuccessSlot(const QJsonObject &);
		void    doAddWordSuccessSlot(const QJsonObject &);
		void    translateFailedSlot();
        void    errorSlot(const QString &);
        void    messageClickedSlot();

    protected:
        void    closeEvent(QCloseEvent *event) override;
        QString getSettingsFilename();
        void    doAuth();
        void    doSaveTrayInfo();

    private slots:
        void    setIcon(int index);
        void    iconActivated(QSystemTrayIcon::ActivationReason reason);
        void    clipboardSelectionChangedSlot();
        void    clipboardDataChangedSlot();
        void    doSaveTraySlot();

    private:
        void    createIconGroupBox();
        void    createMessageGroupBox();
        void    createActions();
        void    createTrayIcon();

        QComboBox       *iconComboBox;
        QAction         *quitAction;

        QSystemTrayIcon *m_tray_icon;
        QMenu           *m_tray_icon_menu;
        Leo             *m_leo;
        TrayWidget      *m_tray_wiget;
        QTimer          *m_timer_start_translate;

        QString         m_api_url;
        QString         m_api_login_uri;
        QString         m_api_translate_uri;
        QString         m_api_add_word_uri;
        QString         m_api_login;
        QString         m_api_password;

        bool            m_is_logged;

        QString         m_text_to_translate;
        bool            m_can_translate;

        QLocalServer    m_instance_server;
        QString         m_app_key;

        int32_t         m_translate_mode;
        int32_t         m_pron_volume;
};

#endif

