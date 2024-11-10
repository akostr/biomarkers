#pragma once

#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QByteArray>
#include <QPoint>

#include <logic/enums.h>

////////////////////////////////////////////////////
//// API
namespace API
{
  ////////////////////////////////////////////////////
  //// REST API
  namespace REST
  {
    //// Handlers
    typedef std::function<void(QNetworkReply*, QJsonDocument)>               ReplyHandleFunc;
    /*
     [](QNetworkReply*, QJsonDocument)
     {
     }
    */
    typedef std::function<void(QNetworkReply*, QVariantMap)>                 ReplyHandleFuncTable;
    /*
     [](QNetworkReply*, QVariantMap)
     {
     }
    */
    typedef std::function<void(QNetworkReply*, QNetworkReply::NetworkError)> ReplyErrorFunc;
    /*
     [](QNetworkReply*, QNetworkReply::NetworkError)
     {
     }
    */
    typedef std::function<void(qint64 bytesSent, qint64 bytesTotal)>         ReplyUploadProgressFunc;
    /*
     [](qint64 bytesSent, qint64 bytesTotal)
     {
     }
    */



    using ReplyHandleByteArray = std::function<void(QNetworkReply*, QByteArray)>;

    //// Passport
    QNetworkReply* GetSampleTypes(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetFluidTypes(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetSamplePreparators(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetGasCarriers(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetDetectors(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetFieldsClustersWellsLayersHierarchy(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* SavePassport(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error, Qt::ConnectionType ConnectionType = Qt::QueuedConnection);
    QNetworkReply* SavePassportV2(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error, Qt::ConnectionType ConnectionType = Qt::QueuedConnection);
    QNetworkReply* GetPassports(const QList<int> filesIds, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* BaselineData(int projectFileId, bool keysOnly, std::optional<int> smoothLvl, ReplyHandleFunc handle, ReplyErrorFunc error, Qt::ConnectionType connType = Qt::QueuedConnection);


    //// Authorize
    void Authorize(const QString& username, const QString& password, ReplyHandleFuncTable handle, API::REST::ReplyErrorFunc error);

    //    //// Get current user info
    //    void GetCurrentUserInfo(ReplyHandleFuncTable handle, API::REST::ReplyErrorFunc error);

    QNetworkReply* GetOccupiedProjectsNames(ReplyHandleFunc handle, API::REST::ReplyErrorFunc error);
    QNetworkReply* RenameAndSetProjectStatus(int id, QString title, QString status, ReplyHandleFunc handle, API::REST::ReplyErrorFunc error);
    QNetworkReply* SetProjectData(int id, QString title, QString status, const QString& comment,
      const QString& codeWord, ReplyHandleFunc handle, API::REST::ReplyErrorFunc error);
    QNetworkReply* RenameTable(int id, QString title, ReplyHandleFunc handle, ReplyErrorFunc error);

    //// Get chromotogram data
    QNetworkReply* GetProjectFileCromotogramData(int id, ReplyHandleFunc handle, ReplyErrorFunc error);
    ////LEGASY
    QNetworkReply* GetFileChromatogramAndPassport(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetFilePassport(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error);
    ////
    QNetworkReply* GetFileChromatogram(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetProjectFileIonsData(int id, const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* calcHotteling(int id, const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error);


    QNetworkReply* GetProjectFileCromotogramPeakIntegration(int id, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetProjectFileCromotogramPeakMarkup(int id, const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetMarkupsTableData(int versionId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetTableAnalysisPCA(QJsonObject tableDataJson, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetTableAnalysisMCR(QJsonObject tableDataJson, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetMarkupVersions(int markupId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetMarkupVersionLast(int markupId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetReference(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetProjectReferences(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* UpdateReferenceData(int referenceId, const QString& title, const QString& comment, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* DeleteReference(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetReferenceDependencies(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* DeleteReferenceDependencies(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* AddMarkup(int projectId, const QString& title, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* UpdateMarkupData(int markupId, const QString& title, const QString& comment, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* AddMarkupWithFiles(int projectId, const QString& title, const QString& comment, QList<int> filesIds, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* RemoveFileFromMarkup(int markupId, int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetMarkupVersion(int projectId, int markupId, int versionId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* SetMarkupVersion(QJsonObject data, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetHeigthRatioMatrix(QJsonObject data, ReplyHandleFunc handle, ReplyErrorFunc error);

    //// Upload file
    QNetworkReply* UploadFile(const QString& fileName, int projectId, int typeId, const QByteArray& data, int groupId, const QString& m_z,
      ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progress = nullptr);
    QNetworkReply* UploadImportTable(const QString& fileName, int projectId, const QByteArray& data,
      ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progress = nullptr);
    QNetworkReply* UploadParsedImportTable(QJsonObject& data,
                                          ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progress = nullptr);
    QNetworkReply *GetXlsColumnsCombos(const QString &tableType, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* ParseXlsTable(const QByteArray& fileData, const QString& fileName, int projectId,
                                 ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* UploadImportTable(const QString& fileName, int userId, int projectId, const QByteArray& data,
      ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progress = nullptr);
    QNetworkReply* RenameTable(int tableId, QString title, QString comment, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* EditTable(int tableId, QString title, QString comment, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* SaveAnalysis(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error);
    // update analysis by json. json is the same as save analysis
    QNetworkReply* UpdateTitleForAnalysis(int analysisId, const QString& title, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* UpdateCommentForAnalysis(int analysisId, const QString& comment, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetAnalysis(int analysisId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* RemoveAnalysis(int analysisId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetAnalysisListByType(int projectId, Constants::AnalysisType analysisType, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetParentAnalysisList(int markupId, Constants::AnalysisType analysisType, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* SaveAnalysisComment(int analysisId, QString newComment, ReplyHandleFunc handle, ReplyErrorFunc error);
    //// MarkupSteps
    QNetworkReply* DetectPeaks(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);          //step1
    QNetworkReply* MarkupStep2(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);          //step2
    QNetworkReply* DetectIntermarkers(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);   //step3
    QNetworkReply* TransferPrepare(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);      //step4
    QNetworkReply* TransferMarkers(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);      //step5
    QNetworkReply* TransferBiomarkers(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);      //step5
    QNetworkReply* IntermarkersTransfer(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error); //step6
    QNetworkReply* Identification(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error); //step6
    QNetworkReply* TransferCheck(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);        //step7
    QNetworkReply* UpdateLaying(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error);         //recalculate peaks
    QNetworkReply* AddNewOrModifyCompound(const QJsonObject data,
                     ReplyHandleFunc handle,
                     ReplyErrorFunc error);
    QNetworkReply* AddExistedCompound(const QString groupName, int existedLibraryGroupId,
                            ReplyHandleFunc handle,
                            ReplyErrorFunc error);
    QNetworkReply* AddExistedCoefficient(const QString groupName, int existedLibraryGroupId,
      ReplyHandleFunc handle,
      ReplyErrorFunc error);
    QNetworkReply* AddExistedPlotTemplate(const QString groupName, int existedLibraryGroupId,
                                ReplyHandleFunc handle,
                                ReplyErrorFunc error);

    QNetworkReply* AddNewOrModifyPlotTemplate(const QJsonObject data,
                                    ReplyHandleFunc handle,
                                    ReplyErrorFunc error);

    QNetworkReply* MakeCompoundSystemic(int libraryGroupId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* SaveMarkupState(int matchStep,
      const QSet<int>& dependentFilesIds,
      int markupId,
      const QJsonObject& matchData,
      int projectId,
      int referenceId,
      QString tableTitle,
      QString tableComment,
      ReplyHandleFunc handle,
      ReplyErrorFunc error,
      ReplyUploadProgressFunc progressFunc = nullptr);
    QNetworkReply* SaveIdentificationMarkupState(int matchStep,
      const QSet<int>& dependentFilesIds,
      int markupId,
      const QJsonObject& matchData,
      int projectId,
      int referenceId,
      const QVariantMap& details,
      ReplyHandleFunc handle,
      ReplyErrorFunc error,
      ReplyUploadProgressFunc progressFunc = nullptr);
    QNetworkReply* SaveMarkupAs(int projectId,
      int matchStep,
      int markupId,
      const QSet<int>& dependentFilesIds,
      const QString& title,
      const QString& comment,
      const QJsonObject& matchData,
      ReplyHandleFunc handle,
      ReplyErrorFunc error);
    QNetworkReply* SaveIdentificationAs(int projectId,
      int matchStep,
      int markupId,
      const QSet<int>& dependentFilesIds,
      const QString& title,
      const QString& comment,
      const QJsonObject& matchData,
      ReplyHandleFunc handle,
      ReplyErrorFunc error);
    QNetworkReply* SaveReference(int markupId, const QString& title, const QString& comment, int projectFileId, const QJsonObject& referenceData, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* LoadMarkupState(int markupId, ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progressFunc = nullptr);
    QNetworkReply* GetChromatogrammInfo(const QList<QPair<int, bool>>& chromatogramms, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetReferenceDataTable(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetAvailableCalculationsForTable(int tableId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetDataTables(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetFileGroups(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetTableGroups(int projectId, const QString& tableType, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* DeleteGroup(int projectId, int groupId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* RenameGroup(int projectId, int groupId, const QString& newName, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* AddFilesToGroup(int projectId, const QList<int>& filesIds, int groupId, const QString& groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* AddTablesToGroup(int projectId, const QList<int>& filesIds, int groupId, const QString& groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* RenameFilesGroup(int projectId, int groupId, const QString& newTitle, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* AddFileGroup(int projectId, const QString& groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* UngroupFiles(int projectId, const QList<int>& filesIds, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* UngroupTables(int projectId, const QList<int>& filesIds, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* ExportTables(const QJsonObject& data, ReplyHandleByteArray handle, ReplyErrorFunc error);

    QNetworkReply* ExportDataTables(const int projectId, const QList<int>& tablesIds, ReplyHandleByteArray handle, ReplyErrorFunc error);

    QNetworkReply* MarkupPlsFit(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* MarkupPlsPred(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* ExtractIons(int projectId, QList<int> filesIds, bool sum, QList<int> selectedIons, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetIons(const int projectId, QList<int> filesIds, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetDependentFiles(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetDependentProjectFiles(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* RemoveProjectAndDependencies(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetDependentTables(int tableId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* RemoveFileAndDependenciesFromProject(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* ExportIons(const QJsonObject& data, ReplyHandleByteArray handle, ReplyErrorFunc error);
    QNetworkReply* EditMZ(int projectFileId, QString mz, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* CalculateEFA(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* getProjectHeightRatioInfo(int tableId, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* getProjectFileNameTemplate(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* saveProjectFileNameTemplate(int projectId, const QString& depthType, const QString& pattern, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* getLibraryClassifiers(ReplyHandleFuncTable handle, ReplyErrorFunc error);
    QNetworkReply* getLibraryCompoundClass(ReplyHandleFuncTable handle, ReplyErrorFunc error);
    QNetworkReply* getLibrarySpecifics(ReplyHandleFuncTable handle, ReplyErrorFunc error);
    QNetworkReply* getUserGroups(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* setCompoundFavourite(int id, const QString& group_name, bool state, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* addLibraryGroupCompound(const QList<int>& ids, const QString& groupName, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* deleteLibraryGroupCompound(int id, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* deleteLibraryGroupCoefficient(int id, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* deleteLibraryGroupPlotTemplate(int id, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* compounudDependencies(int groupId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* uploadPdfAttachmentToCompound(int libraryGroupId, QByteArray file, QString fileName, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* downloadPdfAttachmentFromCompound(int libraryGroupId, ReplyHandleByteArray handle, ReplyErrorFunc error);
    QNetworkReply* uploadSpectrumAttachmentToCompound(int libraryGroupId, QByteArray file, QString fileName, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* removePdfAttachmentFromCompound(int attachmentFileId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* removeSpectrumAttachmentFromCompound(int libraryGroupId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetCalcCoefficientsByTableId(int projectId, int id, const QList<int>& lbid, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* SaveCoefficientTable(const QJsonObject& obj, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* SaveMergedTable(const QJsonObject& obj, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetTemplateObjectsForPlotTemplate(int groupId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* SavePlot(const QJsonObject& obj, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* LoadPlot(int plotId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* getIdentificationPlotGroups(int plotId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* deleteIdentificationPlot(int plotId, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* uploadEntityForIdentificationPlot(int library_group_id, int tableId, QString dataType, QList<int> libraryGroupIds, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* changePlotsGroup(int projectId, QList<int> plotIds, bool newGroup, int groupId,
                       QString groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* changeGroupTitle(int projectId, int groupId, QString title, ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* changePlotTitle(int plotId, QString title, QString comment, ReplyHandleFunc handle, ReplyErrorFunc error);

    QNetworkReply* GetTimeRangeBilling(ReplyHandleFunc handle, ReplyErrorFunc error);
    QNetworkReply* GetStatisticFromCalculationResources(const QDateTime& start, const QDateTime& end, ReplyHandleByteArray handle, ReplyErrorFunc error);
    QNetworkReply* GetStatisticFromStorage(ReplyHandleByteArray handle, ReplyErrorFunc error);


    //// Requests herein shall return sql tables serialized in unified format
    namespace Tables
    {
      QNetworkReply* GetNumericTable(int projectId, const QString& tableType, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      //void getMarkupsDataTables(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetProjectReferences(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* PostCheckRelatedLibraryItems(int elementType, const QString& shortTitle,
                                        const QString& fullTitle, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      //// Get analysis list
      QNetworkReply* GetAnalysisListByProjectId(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetAnalysisListByMarkupVersion(int markupId, int versionId, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetAnalysisListByType(int projectId, Constants::AnalysisType analysisType, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get user projects
      QNetworkReply* GetUserProjects(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get user info
      QNetworkReply* GetUserInfo(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //      //// Get other users
      //      void GetOtherUsers(int userId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

            //// Add project
      QNetworkReply* AddProject(const QString& groupName, const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Create project
      QNetworkReply* AddProjectNew(const QString& groupName, const QString& title, const QString& comment,
        const QString& codeWord, const QString& licence, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get project info
      QNetworkReply* GetProjectInfo(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get project users
      QNetworkReply* GetProjectUsers(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get project children
      QNetworkReply* GetProjectChildren(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get project files
      QNetworkReply* GetProjectFiles(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      QNetworkReply* GetMarkupFiles(int markupId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      QNetworkReply* JoinDataTables(int projectId, QList<int> parentIds, QString joinedTableName, QString joinedTableComment, ReplyHandleFunc handle, ReplyErrorFunc error);

      //// Get access constants
      QNetworkReply* GetAccess(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get roles constants
      QNetworkReply* GetRoles(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get file types constants
      QNetworkReply* GetFileTypes(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get file info
      QNetworkReply* GetFileInfo(int fileId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //      //// Add user to project
      //      void AddUserToProject(int projectId, int userId, int accessId, int roleId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //      //// Remove user from project
      //      void RemoveUserFromProject(int projectId, int userId, ReplyHandleFunc handle, ReplyErrorFunc error);

            //// Get project markups (old - indexations)
      QNetworkReply* GetProjectMarkups(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Remove markup (old - indexation)
      QNetworkReply* RemoveMarkup(int id, ReplyHandleFunc handle, ReplyErrorFunc error);

      //// Remove Numeric Data Table
      QNetworkReply* RemoveTable(int id, ReplyHandleFunc handle, ReplyErrorFunc error);

      //// Get markup (old - indexation) versions
      QNetworkReply* GetMarkupVersions(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get job functions constants
      QNetworkReply* GetJobFunctions(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get fields
      QNetworkReply* GetFields(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Add field
      QNetworkReply* AddField(const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get field wells
      QNetworkReply* GetFieldWells(int fieldId, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Add well
      QNetworkReply* AddWell(int fieldId, const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Get layers
      QNetworkReply* GetLayers(ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Add layer
      QNetworkReply* AddLayer(const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      //// Set master file
      QNetworkReply* SetMasterFile(int id, ReplyHandleFunc handle, ReplyErrorFunc error);

      //// Add file info
      QNetworkReply* AddFileInfo(int fileId, int wellId, int layerId, const QDateTime& fileDateTime, QString title, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      /// Set file info
      QNetworkReply* SetFileInfo(int fileId, int wellId, int layerId, const QDateTime& fileDateTime, QString title, ReplyHandleFuncTable handle, ReplyErrorFunc error);

      QNetworkReply* GetMergedTables(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetLibraryElements(LibraryConnection::ElementType type, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetCoefficientsByTableId(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetAllowedGroups(int libraryGroupId, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetTemplatesByTableId(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetIdentificationPlots(int project_id, ReplyHandleFuncTable handle, ReplyErrorFunc error);
      QNetworkReply* GetLibraryElements(int projectId, int elementType, ReplyHandleFuncTable handle, ReplyErrorFunc error);
    }
  }
}
