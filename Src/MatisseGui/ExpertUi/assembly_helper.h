#ifndef ASSEMBLYHELPER_H
#define ASSEMBLYHELPER_H

#include <QObject>
#include <QWidget>

class AssemblyHelper : public QObject
{
    Q_OBJECT
public:
    explicit AssemblyHelper(QObject *parent = nullptr);

signals:

};

#endif // ASSEMBLYHELPER_H
