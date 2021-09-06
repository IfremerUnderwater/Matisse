#ifndef MATISSE_IMPORT_EXPORT_HELPER_H_
#define MATISSE_IMPORT_EXPORT_HELPER_H_

#include <QObject>
#include <QWidget>

#include "matisse_engine.h"

namespace matisse {

class ImportExportHelper : public QObject
{
    Q_OBJECT
public:
    ImportExportHelper(QObject * _parent = nullptr);
    void executeExportWorkflow(bool _is_job_export_action, QString _entity_name, bool _is_custom_export,
                               QString _custom_prefix="", QString _custom_export_path="");
    bool executeImportWorkflow(bool _is_job_import_action, QString _current_assembly_name="");

    void setEngine(MatisseEngine *_engine) { m_engine = _engine; }

    void init();
    QString customExportFilename() { return m_custom_export_filename; }
    QString exportPath() { return m_export_path; }
    QString importPath() { return m_import_path; }

private:
    void checkImportExportDirs();

    static const QString ASSEMBLY_EXPORT_PREFIX;
    static const QString JOB_EXPORT_PREFIX;

    QWidget *m_parent_widget;
    MatisseEngine * m_engine;
    QString m_export_path;
    QString m_import_path;
    QString m_custom_export_filename;
};

} // namespace matisse

#endif // MATISSE_IMPORT_EXPORT_HELPER_H_
