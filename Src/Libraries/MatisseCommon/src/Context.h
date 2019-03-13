#ifndef CONTEXT_H
#define CONTEXT_H

#include "libmatissecommon_global.h"

#include <QHash>
#include <QList>

namespace MatisseCommon {

///
/// \brief Classe de partage de données entre instances de Processors et d'ImageProvider lors de l'execution d'un assemblage
///
class LIBMATISSECOMMONSHARED_EXPORT Context
{
public:
    Context(){}
    virtual ~Context() {}
    ///
    /// \brief Retourne l'objet associé à la clé
    /// \param name
    /// \return
    ///
    QVariant* getObject(const QString &name) { return _variants.value(name); }

    ///
    /// \brief Retourne l'ensemble des clés
    /// \return
    ///
    QList<QString> getKeys() { return _variants.keys(); }

    ///
    /// \brief Ajoute une clé au contexte
    /// \param name
    /// \param object
    ///
    void addObject(const QString &name, QVariant *object) { _variants.insert(name, object); }
private:
    QHash<QString, QVariant*> _variants;
    // TODO Utiliser une map de QVariant
};
}

#endif // CONTEXT_H
