#ifndef TABLE_WIDGET_HPP
#define TABLE_WIDGET_HPP

#include <QWidget>
#include <QTableWidget>

class TableWidget
    : public QTableWidget
{
    Q_OBJECT

    public:
        TableWidget(QWidget *a_parent = NULL);
        ~TableWidget();

};

#endif

