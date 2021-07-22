#ifndef MATISSE_ASSEMBLY_HELPER_H_
#define MATISSE_ASSEMBLY_HELPER_H_

#include <QObject>
#include <QWidget>

#include "matisse_engine.h"
#include "matisse_preferences.h"

namespace matisse {

class AssemblyHelper : public QObject
{
    Q_OBJECT
public:
    explicit AssemblyHelper(QObject *_parent = nullptr);

    AssemblyDefinition *createNewAssembly();
    bool duplicateAssembly(AssemblyDefinition *_source_assembly);
    bool checkBeforeAssemblyDeletion(QString _assembly_name, bool _prompt_user);
    bool deleteAssembly(QString _assembly_name);
    void loadAssemblyParameters(AssemblyDefinition *_assembly);
    bool promptForAssemblyProperties(QString _assembly_name, KeyValueList *_props);

    void setEngine(MatisseEngine *_engine) { m_engine = _engine; }
signals:

private:

    QWidget *m_parent_widget;
    MatisseEngine *m_engine;
};

} // namespace matisse

#endif // MATISSE_ASSEMBLY_HELPER_H_
