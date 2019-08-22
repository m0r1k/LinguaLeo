#ifndef TRANSLATE_ITEM_HPP
#define TRANSLATE_ITEM_HPP

#include <QObject>
#include <QWidget>
#include <QVector>
#include <QPushButton>
#include <QUrl>
#include <QBuffer>
#include <QDebug>

#include "filedownloader.hpp"

class TranslateItemWidget;

typedef QVector<TranslateItemWidget>          TranslateItemWidgets;
typedef TranslateItemWidgets::iterator        TranslateItemWidgetsIt;
typedef TranslateItemWidgets::const_iterator  TranslateItemWidgetsConstIt;

class TranslateItemWidget
    :   public QWidget
{
    Q_OBJECT

    public:
        TranslateItemWidget(QWidget *a_parent = NULL);
        TranslateItemWidget(const TranslateItemWidget &);
        ~TranslateItemWidget();

        void        setWordId(int32_t);
        void        setWord(const QString &);
        void        setValue(const QString &);
        void        setVotes(int32_t);
        void        setPicUrl(const QUrl &);

        int32_t     getWordId()     const;
        QString     getWord()       const;
        QString     getValue()      const;
        int32_t     getVotes()      const;
        QUrl        getPicUrl()     const;

        void        loadIconTo(QPushButton *);
        void        setAddWordButton(QPushButton *);
		void		clearTranslateIcon();

        bool operator<(const TranslateItemWidget &a_r) const {
            return m_votes > a_r.m_votes;
        }

        TranslateItemWidget & operator=(const TranslateItemWidget &a_r)
        {
            m_word              = a_r.m_word;
            m_word_id           = a_r.m_word_id;
            m_value             = a_r.m_value;
            m_votes             = a_r.m_votes;
            m_pic_url           = a_r.m_pic_url;
            m_dst_icon          = a_r.m_dst_icon;
            m_icon_downloader   = a_r.m_icon_downloader;
            m_add_word_button   = a_r.m_add_word_button;

            return *this;
        }

    signals:
        void        iconClicked();
        void        requestAddWord(
            const QString &,    // eng
            const QString &     // rus
        );

    protected slots:
		void		translateIconSlot();
        void        loadImageSlot();
        void        doAddWordSlot();

    private:
        int32_t     m_word_id;
        QString     m_word;
        QString     m_value;
        int32_t     m_votes;
        QUrl        m_pic_url;

        FileDownloader  *m_icon_downloader;
        QPushButton     *m_dst_icon;
        QPushButton     *m_add_word_button;
};

#endif

