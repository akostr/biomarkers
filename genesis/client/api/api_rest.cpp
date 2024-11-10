#include "api_rest.h"
#include "network_manager.h"
#include <QUrlQuery>
#include <QFile>
#define API_V1 QString("api/v1/")
#define API_V2 QString("api/v2/")

using namespace Constants;

////////////////////////////////////////////////////
//// API
namespace API
{
  ////////////////////////////////////////////////////
  //// REST API
  namespace REST
  {
    //// Authorize
    void Authorize(const QString& username, const QString& password, ReplyHandleFuncTable handle, API::REST::ReplyErrorFunc error)
    {
      NetworkManager::Get()->Authorize(API_V1 + "login", username, password,
        [handle, error](QNetworkReply*, QJsonDocument /* tokens - are kept by network manager */)
        {
          Tables::GetUserInfo(handle, error);
        },
        error);
    }

    //    //// Get current user info
    //    void GetCurrentUserInfo(ReplyHandleFuncTable handle, API::REST::ReplyErrorFunc error)
    //    {
    //      NetworkManager::Get()->GetRequest(API_V1 + "user/info"
    //                                        , QUrlQuery()
    //                                        , handle
    //                                        , error);
    //    }

      //// Get chromotogram data
    QNetworkReply* GetProjectFileCromotogramData(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "imports/%1/chromatogram").arg(id)
        , QUrlQuery()
        , handle
        , error
        , QVariant()
        , Qt::AutoConnection);
    }

    ////Get ions data
    QNetworkReply* GetProjectFileIonsData(int id, const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {

      return NetworkManager::Get()->PostRequest((API_V1 + "imports/%1/chromatogram/ions_data").arg(id)
        , data
        , handle
        , error);
    }

    QNetworkReply* calcHotteling(int id, const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V2 + "project/%1/analysis/hotelling").arg(id)
        , data
        , handle
        , error);
    }

    QNetworkReply* GetProjectFileCromotogramPeakIntegration(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "imports/%1/chromatogram/peak_integration").arg(id)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* GetProjectFileCromotogramPeakMarkup(int id, const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "imports/%1/chromatogram/peak_markup").arg(id)
        , data
        , handle
        , error);
    }

    QNetworkReply* GetMarkupsTableData(int versionId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QUrlQuery params;
      params.addQueryItem("version_id", QString::number(versionId));
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/table")
        , params
        , handle
        , error);
    }

    QNetworkReply* GetMarkupVersions(int markupId,
      ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/%1/versions/old").arg(markupId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* GetMarkupVersionLast(int markupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/%1/versions/last").arg(markupId)
        , QUrlQuery()
        , handle
        , error);
    }
    QNetworkReply* GetProjectReferences(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/project_etalons/%1").arg(projectId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* UpdateReferenceData(int referenceId, const QString& title, const QString& comment, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["title"] = title;
      obj["comment"] = comment;
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/etalon/%1/update_comment_and_title").arg(referenceId)
        , obj
        , handle
        , error);
    }

    QNetworkReply* DeleteReference(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "markups/etalon/%1").arg(referenceId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* AddMarkup(int projectId, const QString& title,
      ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["project_id"] = projectId;
      obj["title"] = title;
      return NetworkManager::Get()->PostRequest((API_V1 + "markups")
        , obj
        , handle
        , error);
    }
    QNetworkReply* AddMarkupWithFiles(int projectId, const QString& title, const QString& comment, QList<int> filesIds, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["project_id"] = projectId;
      obj["title"] = title;
      QJsonArray jfilesIds;
      obj["comment"] = comment;
      //projectFileIds required;
      for (auto& id : filesIds)
        jfilesIds << id;
      obj["files"] = jfilesIds;
      return NetworkManager::Get()->PostRequest((API_V1 + "markups")
        , obj
        , handle
        , error);
    }

    QNetworkReply* GetMarkupVersion(int projectId, int markupId, int versionId,
      ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QUrlQuery params;
      params.addQueryItem("project_id", QString::number(projectId));
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/%1/versions/%2/markups").arg(markupId).arg(versionId)
        , params
        , handle
        , error);
    }

    QNetworkReply* SetMarkupVersion(QJsonObject data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups")
        , data
        , handle
        , error);
    }

    QNetworkReply* GetHeigthRatioMatrix(QJsonObject data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/height_ratio_matrix")
        , data
        , handle
        , error);
    }

    //// Upload file
    QNetworkReply* UploadFile(const QString& fileName, int projectId, int typeId, const QByteArray& data, int groupId, const QString& m_z,
      ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progress)
    {
      //qDebug() << "group id for import = " << data.size();
      //qDebug() << "file size = " << groupId;
      QVariantMap map;
      map["project_id"] = projectId;
      map["type_id"] = typeId;
      map["file_name"] = fileName;
      map["m_z"] = m_z;
      map["group_id"] = groupId;

      //      qDebug().noquote() << "import file json:\n" << QJsonDocument(obj).toJson();

      QString tag = fileName.split("/").last();
      return NetworkManager::Get()->PostRequest((API_V1 + "imports"), map, data, handle, error, tag, Qt::ConnectionType::QueuedConnection, progress);
    }

    QNetworkReply* UploadImportTable(const QString& fileName, int projectId, const QByteArray& data, ReplyHandleFunc handle,
      ReplyErrorFunc error, ReplyUploadProgressFunc progress)
    {
      //qDebug() << "group id for import = " << data.size();
      QString tag = fileName.split("/").last();
      QVariantMap map;
      map["project_id"] = projectId;
      map["file_name"] = tag;

      return NetworkManager::Get()->PostRequest(API_V1 + QString("import_table"), map, data, handle, error, tag, Qt::QueuedConnection, progress);
    }

    QNetworkReply* GetTableAnalysisPCA(QJsonObject tableDataJson, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/pca")
        , tableDataJson
        , handle
        , error);
    }

    QNetworkReply* GetTableAnalysisMCR(QJsonObject tableDataJson, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/mcr")
        , tableDataJson
        , handle
        , error);
    }

    QNetworkReply* SaveAnalysis(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "analysis")
        , data
        , handle
        , error);
    }

    QNetworkReply* UpdateTitleForAnalysis(int analysisId, const QString& title, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "analysis/%1/title").arg(analysisId)
        , QUrlQuery{ {"title", title} }
        , handle
        , error);
    }

    QNetworkReply* UpdateCommentForAnalysis(int analysisId, const QString& comment, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "analysis/%1/comment").arg(analysisId)
        , QUrlQuery{ {"comment", comment} }
        , handle
        , error);
    }

    QNetworkReply* GetAnalysis(int analysisId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "analysis/%1").arg(analysisId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* RemoveAnalysis(int analysisId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "analysis/%1").arg(analysisId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* GetAnalysisListByType(int projectId, AnalysisType AnalysisType, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/analysis/%2")
        .arg(projectId)
        .arg(static_cast<int>(AnalysisType))
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* GetParentAnalysisList(int markupId, Constants::AnalysisType analysisType, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/parent_analyzes/%1/%2")
        .arg(markupId)
        .arg(static_cast<int>(analysisType))
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* GetCalcCoefficientsByTableId(int projectId, int id, const QList<int>& lbid, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["data_table_id"] = id;
      QJsonArray ids;
      for (const auto& id : lbid)
        ids.append(id);
      obj["chosen_library_group_ids"] = ids;
      obj.insert("project_id", projectId);
      return NetworkManager::Get()->PostRequest(API_V1 + QString("library/calc_coefficient"),
        obj,
        handle,
        error);
    }

    QNetworkReply* SaveCoefficientTable(const QJsonObject& obj, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest(API_V1 + QString("library/coefficient_table"),
        obj,
        handle,
        error);
    }

    QNetworkReply* SaveMergedTable(const QJsonObject& obj, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest(API_V1 + QString("markups/data_tables/merge_identified_tables"),
        obj,
        handle,
        error);
    }

    //// Requests herein shall return sql tables serialized in unified format
    namespace Tables
    {
      QNetworkReply* GetProjectReferences(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "markups/project_etalons/%1").arg(projectId)
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get analysis list
      QNetworkReply* GetAnalysisListByMarkupVersion(int markupId, int versionId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "markups/%1/versions/%2/analysis").arg(markupId).arg(versionId)
          , QUrlQuery()
          , handle
          , error);
      }

      QNetworkReply* GetAnalysisListByType(int projectId, Constants::AnalysisType analysisType, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/analysis/%2")
          .arg(projectId)
          .arg(static_cast<int>(analysisType))
          , QUrlQuery()
          , handle
          , error);
      }

      QNetworkReply* GetAnalysisListByProjectId(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/analysis").arg(projectId)
          , QUrlQuery()
          , handle
          , error);
      }


      //// Get user projects
      QNetworkReply* GetUserProjects(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "user/projects")
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get user info
      QNetworkReply* GetUserInfo(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "user/info")
          , QUrlQuery()
          , handle
          , error);
      }

      //      //// Get other users
      //      void GetOtherUsers(int userId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      //      {
      //        NetworkManager::Get()->GetRequest((API_V1 + "users/%1/other").arg(userId)
      //                                          , QUrlQuery()
      //                                          , handle
      //                                          , error);
      //      }

          //// Add project
      QNetworkReply* AddProject(const QString& groupName, const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["title"] = title;
        obj["group_name"] = groupName;
        obj["licence_name"] = QString("licence-reservoir");

#ifdef DEBUG
        qDebug().noquote() << QJsonDocument(obj).toJson();
#endif
        /*
        NetworkManager::Get()->PostRequest((API_V1 + "projects?group_name=%1")
                           .arg(groupName)
                  , obj
                  , handle
                  , error);
        */
        return NetworkManager::Get()->PostRequest((API_V1 + "projects")
          , obj
          , handle
          , error);
      }

      //// Add project
      QNetworkReply* AddProjectNew(const QString& groupName, const QString& title, const QString& comment,
        const QString& codeWord, const QString& licence, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["title"] = title;
        obj["group_name"] = groupName;
        obj["comment"] = comment;
        obj["code_word"] = codeWord;
        obj["licence_name"] = licence;

#ifdef DEBUG
        qDebug().noquote() << QJsonDocument(obj).toJson();
#endif
        return NetworkManager::Get()->PostRequest((API_V1 + "projects_new")
          , obj
          , handle
          , error);
      }

      //// Get project info
      QNetworkReply* GetProjectInfo(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/info").arg(projectId)
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get project users
      QNetworkReply* GetProjectUsers(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/users").arg(projectId)
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get project children
      QNetworkReply* GetProjectChildren(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/children").arg(projectId)
          , QUrlQuery()
          , handle
          , error);
      }


      //// Get project files
      QNetworkReply* GetProjectFiles(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/files").arg(projectId)
          , QUrlQuery()
          , handle
          , error);
      }

      QNetworkReply* GetMarkupFiles(int markupId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "markups/%1/files").arg(markupId)
          , QUrlQuery()
          , handle
          , error
          , QVariant()
          , Qt::AutoConnection);
      }

      //// Get access constants
      QNetworkReply* GetAccess(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/accesses")
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get roles constants
      QNetworkReply* GetRoles(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/roles")
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get file types constants
      QNetworkReply* GetFileTypes(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "files/types")
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get file info
      QNetworkReply* GetFileInfo(int fileId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "files/%1/info").arg(fileId)
          , QUrlQuery()
          , handle
          , error);
      }

      //      //// Add user to project
      //      void AddUserToProject(int projectId, int userId, int accessId, int roleId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      //      {
      //        QJsonObject obj;
      //        obj["id_access"]  = QString::number(accessId);
      //        obj["id_role"]    = QString::number(roleId);

      //        NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/users/%2")
      //                                           .arg(projectId)
      //                                           .arg(userId)
      //                          , obj
      //                          , handle
      //                          , error);
      //      }

      //      //// Remove user from project
      //      void RemoveUserFromProject(int projectId, int userId, ReplyHandleFunc handle, ReplyErrorFunc error)
      //      {
      //        NetworkManager::Get()->DeleteRequest((API_V1 + "projects/%1/users/%2").arg(projectId).arg(userId), QUrlQuery(), handle, error);
      //      }

          //// Get project markups
      QNetworkReply* GetProjectMarkups(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/markups").arg(id)
          , QUrlQuery()
          , handle
          , error);
      }

      //// Remove markup
      QNetworkReply* RemoveMarkup(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->DeleteRequest((API_V1 + "markups/%1").arg(id),
          QUrlQuery(),
          handle,
          error);
      }

      QNetworkReply* RemoveTable(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->DeleteRequest((API_V1 + "markups/data_tables/%1").arg(id),
          QUrlQuery(),
          handle,
          error);
      }

      //// Get markup versions
      QNetworkReply* GetMarkupVersions(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "markups/%1/versions").arg(id)
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get job functions
      QNetworkReply* GetJobFunctions(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "users/job-functions")
          , QUrlQuery()
          , handle
          , error);
      }

      //// Get fields
      QNetworkReply* GetFields(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "fields")
          , QUrlQuery()
          , handle
          , error);
      }

      //// Add field
      QNetworkReply* AddField(const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["title"] = title;

        return NetworkManager::Get()->PostRequest((API_V1 + "fields")
          , obj
          , handle
          , error);
      }

      //// Get field wells
      QNetworkReply* GetFieldWells(int fieldId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "fields/%1/wells").arg(fieldId)
          , QUrlQuery()
          , handle
          , error);
      }

      //// Add well
      QNetworkReply* AddWell(int fieldId, const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["title"] = title;

        return NetworkManager::Get()->PostRequest((API_V1 + "fields/%1/wells").arg(fieldId)
          , obj
          , handle
          , error);
      }

      //// Get layers
      QNetworkReply* GetLayers(ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "layers")
          , QUrlQuery()
          , handle
          , error);
      }

      //// Add layer
      QNetworkReply* AddLayer(const QString& title, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["title"] = title;

        return NetworkManager::Get()->PostRequest((API_V1 + "layers")
          , obj
          , handle
          , error);
      }

      //// Set master file
      QNetworkReply* SetMasterFile(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
      {
        QUrlQuery queryParams;
        queryParams.addQueryItem("id", QString::number(id));

        return NetworkManager::Get()->PutRequest((API_V1 + "master")
          , queryParams
          , handle
          , error);
      }

      //// Add file info
      QNetworkReply* AddFileInfo(int fileId, int wellId, int layerId, const QDateTime& fileDateTime, QString title, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["id_well"] = wellId;
        obj["id_layer"] = layerId;
        obj["file_datetime"] = fileDateTime.toString(Qt::ISODate);
        obj["title"] = title;

        return NetworkManager::Get()->PostRequest((API_V1 + "files/%1/info").arg(fileId)
          , obj
          , handle
          , error);

      }

      //// Set file info
      QNetworkReply* SetFileInfo(int fileId, int wellId, int layerId, const QDateTime& fileDateTime, QString title, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["id_well"] = wellId;
        obj["id_layer"] = layerId;
        obj["file_datetime"] = fileDateTime.toString(Qt::ISODate);
        obj["title"] = title;

        return NetworkManager::Get()->PutRequest((API_V1 + "files/%1/info").arg(fileId)
          , obj
          , handle
          , error);
      }

      QNetworkReply* GetNumericTable(int projectId, const QString& tableType, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QUrlQuery query;
        query.addQueryItem("table_type", tableType);
        return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/data_tables").arg(projectId)
          , query
          , handle
          , error);
      }

      QNetworkReply* JoinDataTables(int projectId, QList<int> parentIds, QString joinedTableName, QString joinedTableComment,
        ReplyHandleFunc handle, ReplyErrorFunc error)
      {
        QJsonObject root;
        //      "user_id": 22, "project_id": 249, "parent_ids": [73, 75], "title": "Имя объединенной таблицы"
        QJsonArray ids;
        for (auto& id : parentIds)
          ids << id;
        root["project_id"] = projectId;
        root["parent_ids"] = ids;
        root["title"] = joinedTableName;
        root["comment"] = joinedTableComment;
        return NetworkManager::Get()->PostRequest((API_V1 + "markups/data_tables/merge_tables")
          , root
          , handle
          , error);
      }

      QNetworkReply* GetLibraryElements(LibraryConnection::ElementType type, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V2 + QString("library/elements/%1/info").arg(static_cast<int>(type))),
          QUrlQuery(),
          handle,
          error);
      }

      QNetworkReply* GetCoefficientsByTableId(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest(API_V1 + QString("library/%1/coefficient").arg(id),
          QUrlQuery(),
          handle,
          error);
      }

      void GetIdentifiedTables(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        NetworkManager::Get()->GetRequest((API_V1 + "markups/data_tables_identified/%1").arg(projectId),
          QUrlQuery(),
          handle,
          error);
      }

      QNetworkReply* GetMergedTables(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "markups/data_tables_identified_merged/%1").arg(projectId),
          QUrlQuery(),
          handle,
          error);
      }

      QNetworkReply* GetTemplatesByTableId(int id, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest(API_V1 + QString("library/%1/templates").arg(id),
          QUrlQuery(),
          handle,
          error);

      }

      QNetworkReply* GetIdentificationPlots(int project_id, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QUrlQuery query;
        query.addQueryItem("plot_type_id", "1");
        return NetworkManager::Get()->GetRequest(API_V1 + QString("projects/%1/plots").arg(project_id),
          query,
          handle,
          error);
      }

      void getJoinedTables(int projectId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        NetworkManager::Get()->GetRequest((API_V1 + "markups/merged_tables/%1").arg(projectId),
          QUrlQuery(),
          handle,
          error);
      }

      QNetworkReply* PostCheckRelatedLibraryItems(int elementType,
        const QString& shortTitle,
        const QString& fullTitle,
        ReplyHandleFuncTable handle,
        ReplyErrorFunc error)
      {
        QJsonObject obj;
        obj["short_title"] = QString(shortTitle.toUtf8());
        obj["full_title"] = QString(fullTitle.toUtf8());
        //        qDebug() << QJsonDocument(obj).toJson(QJsonDocument::Compact);
        return NetworkManager::Get()->PostRequest((API_V1 + "library/%1/title").arg(elementType)
          , obj
          , handle
          , error);
      }

      QNetworkReply* GetAllowedGroups(int libraryGroupId, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        return NetworkManager::Get()->GetRequest((API_V1 + "library/%1/allowed_groups").arg(libraryGroupId),
          QUrlQuery(),
          handle,
          error);
      }

      QNetworkReply* GetLibraryElements(int projectId, int elementType, ReplyHandleFuncTable handle, ReplyErrorFunc error)
      {
        QUrlQuery params;
        //element_type_id: 1 - compounds; 2 - coefficients
        params.addQueryItem("element_type_id", QString::number(elementType));
        return NetworkManager::Get()->GetRequest(QString(API_V1 + "projects/%1/elements").arg(projectId),
          params,
          handle,
          error);
      }

    }//namespace Tables

    QNetworkReply* SaveAnalysisComment(int analysisId, QString newComment, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QUrlQuery params;
      params.addQueryItem("comment", newComment);
      return NetworkManager::Get()->PostRequest((API_V1 + "analysis/%1/comment").arg(analysisId)
        , params
        , handle
        , error);
    }

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
      ReplyUploadProgressFunc progressFunc)
    {
      QJsonObject root;
      root["saved_table_title"] = tableTitle;
      root["saved_table_comment"] = tableComment;
      root["project_id"] = projectId;
      root["match_step"] = matchStep;
      root["match_data"] = matchData;
      root["etalon_id"] = referenceId;
      QJsonArray jfiles;
      for (auto& id : dependentFilesIds)
        jfiles << id;
      root["files"] = jfiles;
      // uncomment this part to save file with markup
      /*
      const auto fileName = QString("markup-match-state%1.json")
        .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy_hh_mm_ss"));
      QFile saveRequest(fileName);
      saveRequest.open(QIODevice::WriteOnly);
      saveRequest.write(QJsonDocument(root).toJson());
      saveRequest.close();
      */
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/match").arg(markupId),
        root,
        handle,
        error,
        QVariant(),
        Qt::AutoConnection,
        progressFunc);
    }

    QNetworkReply* SaveIdentificationMarkupState(int matchStep,
      const QSet<int>& dependentFilesIds,
      int markupId,
      const QJsonObject& matchData,
      int projectId,
      int referenceId,
      const QVariantMap& details,
      ReplyHandleFunc handle,
      ReplyErrorFunc error,
      ReplyUploadProgressFunc progressFunc)
    {
      QJsonObject root;
      if (!details.contains("title")
        && !details.contains("comment"))
      {
        //no table data
        //that way because of compatibility
        root["saved_table_title"] = "";
        root["saved_table_comment"] = "";
      }
      else
      {
        //has table data
        root["saved_table_title"] = details.value("title").toString();
        root["saved_table_comment"] = details.value("comment").toString();

        if (details.value("into_common").toBool())
        {
          //into common tables group
          root["tables_group_id"] = 0;
          root["create_group"] = false;
          root["group_title"] = "";
        }
        else if (details.value("existed").toBool())
        {
          //into specific existed group
          root["tables_group_id"] = details.value("group_id").toInt();
          root["create_group"] = false;
          root["group_title"] = details.value("group_title").toString();
        }
        else
        {
          //into new specific group
          root["tables_group_id"] = 0;
          root["create_group"] = true;
          root["group_title"] = details.value("group_title").toString();
        }
      }
      root["project_id"] = projectId;
      root["match_step"] = matchStep;
      root["match_data"] = matchData;
      root["etalon_id"] = referenceId;
      QJsonArray jfiles;
      for (auto& id : dependentFilesIds)
        jfiles << id;
      root["files"] = jfiles;
      // uncomment this part to save file with markup
      /*
      const auto fileName = QString("markup-match-state%1.json")
        .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy_hh_mm_ss"));
      QFile saveRequest(fileName);
      saveRequest.open(QIODevice::WriteOnly);
      saveRequest.write(QJsonDocument(root).toJson());
      saveRequest.close();
      */
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/identification").arg(markupId),
        root,
        handle,
        error,
        QVariant(),
        Qt::AutoConnection,
        progressFunc);
    }

    QNetworkReply* LoadMarkupState(int markupId, ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progressFunc)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/%1/match").arg(markupId),
        QUrlQuery(),
        handle,
        error,
        QVariant(),
        Qt::AutoConnection,
        progressFunc);
    }

    QNetworkReply* DetectPeaks(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-1").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* MarkupStep2(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-2").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* DetectIntermarkers(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-3").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* TransferPrepare(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-4").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* TransferMarkers(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-5").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* TransferBiomarkers(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {

      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-transfer-biomarkers").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* IntermarkersTransfer(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-6").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* TransferCheck(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-7").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* UpdateLaying(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/update").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* GetChromatogrammInfo(const QList<QPair<int, bool> >& chromatogramms, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      QJsonArray files;
      for (auto& c : chromatogramms)
      {
        QJsonObject file;
        file["master"] = c.second;
        file["project_file_id"] = c.first;
        files.append(file);
      }
      data["files"] = files;
      return NetworkManager::Get()->PostRequest((API_V1 + "imports/%1/chromatogram/info").arg(1)//any non-zero, exact like his said :)
        , data
        , handle
        , error
      );
    }

    QNetworkReply* GetFileGroups(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/group").arg(projectId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* GetTableGroups(int projectId, const QString& tableType, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      // To get only imported table groups
      QUrlQuery params;
      params.addQueryItem("table_type", tableType);
      return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/data_tables/groups").arg(projectId)
        , params
        , handle
        , error);
    }

    QNetworkReply* AddFilesToGroup(int projectId, const QList<int>& filesIds, int groupId, const QString& groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      {
        bool assertCondition = (groupId == 0 && groupTitle.isEmpty());
        Q_ASSERT(!(groupId == 0 && groupTitle.isEmpty()));//for proper message on error
        if (assertCondition)//for release
          return nullptr;
      }
      QJsonObject root;

      QJsonArray filesArray;
      for (auto& fileId : filesIds)
        filesArray << fileId;
      root["files"] = filesArray;

      if (groupId == 0 && !groupTitle.isEmpty())
      {//create group case
        root["title"] = groupTitle;
        root["create_group"] = true;
      }
      else if (groupId != 0)
      {//add to existing group case
        root["group_id"] = groupId;
      }

      //qDebug().noquote() << QJsonDocument(root).toJson();

      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/group-files").arg(projectId)
        , root
        , handle
        , error);
    }

    QNetworkReply* AddTablesToGroup(int projectId, const QList<int>& filesIds, int groupId, const QString& groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      {
        bool assertCondition = (groupId == 0 && groupTitle.isEmpty());
        Q_ASSERT(!(groupId == 0 && groupTitle.isEmpty()));//for proper message on error
        if (assertCondition)//for release
          return nullptr;
      }
      QJsonObject root;

      QJsonArray filesArray;
      for (auto& fileId : filesIds)
        filesArray << fileId;
      root["table_ids"] = filesArray;

      if (groupId == 0 && !groupTitle.isEmpty())
      {//create group case
        root["group_title"] = groupTitle;
        root["create_group"] = true;
      }
      else if (groupId != 0)
      {//add to existing group case
        root["tables_group_id"] = groupId;
      }

      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/data_tables/group-tables").arg(projectId)
        , root
        , handle
        , error);
    }

    QNetworkReply* RenameFilesGroup(int projectId, int groupId, const QString& newTitle, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject root;
      root["group_title"] = newTitle;
      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/data_tables/groups/%2/update").arg(projectId).arg(groupId)
        , root
        , handle
        , error);
    }

    QNetworkReply* UngroupFiles(int projectId, const QList<int>& filesIds, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject root;

      QJsonArray filesArray;
      for (auto& fileId : filesIds)
        filesArray << fileId;
      root["files"] = filesArray;
      root["create_group"] = false;
      root["group_id"] = QJsonValue::Null;

      //      qDebug().noquote() << QJsonDocument(root).toJson();

      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/group-files").arg(projectId)
        , root
        , handle
        , error);
    }

    QNetworkReply* UngroupTables(int projectId, const QList<int>& filesIds, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject root;
      QJsonArray filesArray;
      for (auto& fileId : filesIds)
        filesArray << fileId;
      root["table_ids"] = filesArray;
      root["create_group"] = false;
      root["tables_group_id"] = QJsonValue::Null;

      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/data_tables/group-tables").arg(projectId)
        , root
        , handle
        , error);
    }

    QNetworkReply* ExportTables(const QJsonObject& data, ReplyHandleByteArray handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "exports"), data, handle, error);
    }

    QNetworkReply* ExportDataTables(const int projectId, const QList<int>& tablesIds, ReplyHandleByteArray handle, ReplyErrorFunc error)
    {
      QJsonArray ids;
      for (auto& id : tablesIds)
        ids << id;
      QJsonObject obj;
      obj.insert("project_id", projectId);
      obj["data_table_ids"] = ids;
      return NetworkManager::Get()->PostRequest(API_V1 + QString("markups/data_tables/export"),
        obj,
        handle,
        error);
    }

    QNetworkReply* MarkupPlsFit(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/pls_fit"), data, handle, error);
    }

    QNetworkReply* MarkupPlsPred(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/pls_pred"), data, handle, error);
    }

    QNetworkReply* DeleteGroup(int projectId, int groupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "projects/%1/group?group_id=%2").arg(projectId).arg(groupId)
        , QUrlQuery()
        , handle
        , error);

    }

    QNetworkReply* RenameGroup(int projectId, int groupId, const QString& newName, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject root;
      root["group_id"] = groupId;
      root["title"] = newName;

      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/group").arg(projectId)
        , root
        , handle
        , error);
    }

    QNetworkReply* AddFileGroup(int projectId, const QString& groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject root;
      root["title"] = groupTitle;
      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/group").arg(projectId)
        , root
        , handle
        , error);
    }

    QNetworkReply* RemoveFileFromMarkup(int markupId, int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "markups/%1/markup_files/%2").arg(markupId).arg(projectFileId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* ExtractIons(int projectId, QList<int> filesIds, bool sum, QList<int> selectedIons, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject root;
      QJsonArray jIons;
      QJsonArray jFiles;
      for (auto& ion : selectedIons)
        jIons << ion;
      for (auto& fileId : filesIds)
      {
        QJsonObject file;
        file["project_file_id"] = fileId;
        jFiles << file;
      }
      root["selected_ions"] = jIons;
      root["sum"] = sum;
      root["project_id"] = projectId;
      root["files"] = jFiles;

      return NetworkManager::Get()->PostRequest((API_V1 + "chromatogram/extract"),
        root,
        handle,
        error);
    }

    QNetworkReply* GetIons(const int projectId, QList<int> filesIds, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject root;
      QJsonArray jFiles;
      for (auto& fileId : filesIds)
      {
        QJsonObject file;
        file["project_file_id"] = fileId;
        jFiles << file;
      }
      root["files"] = jFiles;
      root.insert("project_id", projectId);
      return NetworkManager::Get()->PostRequest((API_V1 + "chromatogram/ions"),
        root,
        handle,
        error);
    }

    QNetworkReply* GetDependentFiles(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "project_file/%1/objects").arg(projectFileId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetDependentProjectFiles(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "project/%1/objects").arg(projectId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* RemoveProjectAndDependencies(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "projects/%1/delete").arg(projectId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* RemoveFileAndDependenciesFromProject(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "project_file/%1/objects").arg(projectFileId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* ExportIons(const QJsonObject& data, ReplyHandleByteArray handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "exports/ions"), data, handle, error);
    }

    QNetworkReply* EditMZ(int projectFileId, QString mz, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QUrlQuery queryParams;
      queryParams.addQueryItem("m_z", mz);
      return NetworkManager::Get()->PostRequest((API_V1 + "project_file/%1/m_z").arg(projectFileId),
        queryParams,
        handle,
        error);
    }

    QNetworkReply* CalculateEFA(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest(API_V1 + QString("markups/efa"),
        data,
        handle,
        error);
    }

    QNetworkReply* SaveMarkupAs(int projectId,
      int matchStep,
      int markupId,
      const QSet<int>& dependentFilesIds,
      const QString& title,
      const QString& comment,
      const QJsonObject& matchData,
      ReplyHandleFunc handle,
      ReplyErrorFunc error)
    {
      QJsonObject data;
      data["project_id"] = projectId;
      data["title"] = title;
      data["comment"] = comment;
      data["matching_step"] = matchStep;
      data["current_markup_id"] = markupId;
      data["match_data"] = matchData;
      QJsonArray jfiles;
      for (auto& id : dependentFilesIds)
        jfiles << id;
      data["files"] = jfiles;

      return NetworkManager::Get()->PostRequest((API_V1 + "markups/markup_save_as"),
        data,
        handle,
        error);
    }

    QNetworkReply* SaveIdentificationAs(int projectId,
      int matchStep,
      int markupId,
      const QSet<int>& dependentFilesIds,
      const QString& title,
      const QString& comment,
      const QJsonObject& matchData,
      ReplyHandleFunc handle,
      ReplyErrorFunc error)
    {
      QJsonObject data;
      data["project_id"] = projectId;
      data["title"] = title;
      data["comment"] = comment;
      data["matching_step"] = matchStep;
      data["current_markup_id"] = markupId;
      data["match_data"] = matchData;
      QJsonArray jfiles;
      for (auto& id : dependentFilesIds)
        jfiles << id;
      data["files"] = jfiles;

      return NetworkManager::Get()->PostRequest((API_V1 + "markups/markup_save_as_identification"),
        data,
        handle,
        error);
    }

    QNetworkReply* SaveReference(int markupId, const QString& title, const QString& comment, int projectFileId, const QJsonObject& referenceData, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["project_file_id"] = projectFileId;
      data["created_markup_id"] = markupId;
      data["etalon_title"] = title;
      data["etalon_comment"] = comment;
      data["etalon_data"] = referenceData;

      //qDebug().noquote() << QJsonDocument(data).toJson();

      return NetworkManager::Get()->PostRequest((API_V1 + "markups/etalon"),
        data,
        handle,
        error);
    }

    QNetworkReply* GetFileChromatogramAndPassport(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "imports/%1/chromatogram_data").arg(projectFileId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetFilePassport(int projectFileId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "imports/%1/chromatogram_passport").arg(projectFileId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetReference(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/etalon/%1").arg(referenceId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetReferenceDataTable(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(
        (API_V1 + "markups/data_tables/%1/get_table").arg(referenceId),
        QUrlQuery(), handle, error);
    }

    QNetworkReply* GetAvailableCalculationsForTable(int tableId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(
        (API_V1 + "library/%1/available_calculations").arg(tableId),
        QUrlQuery(), handle, error);
    }

    QNetworkReply* GetDataTables(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/data_tables/%1").arg(projectId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetOccupiedProjectsNames(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "projects"),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* RenameAndSetProjectStatus(int id, QString title, QString status, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["title"] = title;
      data["status_code_name"] = status;
      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/status_update").arg(id),
        data,
        handle,
        error);
    }

    QNetworkReply* SetProjectData(int id, QString title, QString status, const QString& comment,
      const QString& codeWord, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["title"] = title;
      data["status_code_name"] = status;
      data["comment"] = comment;
      data["code_word"] = codeWord;
      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/status_update_new").arg(id),
        data,
        handle,
        error);
    }

    QNetworkReply* RenameTable(int id, QString title, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["title"] = title;
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/data_tables/%1/update_title").arg(id),
        data,
        handle,
        error);
    }

    QNetworkReply* GetSampleTypes(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "files/sample_types",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetFluidTypes(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "files/fluid_types",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetSamplePreparators(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "passport/chemical_data/sample_preparators",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetGasCarriers(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "passport/chemical_data/gas_carriers",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetDetectors(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "passport/chemical_data/detectors",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetFieldsClustersWellsLayersHierarchy(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "fields/wells_hierarchy",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* SavePassport(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error, Qt::ConnectionType ConnectionType)
    {
      return NetworkManager::Get()->PostRequest(API_V1 + "files/pass_info",
        data,
        handle,
        error,
        ConnectionType);
    }

    QNetworkReply* SavePassportV2(const QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error, Qt::ConnectionType ConnectionType)
    {
      return NetworkManager::Get()->PostRequest(API_V2 + "files/pass_info",
        data,
        handle,
        error,
        ConnectionType);
    }

    QNetworkReply* GetPassports(const QList<int> filesIds, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QStringList strList;
      for (auto& id : filesIds)
        strList << QString::number(id);

      return NetworkManager::Get()->GetRequest(API_V1 + QString("files/%1/pass_info").arg(strList.join(',')),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetDependentTables(int tableId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/data_tables/%1/objects").arg(tableId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* RenameTable(int tableId, QString title, QString comment, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["table_id"] = tableId;
      data["title"] = title;
      data["comment"] = comment;

      return NetworkManager::Get()->PostRequest((API_V1 + "markups/data_tables/%1/update_title_comment").arg(tableId),
        data,
        handle,
        error);

    }

    QNetworkReply* UpdateMarkupData(int markupId, const QString& title, const QString& comment, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["title"] = title;
      obj["comment"] = comment;
      obj["markup_id"] = markupId;
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/update_comment_and_title").arg(markupId)
        , obj
        , handle
        , error);
    }

    QNetworkReply* getProjectHeightRatioInfo(int tableId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/data_tables/%1/get_table_info").arg(tableId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* getProjectFileNameTemplate(int projectId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "projects/%1/get_sample_project").arg(projectId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* saveProjectFileNameTemplate(int projectId, const QString& depthType, const QString& pattern, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["depth_type"] = depthType;
      obj["sample"] = pattern;
      return NetworkManager::Get()->PostRequest((API_V1 + "projects/%1/sample_update").arg(projectId)
        , obj
        , handle
        , error);
    }

    QNetworkReply* getLibraryClassifiers(ReplyHandleFuncTable handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "library/classifier"),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* getLibraryCompoundClass(ReplyHandleFuncTable handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "library/classifier_compound_class"),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* getLibrarySpecifics(ReplyHandleFuncTable handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "library/specifics"),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* getUserGroups(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "user/groups"),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* setCompoundFavourite(int id, const QString& group_name, bool state, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject request;
      request["group_name"] = group_name;
      request["state"] = state;
      return NetworkManager::Get()->PostRequest((API_V1 + "library/%1/favourites").arg(id),
        request,
        handle,
        error);
    }

    QNetworkReply* addLibraryGroupCompound(const QList<int>& ids, const QString& groupName, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject request;
      request["group_name"] = groupName;
      QJsonArray arr;
      std::copy(ids.begin(), ids.end(), std::back_inserter(arr));
      request["library_group_id"] = arr;
      return NetworkManager::Get()->PostRequest((API_V1 + "library/group/compound"),
        request,
        handle,
        error);
    }

    QNetworkReply* deleteLibraryGroupCompound(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "library/%1/compound").arg(id),
        QUrlQuery(),
        handle,
        error, QVariant(), Qt::QueuedConnection);
    }

    QNetworkReply* deleteLibraryGroupCoefficient(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "library/%1/coefficient").arg(id),
        QUrlQuery(),
        handle,
        error, QVariant(), Qt::QueuedConnection);
    }

    QNetworkReply* deleteLibraryGroupPlotTemplate(int id, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "library/%1/template").arg(id),
        QUrlQuery(),
        handle,
        error, QVariant(), Qt::QueuedConnection);
    }

    QNetworkReply* compounudDependencies(int groupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "library/%1/objects").arg(groupId),
        QUrlQuery(), handle, error);
    }

    QNetworkReply* EditTable(int tableId, QString title, QString comment, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["title"] = title;
      data["comment"] = comment;

      return NetworkManager::Get()->PostRequest((API_V1 + "markups/data_tables/%1/update_title_comment").arg(tableId),
        data,
        handle,
        error);
    }

    QNetworkReply* GetReferenceDependencies(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "markups/etalon/%1/objects").arg(referenceId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* DeleteReferenceDependencies(int referenceId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "markups/etalon/%1/objects").arg(referenceId)
        , QUrlQuery()
        , handle
        , error);
    }

    QNetworkReply* AddNewOrModifyCompound(const QJsonObject data,
      ReplyHandleFunc handle,
      ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest(API_V1 + "library/element",
        data,
        handle,
        error);
    }

    QNetworkReply* AddExistedCompound(const QString groupName, int existedLibraryGroupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["group_name"] = groupName;
      QJsonArray array;
      array.append(existedLibraryGroupId);
      data["library_group_id"] = array;
      return NetworkManager::Get()->PostRequest((API_V1 + "library/group/compound"),
        data,
        handle,
        error);
    }

    QNetworkReply* AddExistedCoefficient(const QString groupName, int existedLibraryGroupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject data;
      data["group_name"] = groupName;
      QJsonArray array;
      array.append(existedLibraryGroupId);
      data["library_group_id"] = array;
      return NetworkManager::Get()->PostRequest((API_V1 + "library/group/coefficient"),
        data,
        handle,
        error);
    }

    QNetworkReply* AddExistedPlotTemplate(const QString groupName, int existedLibraryGroupId,
      ReplyHandleFunc handle,
      ReplyErrorFunc error)
    {
      QJsonObject data;
      data["group_name"] = groupName;
      QJsonArray array;
      array.append(existedLibraryGroupId);
      data["library_group_id"] = array;
      return NetworkManager::Get()->PostRequest((API_V1 + "library/group/template"),
        data,
        handle,
        error);
    }

    QNetworkReply* AddNewOrModifyPlotTemplate(const QJsonObject data,
      ReplyHandleFunc handle,
      ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest(API_V1 + "library/template",
        data,
        handle,
        error);
    }

    QNetworkReply* GetTemplateObjectsForPlotTemplate(int groupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + QString("library/%1/template/objects").arg(groupId)),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* SavePlot(const QJsonObject& obj, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest(API_V1 + QString("library/plot"),
        obj,
        handle,
        error);
    }

    QNetworkReply* LoadPlot(int plotId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + QString("plots/%1").arg(plotId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* getIdentificationPlotGroups(int project_id, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QUrlQuery query;
      query.addQueryItem("plot_type_id", "1");
      return NetworkManager::Get()->GetRequest(API_V1 + QString("projects/%1/plots/groups").arg(project_id),
        query,
        handle,
        error);
    }

    QNetworkReply* deleteIdentificationPlot(int plotId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest(API_V1 + QString("plots/%1/delete").arg(plotId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* uploadEntityForIdentificationPlot(int library_group_id, int tableId, QString dataType, QList<int> libraryGroupIds, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject request;
      request["key"] = dataType;
      if (!libraryGroupIds.empty())
      {
        QJsonArray ids;
        std::copy(libraryGroupIds.begin(), libraryGroupIds.end(), std::back_inserter(ids));
        request["library_element_ids"] = ids;
      }
      request["table_id"] = tableId;
      request["template_library_group_id"] = library_group_id;
      return NetworkManager::Get()->PostRequest(API_V2 + QString("library/plot/data"),
        request,
        handle,
        error);
    }

    QNetworkReply* changePlotsGroup(int projectId, QList<int> plotIds, bool newGroup, int groupId,
      QString groupTitle, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject json;
      QJsonArray ids;
      for (int id : plotIds)
      {
        ids.append(id);
      }
      json["plot_ids"] = ids;
      json["create_group"] = newGroup;
      if (newGroup)
      {
        json["group_title"] = groupTitle;
      }
      else
      {
        if (groupId > 0)
          json["plot_group_id"] = groupId;

      }
      return NetworkManager::Get()->PostRequest(
        (API_V1 + "projects/%1/plots/group").arg(projectId),
        json,
        handle,
        error);
    }

    QNetworkReply* changeGroupTitle(int projectId, int groupId, QString title, ReplyHandleFunc handle, ReplyErrorFunc error)
    {

      QJsonObject obj;
      obj["plot_group_id"] = groupId;
      obj["group_title"] = title;
      return NetworkManager::Get()->PostRequest(API_V1 + QString("projects/%1/plots/group/update").arg(projectId),
        obj,
        handle,
        error);
    }

    QNetworkReply* changePlotTitle(int plotId, QString title, QString comment, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["title"] = title;
      obj["comment"] = comment;

      return NetworkManager::Get()->PostRequest(API_V1 + QString("plots/%1/update").arg(plotId),
        obj,
        handle,
        error);
    }

    QNetworkReply* GetTimeRangeBilling(ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "billing/timerange",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* GetStatisticFromCalculationResources(const QDateTime& start, const QDateTime& end, ReplyHandleByteArray handle, ReplyErrorFunc error)
    {
      QJsonObject obj;
      obj["from_date"] = start.toString(Qt::ISODate);
      obj["to_date"] = end.toString(Qt::ISODate);
      return NetworkManager::Get()->PostRequest(API_V1 + "billing/stats/backend",
        obj,
        handle,
        error);
    }

    QNetworkReply* GetStatisticFromStorage(ReplyHandleByteArray handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest(API_V1 + "billing/stats/s3",
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* uploadPdfAttachmentToCompound(int libraryGroupId, QByteArray file, QString fileName, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QVariantMap requestFormDataFields;
      requestFormDataFields["file_name"] = fileName;
      return NetworkManager::Get()->PostRequest((API_V1 + "library/%1/file").arg(libraryGroupId),
        requestFormDataFields,
        file,
        handle,
        error);
    }

    QNetworkReply* uploadSpectrumAttachmentToCompound(int libraryGroupId, QByteArray file, QString fileName, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QVariantMap requestFormDataFields;
      requestFormDataFields["file_name"] = fileName;
      return NetworkManager::Get()->PostRequest((API_V1 + "library/%1/etalon").arg(libraryGroupId),
        requestFormDataFields,
        file,
        handle,
        error);
    }

    QNetworkReply* removePdfAttachmentFromCompound(int attachmentFileId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "library_files/%1").arg(attachmentFileId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* removeSpectrumAttachmentFromCompound(int libraryGroupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->DeleteRequest((API_V1 + "library/%1/etalon").arg(libraryGroupId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* downloadPdfAttachmentFromCompound(int libraryGroupId, ReplyHandleByteArray handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->GetRequest((API_V1 + "library/%1/file").arg(libraryGroupId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* MakeCompoundSystemic(int libraryGroupId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "library/%1/system").arg(libraryGroupId),
        QUrlQuery(),
        handle,
        error);
    }

    QNetworkReply* Identification(int markupId, const QJsonObject& json, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      return NetworkManager::Get()->PostRequest((API_V1 + "markups/%1/step-compound-search").arg(markupId)
        , json
        , handle
        , error);
    }

    QNetworkReply* ParseXlsTable(const QByteArray& fileData, const QString& fileName, int projectId, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QVariantMap info;
      info["project_id"] = projectId;
      info["file_name"] = fileName;
      return NetworkManager::Get()->PostRequest(API_V1 + "parse_xlsx",
        info,
        fileData,
        handle,
        error);
    }

    QNetworkReply* UploadParsedImportTable(QJsonObject& data, ReplyHandleFunc handle, ReplyErrorFunc error, ReplyUploadProgressFunc progress)
    {
      return NetworkManager::Get()->PostRequest(API_V2 + "import_table",
        data,
        handle,
        error);
    }

    QNetworkReply* GetXlsColumnsCombos(const QString& tableType, ReplyHandleFunc handle, ReplyErrorFunc error)
    {
      QUrlQuery params;
      params.addQueryItem("table_type", tableType);
      return NetworkManager::Get()->GetRequest(API_V1 + "import_table/passport_keys",
        params,
        handle,
        error);
    }

    QNetworkReply* BaselineData(int projectFileId, bool keysOnly, std::optional<int> smoothLvl, ReplyHandleFunc handle, ReplyErrorFunc error, Qt::ConnectionType connType)
    {
      QJsonObject details;
      details["baseline_smooth_levels_only"] = keysOnly;
      details["baseline_smooth_level"] = smoothLvl.has_value() ? QJsonValue(smoothLvl.value()) : QJsonValue();

      return NetworkManager::Get()->PostRequest((API_V1 + "project_file/%1/baseline").arg(projectFileId),
        details,
        handle,
        error,
        QVariant(),
        connType);
    }

  }
}
