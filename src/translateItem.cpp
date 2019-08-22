#include "translateItem.hpp"

TranslateItemWidget::TranslateItemWidget(
    QWidget *a_parent)
    :   QWidget(a_parent)
{
    m_word_id           = 0;
    m_votes             = 0;
    m_dst_icon          = NULL;
    m_icon_downloader   = NULL;
    m_add_word_button   = NULL;
}

TranslateItemWidget::TranslateItemWidget(
    const TranslateItemWidget &a_val)
    :   QWidget(a_val.parentWidget())
{
    m_word              = a_val.m_word;
    m_word_id           = a_val.m_word_id;
    m_value             = a_val.m_value;
    m_votes             = a_val.m_votes;
    m_pic_url           = a_val.m_pic_url;
    m_dst_icon          = a_val.m_dst_icon;
    m_icon_downloader   = a_val.m_icon_downloader;
    m_add_word_button   = a_val.m_add_word_button;
}

TranslateItemWidget::~TranslateItemWidget()
{
    if (m_icon_downloader){
        m_icon_downloader->deleteLater();
    }
}

// set

void TranslateItemWidget::setWordId(
    int32_t a_val)
{
    m_word_id = a_val;
}

void TranslateItemWidget::setWord(
    const QString &a_val)
{
    m_word = a_val;
}

void TranslateItemWidget::setValue(
    const QString &a_val)
{
    m_value = a_val;
}

void TranslateItemWidget::setVotes(
    int32_t a_val)
{
    m_votes = a_val;
}

void TranslateItemWidget::setPicUrl(
    const QUrl &a_val)
{
    m_pic_url = a_val;
}

// get

int32_t TranslateItemWidget::getWordId() const
{
    return m_word_id;
}

QString TranslateItemWidget::getWord() const
{
    return m_word;
}

QString TranslateItemWidget::getValue() const
{
    return m_value;
}

int32_t TranslateItemWidget::getVotes() const
{
    return m_votes;
}

QUrl TranslateItemWidget::getPicUrl() const
{
    return m_pic_url;
}

void TranslateItemWidget::clearTranslateIcon()
{
    if (m_dst_icon){
        QIcon icon;
        m_dst_icon->setIcon(icon);
    }
}

void TranslateItemWidget::loadImageSlot()
{
    if (    m_icon_downloader
        &&  m_dst_icon)
    {
        QPixmap pixmap;
        pixmap.loadFromData(m_icon_downloader->downloadedData());

        QIcon icon(pixmap);
        m_dst_icon->setIcon(icon);
        m_dst_icon->setIconSize(pixmap.rect().size());
    }
}

void TranslateItemWidget::translateIconSlot()
{
    emit iconClicked();
}

void TranslateItemWidget::loadIconTo(
    QPushButton     *a_icon)
{
    QString url = m_pic_url.toString();

    m_dst_icon = a_icon;

    if (m_icon_downloader){
        m_icon_downloader->deleteLater();
        m_icon_downloader = NULL;
    }

    if (m_dst_icon){
        qInfo() << "load image from: '" << url << "'";

        connect(
            m_dst_icon,
            SIGNAL(clicked()),
            SLOT(translateIconSlot())
        );

        if (url.size()){
            m_icon_downloader = new FileDownloader(
                QUrl(m_pic_url),
                this
            );
            connect(
                m_icon_downloader,
                SIGNAL(downloaded()),
                SLOT(loadImageSlot())
            );
        }
    }
}

void TranslateItemWidget::setAddWordButton(
    QPushButton     *a_btn)
{
    m_add_word_button = a_btn;

    if (m_add_word_button){
        connect(
            m_add_word_button,
            SIGNAL(clicked()),
            SLOT(doAddWordSlot())
        );
    }
}

void TranslateItemWidget::doAddWordSlot()
{
    qInfo() << "TranslateItemWidget::doAddWordSlot()";

    QString word    = getWord();
    QString value   = getValue();

    if (    word.size()
        &&  value.size())
    {
        emit requestAddWord(word, value);
    }
}

