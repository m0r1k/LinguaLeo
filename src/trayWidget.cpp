// vim: ts=4:expandtab

#include "main.hpp"
#include "ui_trayWidget.h"
#include "version.h"
#include "translateItem.hpp"
#include "trayWidget.hpp"

TrayWidget::TrayWidget(
    QWidget *a_parent)
    :   QDialog(a_parent),
        m_ui(new Ui::TrayWidget)
{
    m_ui->setupUi(this);

    m_sound_downloader  = NULL;
    m_sound_player      = NULL;

    m_ui->settings_save->setEnabled(false);
    m_ui->about_version->setText(APP_VERSION "." APP_RELEASE);

    m_ui->settings_pron_volume->setMinimum(0);
    m_ui->settings_pron_volume->setMaximum(100);

    m_ui->translate_results->horizontalHeader()->hide();
    m_ui->translate_results->horizontalHeader()->setStretchLastSection(true);
    m_ui->translate_results->verticalHeader()->hide();
    m_ui->translate_results->setSortingEnabled(false);
    m_ui->translate_results->setRowCount(0);
    m_ui->translate_results->setColumnCount(4);
    m_ui->translate_results->setWordWrap(true);
    m_ui->translate_results->setEditTriggers(QAbstractItemView::NoEditTriggers);

    {
        int32_t width = m_ui->translate_results->width();
        m_ui->translate_results->setColumnWidth(0, 100);
        m_ui->translate_results->setColumnWidth(1, width - 250);
        m_ui->translate_results->setColumnWidth(2, 100);
        m_ui->translate_results->setColumnWidth(3, 100);
    }

    connect(
        m_ui->translate_results,
        SIGNAL(cellActivated(int, int)),
        SLOT(cellActivatedSlot(int, int))
    );

    connect(
        m_ui->translate_results->horizontalHeader(),
        SIGNAL(sectionResized(int, int, int)),
        m_ui->translate_results,
        SLOT(resizeRowsToContents())
    );

    connect(
        m_ui->settings_login,
        SIGNAL(textEdited(const QString &)),
        SLOT(loginChangedSlot(const QString &))
    );

    connect(
        m_ui->settings_password,
        SIGNAL(textEdited(const QString &)),
        SLOT(passwordChangedSlot(const QString &))
    );

    connect(
        m_ui->settings_save,
        SIGNAL(clicked(bool)),
        SLOT(saveSlot(bool))
    );

    connect(
        m_ui->translate_text,
        SIGNAL(textEdited(const QString &)),
        SLOT(translateTextSlot(const QString &))
    );

    connect(
        m_ui->settings_translate_mode,
        SIGNAL(currentIndexChanged(int)),
        SLOT(translateModeChangedSlot(int))
    );

    connect(
        m_ui->settings_pron_volume,
        SIGNAL(valueChanged(int)),
        SLOT(pronValueChangedSlot(int))
    );
}

TrayWidget::~TrayWidget()
{
    if (m_sound_downloader){
        m_sound_downloader->deleteLater();
    }

    delete m_ui;
    m_ui = NULL;
}

// translate

void TrayWidget::cellActivatedSlot(
    int a_row,
    int a_col)
{
    qInfo() << "MORIK cell activated: " << a_row << "x" << a_col;
}

void TrayWidget::translateTextSlot(
    const QString &a_val)
{
    emit(requestTranslate(a_val));
}

void TrayWidget::setTranslateText(
    const QString &a_val)
{
    m_ui->translate_text->setText(a_val);
}

void TrayWidget::setTranslateSoundUrl(
    const QUrl &a_val)
{
    QString url = a_val.toString();

    qInfo() << "load sound from: '" << url << "'";

    if (m_sound_downloader){
        m_sound_downloader->deleteLater();
        m_sound_downloader = NULL;
    }

    if (url.size()){
        m_sound_downloader = new FileDownloader(a_val, this);
        connect(
            m_sound_downloader,
            SIGNAL(downloaded()),
            SLOT(loadSoundSlot())
        );
    }
}

void TrayWidget::loadSoundSlot()
{
    if (m_sound_player){
        m_sound_player->stop();
        m_sound_player->deleteLater();
        m_sound_player = NULL;
    }

    if (m_sound_downloader){
		m_sound_player  = new QMediaPlayer(this);
        QBuffer *buffer = new QBuffer(m_sound_player);
        buffer->setData(m_sound_downloader->downloadedData());
        buffer->open(QIODevice::ReadOnly);

        m_sound_player->setMedia(QMediaContent(), buffer);
        m_sound_player->setVolume(
            m_ui->settings_pron_volume->value()
        );
		m_sound_player->play();
    }
}

void TrayWidget::clearTranslateResults()
{
    m_ui->translate_results->clearContents();
    m_ui->translate_results->setRowCount(0);
}

void TrayWidget::playSoundSlot()
{
    if (m_sound_player){
        m_sound_player->setVolume(
            m_ui->settings_pron_volume->value()
        );
        m_sound_player->play();
    }
}

void TrayWidget::setTranslateResults(
    const QJsonObject &a_obj)
{
    QJsonArray  translates  = a_obj["translate"].toArray();
    int32_t     word_id     = a_obj["word_id"].toInt();
    QString     word        = m_ui->translate_text->text();
    int32_t     i, len      = translates.size();
    int32_t     votes_max   = -1;

    qInfo() << "MORIK word_id: " << word_id;
    qInfo() << "MORIK word: " << word;

    TranslateItemWidgets    sorted;
    TranslateItemWidgetsIt  sorted_it;

    clearTranslateResults();

    for (i = 0; i < len; i++){
        QJsonObject     cur_obj     = translates[i].toObject();
        QString         cur_value   = cur_obj["value"].toString();
        QString         cur_pic_url = cur_obj["pic_url"].toString();
        int32_t         cur_votes   = cur_obj["votes"].toInt();

        TranslateItemWidget   cur_item;

        cur_item.setWord(word);
        cur_item.setWordId(word_id);
        cur_item.setValue(cur_value);
        cur_item.setVotes(cur_votes);
        cur_item.setPicUrl(cur_pic_url);

        if (    -1 == votes_max
            || votes_max < cur_votes)
        {
            votes_max = cur_votes;
        }

        sorted.append(cur_item);
    }

    qSort(sorted);

    for (sorted_it = sorted.begin(), i = 0;
        sorted_it != sorted.end();
        sorted_it++, i++)
    {
        const TranslateItemWidget &cur_item = *sorted_it;

        // int32_t     cur_word_id = cur_item.getWordId();
        QString     cur_word    = cur_item.getWord();
        QString     cur_value   = cur_item.getValue();
        int32_t     cur_votes   = cur_item.getVotes();
        QUrl        cur_pic_url = cur_item.getPicUrl();

        m_ui->translate_results->insertRow(i);

        // icon
        {
            TranslateItemWidget *widget      = new TranslateItemWidget(cur_item);
            QPushButton         *icon        = new QPushButton();
            QHBoxLayout         *icon_layout = NULL;

            widget->loadIconTo(icon);

            icon->setStyleSheet(
                "QPushButton { border: none; }"
            );

            icon_layout = new QHBoxLayout(widget);
            icon_layout->addWidget(icon);
            icon_layout->setAlignment(Qt::AlignCenter);
            icon_layout->setContentsMargins(5, 5, 5, 5);
            widget->setLayout(icon_layout);
            m_ui->translate_results->setCellWidget(i, 0, widget);

            connect(
                widget,
                SIGNAL(iconClicked()),
                SLOT(playSoundSlot())
            );
        }

        // text
        {
            TranslateItemWidget *widget      = new TranslateItemWidget(cur_item);
            QTextEdit           *text        = new QTextEdit();
            QHBoxLayout         *text_layout = NULL;

            text->setReadOnly(true);
            TrayWidget::setTextEditRowsCount(text, 3);
            text->setText(cur_value);

            text_layout = new QHBoxLayout(widget);
            text_layout->addWidget(text);
            text_layout->setAlignment(Qt::AlignCenter);
            text_layout->setContentsMargins(5, 5, 5, 5);
            widget->setLayout(text_layout);
            m_ui->translate_results->setCellWidget(i, 1, widget);
        }

        // votes
        {
            TranslateItemWidget *widget       = new TranslateItemWidget(cur_item);
            QProgressBar        *votes        = new QProgressBar();
            QHBoxLayout         *votes_layout = NULL;

            votes->setMaximum(votes_max);
            votes->setValue(cur_votes);

            votes_layout = new QHBoxLayout(widget);
            votes_layout->addWidget(votes);
            votes_layout->setAlignment(Qt::AlignCenter);
            votes_layout->setContentsMargins(5, 5, 5, 5);
            widget->setLayout(votes_layout);
            m_ui->translate_results->setCellWidget(i, 2, widget);
        }

        // button 'add'
        {
            TranslateItemWidget *widget     = new TranslateItemWidget(cur_item);
            QPushButton         *btn        = new QPushButton();
            QHBoxLayout         *btn_layout = NULL;

            btn->setText(tr("Add"));
            widget->setAddWordButton(btn);

            btn_layout = new QHBoxLayout(widget);
            btn_layout->addWidget(btn);
            btn_layout->setAlignment(Qt::AlignCenter);
            btn_layout->setContentsMargins(5, 5, 5, 5);
            widget->setLayout(btn_layout);
            m_ui->translate_results->setCellWidget(i, 3, widget);

            connect(
                widget,
                SIGNAL(requestAddWord(
                    const QString &,    // en
                    const QString &     // ru
                )),
                SLOT(requestAddWordSlot(
                    const QString &,    // en
                    const QString &     // ru
                ))
            );
        }
    }

    //m_ui->translate_results->resizeColumnsToContents();
    m_ui->translate_results->resizeRowsToContents();
}

void TrayWidget::setTranslateTranscription(
    const QString &a_val)
{
    m_ui->translate_transcription->setText(a_val);
}

// login

QString TrayWidget::getLogin()
{
    return m_ui->settings_login->text();
}

void TrayWidget::setLogin(
    const QString &a_val)
{
    m_ui->settings_login->setText(a_val);
}

void TrayWidget::loginChangedSlot(
    const QString &)
{
    m_ui->settings_save->setEnabled(true);
}

// password

QString TrayWidget::getPassword()
{
    return m_ui->settings_password->text();
}

void TrayWidget::setPassword(
    const QString &a_val)
{
    m_ui->settings_password->setText(a_val);
}

void TrayWidget::passwordChangedSlot(
    const QString &)
{
    m_ui->settings_save->setEnabled(true);
}

//

void TrayWidget::translateModeChangedSlot(
    int a_val)
{
    qInfo() << "translation mode changed: " << a_val;
    m_ui->settings_save->setEnabled(true);
}

void TrayWidget::pronValueChangedSlot(
    int a_val)
{
    qInfo() << "pron volume: " << a_val;
    m_ui->settings_save->setEnabled(true);
}

//

int32_t TrayWidget::getTranslateMode() const
{
    return m_ui->settings_translate_mode->currentIndex();
}

int32_t TrayWidget::getPronVolume() const
{
    return m_ui->settings_pron_volume->value();
}

void TrayWidget::setTranslateMode(
    int32_t a_val)
{
    m_ui->settings_translate_mode->setCurrentIndex(a_val);
}

void TrayWidget::setPronVolume(
    int32_t a_val)
{
    m_ui->settings_pron_volume->setValue(a_val);
}

// add word

void TrayWidget::requestAddWordSlot(
    const QString &a_en_word,
    const QString &a_ru_word)
{
    qInfo() << "TrayWidget::requestAddWordSlot,"
        << " en word: " << a_en_word
        << " ru word: " << a_ru_word;

    emit requestAddWord(a_en_word, a_ru_word);
}

// save

void TrayWidget::saveSlot(
    bool)
{
    m_ui->settings_save->setEnabled(false);
    emit doSave();
}

// static

void TrayWidget::setTextEditRowsCount(
    QTextEdit   *a_text_edit,
    int32_t     a_rows)
{
    QFontMetrics metrics(a_text_edit->font());
    int32_t row_height = metrics.lineSpacing();
    a_text_edit->setFixedHeight(a_rows * row_height);
}

