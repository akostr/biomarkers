#include "web_dialog_file_removing.h"
#include "ui_web_dialog_file_removing.h"
#include <genesis_style/style.h>
#include <api/api_rest.h>

#include <QLayout>
#include <QPushButton>

WebDialogFileRemovingConfirmation::WebDialogFileRemovingConfirmation(const QList<QPair<int, QString>> &projectFilesInfos, QWidget *parent) :
  WebDialog(parent, nullptr, QSizeF(0, 0), Btns::Ok | Btns::Cancel),
  ui(new Ui::WebDialogFileRemoving),
  m_projectFilesInfos(projectFilesInfos)
{
  setupUi();
}

WebDialogFileRemovingConfirmation::~WebDialogFileRemovingConfirmation()
{
  delete ui;
}

void WebDialogFileRemovingConfirmation::setupUi()
{
  auto body = new QWidget();
  ui->setupUi(body);
  Content->layout()->addWidget(body);
  body->setStyleSheet(Style::Genesis::GetUiStyle());
  ui->caption->setStyleSheet(Style::Genesis::GetH1());
  ui->warningTextLabel->setStyleSheet(Style::Genesis::GetH3());

  ButtonBox->button(Btns::Ok)->setText(tr("Remove"));

  ui->listLabel->setText("loading...");
  m_remainingQueries = m_projectFilesInfos.count();
  for(auto& fileInfo : m_projectFilesInfos)
  {
    API::REST::GetDependentFiles(fileInfo.first,
    [this, fileInfo](QNetworkReply*, QJsonDocument doc)
    {
      QString info;
      info += fileInfo.second;
      auto root = doc.object();
      if(root.contains("error"))
      {
        qDebug() << "error" << root["msg"].toString();
        queryComplete();
        return;
      }

      auto columns = root["columns"].toArray();
      auto data = root["data"].toArray();

      for(int i = 0; i < data.size(); i++)
      {
        QString tabSym;
        if(i < data.size() - 1)
          tabSym = " ├";
        else
          tabSym = " └";
        auto item = data[i].toArray();
        if(item.first().toString() == "markup")
          info += "\n" + tabSym + tr("Markup") + " " + item.last().toString();
        else if(item.first().toString() == "analysis")
          info += "\n" + tabSym + tr("Analysis") + " " + item.last().toString();
        else
          info += "\n" + tabSym + item.first().toString() + " " + item.last().toString();
      }


      m_fileIdToInfoMap[fileInfo.first] = info;
      queryComplete();
    },
    [this](QNetworkReply* r, QNetworkReply::NetworkError e)
    {
      qDebug() << "error" << r->errorString();
      queryComplete();
    });
  }
}

void WebDialogFileRemovingConfirmation::queryComplete()
{
  if(--m_remainingQueries <= 0)
  {
    if(m_fileIdToInfoMap.isEmpty())
    {
      ui->listLabel->setText(tr("No dependencies"));
    }
    else
    {
      QString labelText;
      for(const auto& info : m_fileIdToInfoMap)
      {
        labelText.append(info);
        labelText.append('\n');
      }
      ui->listLabel->setText(labelText);
    }
  }
}
