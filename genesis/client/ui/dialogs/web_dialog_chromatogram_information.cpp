#include "web_dialog_chromatogram_information.h"

#include "../../api/api_rest.h"

#include "../../logic/notification.h"
#include "../../logic/tree_model_item.h"

#include <genesis_style/style.h>

#include <QPushButton>
#include <QLineEdit>

/////////////////////////////////////////////////////
//// Web Dialog / Chromatogram information
WebDialogChromatogramInformation::WebDialogChromatogramInformation(QWidget* parent, int fileId)
  : WebDialog(parent, nullptr, QSize(0.4, 0), QDialogButtonBox::Apply | QDialogButtonBox::Cancel)
  , FileId(fileId)
  , CreatedFieldId(0)
  , CreatedWellId(0)
  , CreatedLayerId(0)
{
  SetupModels();
  SetupUi();
}

WebDialogChromatogramInformation::~WebDialogChromatogramInformation()
{
}

void WebDialogChromatogramInformation::SetupModels()
{
  FileInfo    = new TreeModelDynamicFileInfos(this, FileId);
  Fields      = new TreeModelDynamicFields(this);
  FieldWells  = nullptr;
  Layers      = new TreeModelDynamicLayers(this);
}

void WebDialogChromatogramInformation::SetupUi()
{
  //// Body
  Body = new QWidget(Content);
  Content->layout()->addWidget(Body);

  BodyLayout = new QVBoxLayout(Body);
  BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  BodyLayout->setSpacing(0);

  //// Content
  {
    //// Header
    {
      QLabel* caption = new QLabel(tr("Geological object information"), Body);
      BodyLayout->addWidget(caption);

      caption->setAlignment(Qt::AlignCenter);
      caption->setStyleSheet(Style::Genesis::GetH1());
    }

    //// Space
    BodyLayout->addSpacing(Style::Scale(26));

    //// Input
    {
      //// Title
      {
        QLabel* caption = new QLabel(tr("Title"), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        EditTitle = new QLineEdit(Body);
        BodyLayout->addWidget(EditTitle);
      }

      BodyLayout->addSpacing(Style::Scale(16));

      //// Field
      {
        QLabel* caption = new QLabel(tr("Field %1").arg(Style::GetInputAlert()), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        ComboField = new QComboBox(Body);
        BodyLayout->addWidget(ComboField);

        ComboField->setEditable(true);
      }

      BodyLayout->addSpacing(Style::Scale(16));

      //// Well Cluster
      /*
      {
        QLabel* caption = new QLabel(tr("Well cluster %1").arg(Style::GetInputAlert()), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        ComboWellCluster = new QComboBox(Body);
        BodyLayout->addWidget(ComboWellCluster);

        ComboWellCluster->setEditable(true);
      }

      BodyLayout->addSpacing(Style::Scale(16));
      */
      //// Well
      {
        QLabel* caption = new QLabel(tr("Well %1").arg(Style::GetInputAlert()), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        ComboWell = new QComboBox(Body);
        BodyLayout->addWidget(ComboWell);

        ComboWell->setEditable(true);
      }

      BodyLayout->addSpacing(Style::Scale(16));

      //// Layer
      {
        QLabel* caption = new QLabel(tr("Layer %1").arg(Style::GetInputAlert()), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        ComboLayer = new QComboBox(Body);
        BodyLayout->addWidget(ComboLayer);

        ComboLayer->setEditable(true);
      }

      BodyLayout->addSpacing(Style::Scale(16));

      //// Datetime
      {
        QLabel* caption = new QLabel(tr("Sampling date %1").arg(Style::GetInputAlert()), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        EditDateTime = new QDateTimeEdit(Body);
        BodyLayout->addWidget(EditDateTime);

        EditDateTime->setCalendarPopup(true);
      }

      BodyLayout->addSpacing(Style::Scale(16));
    }
  }

  //// Buttons
  if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
  {
    apply->setText(tr("Save"));
    apply->setDefault(true);

    connect(apply, &QPushButton::clicked, ButtonBox, &QDialogButtonBox::accepted);
  }

  //// Connect updates
  {
    //// Connect field updates
    {
      auto updateFields = [this]()
      {
        //// Save
        QString current = ComboField->currentText();

        //// Refill
        ComboField->clear();
        for (int r = 0; r < Fields->rowCount(); r++)
        {
          ComboField->addItem(Fields->data(Fields->index(r, TreeModelDynamicFields::ColumnField), Qt::EditRole).toString(),
                              Fields->data(Fields->index(r, TreeModelDynamicFields::ColumnId), Qt::EditRole).toInt());
        }

        //// Restore
        int currentIndex = ComboField->findText(current);
        if (currentIndex != -1)
        {
          ComboField->setCurrentIndex(currentIndex);
        }
        else
        {
          ComboField->setCurrentIndex(-1);
          ComboField->setCurrentText(current);
        }
      };
      connect(Fields, &QAbstractItemModel::modelReset, updateFields);
      updateFields();
    }

    //// Connect field input
    {
      auto handleInputField = [this]()
      {
        //// Cant trust current index
        int currentIndex = ComboField->findText(ComboField->currentText());
        int currentField = 0;
        if (currentIndex != -1)
        {
          currentField = ComboField->itemData(currentIndex).toInt();
        }

        //// Handle
        if (currentField)
        {
          if (!FieldWells || FieldWells->GetFieldId() != currentField)
          {
            FieldWells = new TreeModelDynamicFieldWells(this, currentField);

            //// Connect wells here
            auto updateWells = [this]()
            {
              //// Save
              QString current = ComboWell->currentText();

              //// Refill
              ComboWell->clear();
              for (int r = 0; r < FieldWells->rowCount(); r++)
              {
                ComboWell->addItem(FieldWells->data(FieldWells->index(r, TreeModelDynamicFieldWells::ColumnFieldWell), Qt::EditRole).toString(),
                                   FieldWells->data(FieldWells->index(r, TreeModelDynamicFieldWells::ColumnId), Qt::EditRole).toInt());
              }

              //// Restore
              int currentIndex = ComboWell->findText(current);
              if (currentIndex != -1)
              {
                ComboWell->setCurrentIndex(currentIndex);
              }
              else
              {
                ComboWell->setCurrentIndex(-1);
                ComboWell->setCurrentText(current);
              }
            };
            connect(FieldWells, &QAbstractItemModel::modelReset, updateWells);
            updateWells();
          }
        }
        else
        {
          delete FieldWells;
        }
      };
      connect(ComboField, QOverload<int>::of(&QComboBox::currentIndexChanged), handleInputField);
      connect(ComboField, &QComboBox::currentTextChanged,                      handleInputField);
    }

    //// Connect layers
    {
      auto updateLayers = [this]()
      {
        //// Save
        QString current = ComboLayer->currentText();

        //// Refill
        ComboLayer->clear();
        for (int r = 0; r < Layers->rowCount(); r++)
        {
          ComboLayer->addItem(Layers->data(Layers->index(r, TreeModelDynamicLayers::ColumnLayer), Qt::EditRole).toString(),
                              Layers->data(Layers->index(r, TreeModelDynamicLayers::ColumnId),    Qt::EditRole).toInt());
        }

        //// Restore
        int currentIndex = ComboLayer->findText(current);
        if (currentIndex != -1)
        {
          ComboLayer->setCurrentIndex(currentIndex);
        }
        else
        {
          ComboLayer->setCurrentIndex(-1);
          ComboLayer->setCurrentText(current);
        }
      };
      connect(Layers, &QAbstractItemModel::modelReset, updateLayers);
      updateLayers();
    }

    //// Connect all inputs
    connect(ComboField, &QComboBox::currentTextChanged, this, &WebDialogChromatogramInformation::UpdateButtons);
    connect(ComboWell,  &QComboBox::currentTextChanged, this, &WebDialogChromatogramInformation::UpdateButtons);
    connect(ComboLayer, &QComboBox::currentTextChanged, this, &WebDialogChromatogramInformation::UpdateButtons);
    UpdateButtons();

    //// Connect initial data setup
    {
      auto updateInitial = [this]()
      {
        int currentFieldIndex = -1;
        QString currentField;

        int currentWellIndex = -1;
        QString currentWell;

        int currentLayerIndex = -1;
        QString currentLayer;

        QDateTime dateTime = QDateTime::currentDateTime();

        QString title;

        if (FileInfo->rowCount() == 1)
        {
          //// Ttitles
          currentField = FileInfo->data(FileInfo->index(0, TreeModelDynamicFileInfos::ColumnFieldTitle), Qt::EditRole).toString();
          currentFieldIndex = ComboField->findText(currentField);

          currentWell  = FileInfo->data(FileInfo->index(0, TreeModelDynamicFileInfos::ColumnWellTitle),  Qt::EditRole).toString();
          currentWellIndex = ComboWell->findText(currentWell);

          currentLayer = FileInfo->data(FileInfo->index(0, TreeModelDynamicFileInfos::ColumnLayerTitle), Qt::EditRole).toString();
          currentLayerIndex = ComboLayer->findText(currentLayer);

          //// Datetime
          dateTime = FileInfo->data(FileInfo->index(0, TreeModelDynamicFileInfos::ColumnDateTime), Qt::EditRole).toDateTime();

          //// Title
          title = FileInfo->data(FileInfo->index(0, TreeModelDynamicFileInfos::ColumnTitle), Qt::EditRole).toString();;
        }

        //// Apply
        ComboField->setCurrentIndex(currentFieldIndex);
        ComboField->setCurrentText(currentField);

        ComboWell->setCurrentIndex(currentWellIndex);
        ComboWell->setCurrentText(currentWell);

        ComboLayer->setCurrentIndex(currentLayerIndex);
        ComboLayer->setCurrentText(currentLayer);

        EditDateTime->setDateTime(dateTime);
        EditTitle->setText(title);
      };
      connect(FileInfo, &QAbstractItemModel::modelReset, updateInitial);
      updateInitial();
    }
  }
}

bool WebDialogChromatogramInformation::DependenciesSafisfied()
{
  return DependenciesSafisfiedField()
    && DependenciesSafisfiedWell()
    && DependenciesSafisfiedLayer();
}

bool WebDialogChromatogramInformation::DependenciesSafisfiedField()
{
  return GetDependencyField() != 0;
}

bool WebDialogChromatogramInformation::DependenciesSafisfiedWell()
{
  return GetDependencyWell() != 0;
}

bool WebDialogChromatogramInformation::DependenciesSafisfiedLayer()
{
  return GetDependencyLayer() != 0;
}

int WebDialogChromatogramInformation::GetDependencyField()
{
  int fieldId = ComboField->currentData().toInt();
  if (ComboField->itemText(ComboField->currentIndex())
   != ComboField->lineEdit()->text())
    fieldId = 0;
  if (!fieldId)
  {
    QString current = ComboField->lineEdit()->text();
    int ci = ComboField->findText(current);
    if (ci != -1)
    {
      fieldId = ComboField->itemData(ci).toInt();
    }
  }
  if (!fieldId)
  {
    fieldId = CreatedFieldId;
  }
  return fieldId;
}

int WebDialogChromatogramInformation::GetDependencyWell()
{
  int wellId = ComboWell->currentData().toInt();
  if (ComboWell->itemText(ComboWell->currentIndex())
   != ComboWell->lineEdit()->text())
    wellId = 0;
  if (!wellId)
  {
    QString current = ComboWell->lineEdit()->text();
    int ci = ComboWell->findText(current);
    if (ci != -1)
    {
      wellId = ComboWell->itemData(ci).toInt();
    }
  }
  if (!wellId)
  {
    wellId = CreatedWellId;
  }
  return wellId;
}

int WebDialogChromatogramInformation::GetDependencyLayer()
{
  int layerId = ComboLayer->currentData().toInt();
  if (ComboLayer->itemText(ComboLayer->currentIndex())
   != ComboLayer->lineEdit()->text())
    layerId = 0;
  if (!layerId)
  {
    QString current = ComboLayer->lineEdit()->text();
    int ci = ComboLayer->findText(current);
    if (ci != -1)
    {
      layerId = ComboLayer->itemData(ci).toInt();
    }
  }
  if (!layerId)
  {
    layerId = CreatedLayerId;
  }
  return layerId;
}

QString WebDialogChromatogramInformation::GetTitle()
{
  return EditTitle->text();
}

void WebDialogChromatogramInformation::SendRequest()
{
  //// Ids from input
  int fieldId = GetDependencyField();
  int wellId  = GetDependencyWell();
  int layerId = GetDependencyLayer();
  QString title = GetTitle();

  QDateTime dateTime = EditDateTime->dateTime();

  //// All set, add file info
  if (fieldId && wellId && layerId)
  {
    decltype(&API::REST::Tables::AddFileInfo) fn = nullptr;

    if (FileInfo->rowCount() == 0)
      fn = &API::REST::Tables::AddFileInfo;
    else
      fn = &API::REST::Tables::SetFileInfo;

    fn(FileId, wellId, layerId, dateTime, title,
      [this](QNetworkReply*, QVariantMap /*result*/)
      {
        //// Notify
        Notification::NotifySuccess(tr("Geoligical object information updated"));

        //// Done
        WebDialog::Accept();
      },
      [](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        //// Notify
        Notification::NotifyError(tr("Failed to set geoligical object information"), err);
      });
  }
}

void WebDialogChromatogramInformation::CreateDependencies()
{
  if (!DependenciesSafisfiedField())
    CreateDependencyField();
  else if (!DependenciesSafisfiedWell())
    CreateDependencyWell();
  else if (!DependenciesSafisfiedLayer())
    CreateDependencyLayer();
}

void WebDialogChromatogramInformation::CreateDependencyField()
{
  QString field = ComboField->lineEdit()->text();
  API::REST::Tables::AddField(field,
    [this](QNetworkReply*, QVariantMap result)
    {
      auto children = result["children"].toList();
      if (children.size())
      {
        if (int id = children[0].toMap()["add_field"].toInt())
        {
          //// Save
          CreatedFieldId = id;

          //// Notify
          Notification::NotifySuccess(tr("Field successfully created"));

          //// Proceed
          if (DependenciesSafisfied())
          {
            SendRequest();
          }
          else if (!DependenciesSafisfiedWell())
          {
            CreateDependencyWell();
          }
          else if (!DependenciesSafisfiedLayer())
          {
            CreateDependencyLayer();
          }
        }
      }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to create field"), err);
    });
}

void WebDialogChromatogramInformation::CreateDependencyWell()
{
  int fieldId = GetDependencyField();
  QString well = ComboWell->lineEdit()->text();
  API::REST::Tables::AddWell(fieldId, well,
    [this](QNetworkReply*, QVariantMap result)
    {
      auto children = result["children"].toList();
      if (children.size())
      {
        if (int id = children[0].toMap()["add_well"].toInt())
        {
          //// Save
          CreatedWellId = id;

          //// Notify
          Notification::NotifySuccess(tr("Well successfully created"));

          //// Proceed
          if (DependenciesSafisfied())
          {
            SendRequest();
          }
          else if (!DependenciesSafisfiedLayer())
          {
            CreateDependencyLayer();
          }
        }
      }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to create well"), err);
    });
}

void WebDialogChromatogramInformation::CreateDependencyLayer()
{
  QString layer = ComboLayer->lineEdit()->text();
  API::REST::Tables::AddLayer(layer,
    [this](QNetworkReply*, QVariantMap result)
    {
      auto children = result["children"].toList();
      if (children.size())
      {
        if (int id = children[0].toMap()["add_layer"].toInt())
        {
          //// Save
          CreatedLayerId = id;

          //// Notify
          Notification::NotifySuccess(tr("Layer successfully created"));

          //// Proceed
          if (DependenciesSafisfied())
          {
            SendRequest();
          }
        }
      }
    },
    [](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to create layer"), err);
    });
}

void WebDialogChromatogramInformation::Accept()
{
  if (DependenciesSafisfied())
  {
    SendRequest();
  }
  else
  {
    CreateDependencies();
  }
}

void WebDialogChromatogramInformation::Reject()
{
  WebDialog::Reject();
}

void WebDialogChromatogramInformation::UpdateButtons()
{
  if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
  {
    apply->setEnabled(!ComboField->currentText().isEmpty()
      && !ComboWell->currentText().isEmpty()
      && !ComboLayer->currentText().isEmpty());
  }
}
