#include "assembly_helper.h"
#include "assembly_dialog.h"
#include "duplicate_dialog.h"

namespace matisse {

AssemblyHelper::AssemblyHelper(QObject *_parent) : QObject(_parent)
{
    m_parent_widget = static_cast<QWidget*>(_parent);
}

AssemblyDefinition * AssemblyHelper::createNewAssembly()
{
    m_engine->parametersManager()->restoreParametersDefaultValues();

    QString name;
    KeyValueList fields;

    AssemblyDialog dialog(m_parent_widget, name, fields, true);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return nullptr;
    }

    qDebug() << "Creating assembly : " << name;
    foreach (QString key, fields.getKeys()) {
        qDebug() << "Key: " << key << " - Value: " << fields.getValue(key);
    }

    // Ajout d'un caractère '*' à la fin du nom pour indiquer qu'il n'est pas enregistré
    QString display_name = name;
    display_name.append('*');

    AssemblyDefinition *new_assembly = new AssemblyDefinition();
    new_assembly->setName(display_name);
    new_assembly->setCreationDate(QDate::fromString(fields.getValue("Date")));
    new_assembly->setDate(fields.getValue("Date"));
    new_assembly->setVersion(fields.getValue("Version"));
    new_assembly->setAuthor(fields.getValue("Author"));
    new_assembly->setUsable(fields.getValue("Valid").toInt());
    new_assembly->setComment(fields.getValue("Comments"));

    // filename is derived from the assembly name
    QString filename = ProcessDataManager::instance()->fromNameToFileName(name);
    new_assembly->setFilename(filename);

    return new_assembly;
}

bool AssemblyHelper::duplicateAssembly(AssemblyDefinition *_source_assembly)
{
    QString assembly_name = _source_assembly->name();
    QString new_assembly_name;

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QStringList existing_assembly_names = process_data_manager->getAssembliesList();

    DuplicateDialog dialog(m_parent_widget, assembly_name, new_assembly_name, true, existing_assembly_names);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    qDebug() << QString("Duplicating assembly %1 to %2").arg(assembly_name, new_assembly_name);
    bool duplicated = process_data_manager->duplicateAssembly(assembly_name, new_assembly_name);

    if (duplicated) {
        QMessageBox::information(m_parent_widget, tr("Processing chain copy"), tr("Processing chain '%1' copied")
                                 .arg(assembly_name));
    } else {
        QMessageBox::critical(m_parent_widget, tr("Processing chain copy failed"), tr("Processing chain '%1' copy failed.")
                              .arg(assembly_name));
    }

    return duplicated;
}


bool AssemblyHelper::checkBeforeAssemblyDeletion(QString _assembly_name, bool _prompt_user)
{
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    if (process_data_manager->assemblyHasArchivedJob(_assembly_name)) {
        QMessageBox::critical(m_parent_widget, tr("Delete assembly"), tr("Processing chain has archived tasks and cannot be removed.").arg(_assembly_name));
        return false;
    }

    if (_prompt_user) {
        int ret = QMessageBox::question(m_parent_widget, tr("Delete assembly"), tr("Do you want to delete assembly %1 ?").arg(_assembly_name), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if (ret==QMessageBox::Cancel) {
            return false;
        }
    }

    return true;
}


bool AssemblyHelper::deleteAssembly(QString _assembly_name)
{
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    AssemblyDefinition *assembly = process_data_manager->getAssembly(_assembly_name);
    if (!assembly) {
        qCritical() << QString("Assembly '%1' selected to be removed was not found in local repository").arg(_assembly_name);
        return false;
    }

    qDebug() << "Removing assembly and assembly parameters files...";

    QString filename = assembly->filename();

    // Removing assembly file
    QString assembly_filepath = process_data_manager->getAssembliesPath() + QDir::separator() + filename;
    QString param_filepath = process_data_manager->getAssembliesParametersPath() + QDir::separator() + filename;

    QFile file(assembly_filepath);
    if (!file.exists()) {
        qCritical() << QString("Assembly file '%1' does not exist, cannot be removed").arg(assembly_filepath);
        return false;
    }

    if (!file.remove()) {
        qCritical() << QString("Error removing assembly file '%1'").arg(assembly_filepath);
        return false;
    }

    // Assembly parameters file removal is verified, the process is not interrupted if the file cannot be removed
    QFile param_file(param_filepath);

    if (param_file.exists()) {
        if (param_file.remove()) {
            qDebug() << "... done";
        } else {
            qCritical() << QString("Error removing assembly parameters file '%1'").arg(param_filepath);
        }
    } else {
        qCritical() << QString("Assembly parameters file '%1' does not exist, cannot be removed").arg(param_filepath);
    }

    return true;
}

void AssemblyHelper::loadAssemblyParameters(AssemblyDefinition *_assembly)
{
    // reinitialize parameters with their default values
    m_engine->parametersManager()->restoreParametersDefaultValues();

    // loading parameters expected by assembly modules
    m_engine->parametersManager()->clearExpectedParameters();
    m_engine->addParametersForInputDataProvider(_assembly->sourceDefinition()->name());
    foreach (ProcessorDefinition* processor, _assembly->processorDefs()) {
        m_engine->addParametersForProcessor(processor->name());
    }
    m_engine->addParametersForOutputDataWriter(_assembly->destinationDefinition()->name());

    // loading assembly template parameter values
    m_engine->parametersManager()->loadParameters(_assembly->name(), true);
}

bool AssemblyHelper::promptForAssemblyProperties(QString _assembly_name, KeyValueList *_props)
{
    qDebug() << "Updating properties for assembly " << _assembly_name;

    AssemblyDefinition *assembly = ProcessDataManager::instance()->getAssembly(_assembly_name);

    if (!assembly) {
        return false;
    }

    // update properties

    AssemblyDialog dialog(m_parent_widget, _assembly_name, *_props, false, false);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    assembly->setVersion(_props->getValue("Version"));
    assembly->setUsable(_props->getValue("Valid").toInt());
    assembly->setComment(_props->getValue("Comments"));

    return true;
}


} // namespace matisse
