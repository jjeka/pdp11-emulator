#include "disasmodel.h"
#include <QColor>

DisasModel::DisasModel(Vcpu* vcpu) :
    vcpu_(vcpu)
{
}

void DisasModel::reload()
{
    emit dataChanged(index(0), index(vcpu_->getMemSize() / sizeof(uint16_t) - 1));
}

int DisasModel::rowCount(const QModelIndex&) const
{
    return vcpu_->getMemSize() / sizeof(uint16_t);
}

QVariant DisasModel::data(const QModelIndex &index, int role) const
{
    uint16_t addr = uint16_t(index.row() * sizeof (uint16_t));

    if (!index.isValid() || addr >= vcpu_->getMemSize())
        return QVariant();

    switch (role)
    {
    case  Qt::DisplayRole:
        return QString().sprintf("%.6o: %.6o %s", unsigned(addr),
                                 unsigned(vcpu_->getWordAtAddress(addr)), vcpu_->instrAtAddress(addr).c_str());
    case Qt::CheckStateRole:
        return vcpu_->breakpointExists(addr) ? Qt::Checked : Qt::Unchecked;
    case Qt::BackgroundColorRole:
        if (vcpu_->breakpointExists(addr))
        {
            if (vcpu_->getPC() == addr)
                return QColor(255, 200, 200);
            else
                return QColor(200, 200, 255);
        }
        else if (vcpu_->getPC() == addr)
             return QColor(200, 255, 200);
        else
             return QColor(255, 255, 255);
    default:
        return QVariant();
    }
}

bool DisasModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    uint16_t address = uint16_t(index.row() * sizeof (uint16_t));

    if (role == Qt::CheckStateRole)
    {
        if (value == Qt::Checked)
            vcpu_->addBreakpoint(address);
        else
           vcpu_->removeBreakpoint(address);
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags DisasModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}
