// vim: ts=4:expandtab

#include <QAction>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QCloseEvent>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QClipboard>
#include <QTimer>
#include <QLocalSocket>
#include <QApplication>

#include "mainWidget.hpp"

MainWidget::MainWidget()
{
    m_can_translate     = false;
    m_app_key           = APP_NAME;
    m_is_logged         = false;
    m_translate_mode    = 0;
    m_pron_volume       = 100;

    // init only after contructor will be done
    QTimer::singleShot(500, this, SLOT(initSlot()));
}

void MainWidget::initSlot()
{
    QLocalSocket    *socket = NULL;
    int32_t         res;

    // check that onother instance doesn't exist
    socket = new QLocalSocket();
    socket->connectToServer(m_app_key);
    if (socket->isOpen()){
        socket->close();
        QCoreApplication::quit();
        goto out;
    }

    QObject::connect(
        &m_instance_server,
        &QLocalServer::newConnection,
        [this] () {
            // if client (another instance) is connected
            showTrayWidget();
        }
    );

    QLocalServer::removeServer(m_app_key);
    res = m_instance_server.listen(m_app_key);
    if (!res){
        QCoreApplication::quit();
        goto out;
    }

    // icons
    createIconGroupBox();

    // tray
    m_tray_wiget = new TrayWidget(this);
    connect(
        m_tray_wiget,
        SIGNAL(doSave()),
        SLOT(doSaveTraySlot())
    );
    connect(
        m_tray_wiget,
        SIGNAL(requestTranslate(const QString &)),
        SLOT(requestTranslateSlot(const QString &))
    );
    connect(
        m_tray_wiget,
        SIGNAL(requestAddWord(
            const QString &,    // en
            const QString &     // ru
        )),
        SLOT(doAddWordSlot(
            const QString &,    // en
            const QString &     // ru
        ))
    );

    // leo
    m_leo = new Leo(this);
    connect(
        m_leo,
        SIGNAL(loginSuccess()),
        SLOT(loginSuccessSlot())
    );
    connect(
        m_leo,
        SIGNAL(loginFailed()),
        SLOT(loginFailedSlot())
    );
    connect(
        m_leo,
        SIGNAL(translateSuccess(const QJsonObject &)),
        SLOT(translateSuccessSlot(const QJsonObject &))
    );
    connect(
        m_leo,
        SIGNAL(doAddWordSuccess(const QJsonObject &)),
        SLOT(doAddWordSuccessSlot(const QJsonObject &))
    );
    connect(
        m_leo,
        SIGNAL(translateFailed()),
        SLOT(translateFailedSlot())
    );
    connect(
        m_leo,
        SIGNAL(error(const QString &)),
        SLOT(errorSlot(const QString &))
    );

    // timer
    m_timer_start_translate = new QTimer(this);
    m_timer_start_translate->setSingleShot(true);
    connect(
        m_timer_start_translate,
        SIGNAL(timeout()),
        SLOT(timerStartTranslateSlot())
    );

    createActions();
    createTrayIcon();

    loadSettings();
    doAuth();

    // init clipboard
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        connect(
            clipboard,
            SIGNAL(selectionChanged()),
            SLOT(clipboardSelectionChangedSlot())
        );
        connect(
            clipboard,
            SIGNAL(dataChanged()),
            SLOT(clipboardDataChangedSlot())
        );
    }

    connect(
        m_tray_icon,
        &QSystemTrayIcon::activated,
        this,
        &MainWidget::iconActivated
    );

    connect(
        m_tray_icon,
        SIGNAL(messageClicked()),
        this,
        SLOT(messageClickedSlot())
    );

    setIcon(1);

    m_tray_icon->show();

    setWindowTitle(APP_NAME);

    {
        QString msg = tr("%1 started and hidden to the system tray")
            .arg(APP_NAME);

        QMessageBox *message_box = new QMessageBox(
            QMessageBox::Information,
            tr(APP_NAME),
            msg,
            QMessageBox::Ok,
            this
        );
        message_box->setModal(false);
        message_box->setAttribute(Qt::WA_DeleteOnClose, true);
        message_box->exec();
    }

out:
    if (socket){
        socket->deleteLater();
        socket = NULL;
    }
    return;
}

void MainWidget::setCanTranslate(
    bool a_val)
{
    m_can_translate = a_val;
}

void MainWidget::loginSuccessSlot()
{
    m_is_logged = true;
    setIcon(0);
}

void MainWidget::loginFailedSlot()
{
    m_is_logged = false;
    setIcon(1);
}

void MainWidget::showTrayWidget() const
{
    m_tray_wiget->show();
    m_tray_wiget->activateWindow();
    m_tray_wiget->raise();
}

void MainWidget::translateSuccessSlot(
    const QJsonObject   &a_obj)
{
    QString     sound_url       = a_obj["sound_url"].toString();
    QString     transcription   = a_obj["transcription"].toString();

    m_tray_wiget->setTranslateTranscription(transcription);
    m_tray_wiget->setTranslateSoundUrl(sound_url);
    m_tray_wiget->setTranslateResults(a_obj);

    showTrayWidget();
}

void MainWidget::doAddWordSuccessSlot(
    const QJsonObject   &a_obj __attribute__((unused)))
{
    QString msg = tr("a word has been added successfully");

    m_tray_icon->showMessage(
        APP_NAME,
        msg,
        QSystemTrayIcon::Warning,
        5 * 1000
    );
}

void MainWidget::translateFailedSlot()
{
}

void MainWidget::errorSlot(
    const QString &a_msg)
{
    qInfo() << "ERROR: " << a_msg;

    m_tray_icon->showMessage(
        APP_NAME,
        a_msg,
        QSystemTrayIcon::Warning,
        5 * 1000
    );
}

QString MainWidget::getSettingsFilename()
{
    QString ret;

    ret = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    if (!QDir(ret).exists()){
        QDir().mkdir(ret);
    }

    ret += "/main.ini";

    return ret;
}

void MainWidget::loadSettings()
{
    QString fname = getSettingsFilename();

    QSettings settings(fname, QSettings::NativeFormat);
    settings.setIniCodec("UTF-8");

    setApiUrl(
        settings.value("api_url", DEFAULT_API_URL).toString(),
        false
    );
    setApiLoginUri(
        settings.value("api_login_uri", DEFAULT_API_LOGIN_URI).toString(),
        false
    );
    setApiTranslateUri(
        settings.value("api_translate_uri", DEFAULT_API_TRANSLATE_URI).toString(),
        false
    );
    setApiAddWordUri(
        settings.value("api_add_word_uri", DEFAULT_API_ADD_WORD_URI).toString(),
        false
    );
    setApiLogin(
        settings.value("api_login", DEFAULT_API_LOGIN).toString(),
        false
    );
    setApiPassword(
        settings.value("api_password", DEFAULT_API_PASSWORD).toString(),
        false
    );
    setTranslateMode(
        settings.value("translate_mode", DEFAULT_TRANSLATE_MODE).toInt(),
        false
    );
    setPronVolume(
        settings.value("pron_volume", DEFAULT_PRON_VALUE).toInt(),
        false
    );
}

QString MainWidget::getApiUrl() const
{
    return m_api_url;
}

QString MainWidget::getApiLoginUri() const
{
    return m_api_login_uri;
}

QString MainWidget::getApiTranslateUri() const
{
    return m_api_translate_uri;
}

QString MainWidget::getApiAddWordUri() const
{
    return m_api_add_word_uri;
}

void MainWidget::setApiUrl(
    const QString   &a_val,
    bool            a_save_settings)
{
    m_api_url = a_val;

    m_leo->setApiUrl(a_val);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::setApiLoginUri(
    const QString   &a_val,
    bool            a_save_settings)
{
    m_api_login_uri = a_val;

    m_leo->setApiLoginUri(a_val);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::setApiTranslateUri(
    const QString   &a_val,
    bool            a_save_settings)
{
    m_api_translate_uri = a_val;

    m_leo->setApiTranslateUri(a_val);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::setApiAddWordUri(
    const QString   &a_val,
    bool            a_save_settings)
{
    m_api_add_word_uri = a_val;

    m_leo->setApiAddWordUri(a_val);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::setApiLogin(
    const QString   &a_val,
    bool            a_save_settings)
{
    m_api_login = a_val;

    m_tray_wiget->setLogin(m_api_login);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::setApiPassword(
    const QString   &a_val,
    bool            a_save_settings)
{
    m_api_password = a_val;

    m_tray_wiget->setPassword(m_api_password);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::setTranslateMode(
    int32_t         a_val,
    bool            a_save_settings)
{
    m_translate_mode = a_val;

    m_tray_wiget->setTranslateMode(m_translate_mode);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::setPronVolume(
    int32_t         a_val,
    bool            a_save_settings)
{
    m_pron_volume = a_val;

    m_tray_wiget->setPronVolume(m_pron_volume);

    if (a_save_settings){
        saveSettings();
    }
}

void MainWidget::saveSettings()
{
    QString fname = getSettingsFilename();
    QSettings settings(fname, QSettings::NativeFormat);
    settings.setIniCodec("UTF-8");

    settings.setValue("api_url",            m_api_url);
    settings.setValue("api_login_uri",      m_api_login_uri);
    settings.setValue("api_translate_uri",  m_api_translate_uri);
    settings.setValue("api_add_word_uri",   m_api_add_word_uri);
    settings.setValue("api_login",          m_api_login);
    settings.setValue("api_password",       m_api_password);

    settings.setValue("translate_mode",     m_translate_mode);
    settings.setValue("pron_volume",        m_pron_volume);
}

void MainWidget::doSaveTraySlot()
{
    setApiLogin(m_tray_wiget->getLogin(),               false);
    setApiPassword(m_tray_wiget->getPassword(),         false);
    setTranslateMode(m_tray_wiget->getTranslateMode(),  false);
    setPronVolume(m_tray_wiget->getPronVolume(),        false);
    saveSettings();

    if (!m_is_logged){
        QString login           = m_tray_wiget->getLogin();
        QString password        = m_tray_wiget->getPassword();

        if (    login.size()
            &&  password.size())
        {
            doAuth();
        }
    }
}

void MainWidget::doAuth()
{
    QString api_url         = getApiUrl();
    QString api_login_uri   = getApiLoginUri();
    QString login           = m_tray_wiget->getLogin();
    QString password        = m_tray_wiget->getPassword();

    if (!api_url.size()){
        QString error_str = tr("empty API URL");
        errorSlot(error_str);
        goto fail;
    }

    if (!api_login_uri.size()){
        QString error_str = tr("empty API login URI");
        errorSlot(error_str);
        goto fail;
    }

    if (!login.size()){
        QString error_str = tr("empty login");
        errorSlot(error_str);
        goto fail;
    }

    if (!password.size()){
        QString error_str = tr("empty password");
        errorSlot(error_str);
        goto fail;
    }

    m_leo->doAuth(login, password);

out:
    return;
fail:
    goto out;
}

void MainWidget::requestTranslateSlot(
    const QString &a_text)
{
    setCanTranslate(true);
    doTranslateSlot(a_text);
}

void MainWidget::doTranslateSlot(
    const QString &a_text)
{
    if (a_text.size()){
        m_text_to_translate = a_text;
        m_timer_start_translate->start(1000);
    }
}

void MainWidget::doAddWordSlot(
    const QString &a_en_word, // en
    const QString &a_ru_word) // ru
{
    if (    a_en_word.size()
        &&  a_ru_word.size())
    {
        m_leo->doAddWord(
            a_en_word,
            a_ru_word
        );
    }
}

bool MainWidget::isModifierPressed() const
{
    bool                  ret  = false;
    Qt::KeyboardModifiers kmod = QApplication::queryKeyboardModifiers();

    if (0 == m_translate_mode){
        if (    (Qt::ControlModifier & kmod)
            &&  (Qt::ShiftModifier   & kmod))
        {
            ret = true;
        }
    } else if (1 == m_translate_mode){
        ret = true;
    }

    return ret;
}

void MainWidget::timerStartTranslateSlot()
{
    qInfo() << "MORIK timerStartTranslateSlot: " << m_timer_start_translate;

    if (isModifierPressed()){
        m_can_translate = true;
    }

    if (m_can_translate){
        m_tray_wiget->setTranslateText(m_text_to_translate);
        m_tray_wiget->clearTranslateResults();
        m_tray_wiget->setTranslateTranscription("");

        m_leo->doTranslate(m_text_to_translate);
        m_can_translate = false;
    }
}

void MainWidget::clipboardSelectionChangedSlot()
{
    QClipboard  *clipboard  = QGuiApplication::clipboard();
    QString     text        = clipboard->text(QClipboard::Selection);

    qInfo() << "MORIK selection changed: " << text;

    if (    !text.size()
        ||  "\n" == text)
    {
        goto out;
    }

    if (isModifierPressed()){
        setCanTranslate(true);
    }

    doTranslateSlot(text);

out:
    return;
}

void MainWidget::clipboardDataChangedSlot()
{
    QClipboard  *clipboard  = QGuiApplication::clipboard();
    QString     text        = clipboard->text();

    qInfo() << "MORIK data changed: " << text;
}

void MainWidget::setVisible(
    bool a_visible __attribute__((unused)))
{
    QDialog::setVisible(false);
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    if (m_tray_icon->isVisible()){
        QMessageBox::information(
            this,
            tr("Systray"),
            tr("The program will keep running in the "
               "system tray. To terminate the program, "
               "choose <b>Quit</b> in the context menu "
               "of the system tray entry.")
        );
        hide();
        event->ignore();
    }
}

void MainWidget::setIcon(int index)
{
    QIcon icon = iconComboBox->itemIcon(index);
    m_tray_icon->setIcon(icon);
    setWindowIcon(icon);

    m_tray_icon->setToolTip(iconComboBox->itemText(index));
}

void MainWidget::iconActivated(
    QSystemTrayIcon::ActivationReason a_reason)
{
    qInfo() << "MORIK iconActivated " << a_reason;

    switch (a_reason){
        case QSystemTrayIcon::Trigger:
            showTrayWidget();
            break;

        case QSystemTrayIcon::DoubleClick:
            break;

        case QSystemTrayIcon::MiddleClick:
            break;

        default:
            break;
    }
}

void MainWidget::messageClickedSlot()
{
    qInfo() << "MORIK messageClicked\n";
}

void MainWidget::createIconGroupBox()
{
    iconComboBox = new QComboBox;
    iconComboBox->addItem(QIcon(":/images/i128.png"),   tr("LinguaLeo online"));
    iconComboBox->addItem(QIcon(":/images/i128bw.png"), tr("LinguaLeo offline"));
}

void MainWidget::createActions()
{
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void MainWidget::createTrayIcon()
{
    m_tray_icon_menu = new QMenu(this);

    m_tray_icon_menu->addAction(quitAction);

    m_tray_icon = new QSystemTrayIcon(this);
    m_tray_icon->setContextMenu(m_tray_icon_menu);
}

