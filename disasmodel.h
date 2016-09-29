#ifndef DISASMODEL_H
#define DISASMODEL_H

#include <QAbstractListModel>
#include "vcpu.h"

class DisasModel : public QAbstractListModel
{
    Q_OBJECT

    Vcpu* vcpu_;

public:
    DisasModel(Vcpu* vcpu);

    void reload();

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif // DISASMODEL_H
