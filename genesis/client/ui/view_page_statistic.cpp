#include "view_page_statistic.h"

#include "ui_view_page_statistic.h"

#include "genesis_style/style.h"
#include "api/api_rest.h"
#include "logic/known_json_tag_names.h"
#include "logic/known_context_tag_names.h"
#include "ui/known_view_names.h"
#include "logic/notification.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

#include <QRegExp>
#include <functional>

namespace Views
{
  ViewPageStatistic::ViewPageStatistic(QWidget* parent)
  {
    SetupUi();
    ConnectSignals();
  }

  void ViewPageStatistic::ApplyContextUi(const QString& name, const QVariant& data)
  {
    // TODO check why name and data income from second call;
    /*if(name != Names::ContextTagNames::Page || data.toString() != ViewPageNames::ViewPageStatisticName)
      return;*/

    API::REST::GetTimeRangeBilling(
      [&](QNetworkReply*, QJsonDocument doc)
      {
        if (const auto object = doc.object();
          object.value(JsonTagNames::Error).toBool())
        {
          Notification::NotifyError(tr("Error : %1").arg(object.value(JsonTagNames::Msg).toString()));
        }
        else
        {
          const auto min = QDateTime::fromString(object.value(JsonTagNames::min_time).toString(), Qt::ISODate);
          const auto max = QDateTime::fromString(object.value(JsonTagNames::max_time).toString(), Qt::ISODate);
          mUi->fromDate->setDateTimeRange(min, max);
          mUi->toDate->setDateTimeRange(min, max);
        }
      },
      [](QNetworkReply*, QNetworkReply::NetworkError)
      {
        Notification::NotifyError(tr("Error while receive data from server."));
      });
  }

  void ViewPageStatistic::SetupUi()
  {
    setStyleSheet(Style::Genesis::GetUiStyle());
    mUi = new Ui::ViewPageStatistic();
    mUi->setupUi(this);
  }

  void ViewPageStatistic::ConnectSignals()
  {
    connect(mUi->downloadButton, &QPushButton::clicked, this, &ViewPageStatistic::downloadClicked);
    connect(mUi->downloadFromStorageButton, &QPushButton::clicked, this, &ViewPageStatistic::downloadFromStorageClicked);
  }

  void ViewPageStatistic::downloadClicked()
  {
    API::REST::GetStatisticFromCalculationResources(
      mUi->fromDate->dateTime(), mUi->toDate->dateTime(),
      [&](QNetworkReply* reply, QByteArray fileArray) { saveFileFromBackend(reply, fileArray); },
      [](QNetworkReply*, QNetworkReply::NetworkError)
      {
        Notification::NotifyError(tr("Error while receive data from server."));
      });
  }

  void ViewPageStatistic::downloadFromStorageClicked()
  {
    API::REST::GetStatisticFromStorage(
      [&](QNetworkReply* reply, QByteArray fileArray) { saveFileFromBackend(reply, fileArray); },
      [](QNetworkReply*, QNetworkReply::NetworkError)
      {
        Notification::NotifyError(tr("Error while receive data from server."));
      });
  }

  void ViewPageStatistic::saveFileFromBackend(QNetworkReply* reply, QByteArray fileArray)
  {
    auto fileName = QString::fromUtf8(reply->rawHeader("file-title"));
    QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
    int pos = 0;
    while ((pos = rx.indexIn(fileName, pos)) != -1) {
      fileName.replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
    }
    fileName.replace("\"", "");

    auto fileExtension = QString::fromUtf8(reply->rawHeader("file-extension"));
    fileExtension.replace("\"", "");
    auto path = QFileDialog::getSaveFileName(nullptr,
      tr("Save file"), fileName,
      fileExtension, nullptr, QFileDialog::ReadOnly);

    path = QDir::toNativeSeparators(path);
    if (path.isEmpty())
      return;

    QFile file(path + "." + fileExtension);
    if (file.open(QFile::WriteOnly))
    {
      file.write(fileArray);
      file.close();
    }
    else
    {
      Notification::NotifyError(tr("Can't open file: %1").arg(fileName));
    }
  }
}