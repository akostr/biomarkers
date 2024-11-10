#include "naming_pattern_edit_dialog.h"
#include "ui_naming_pattern_edit_dialog.h"
#include "ui/dialogs/web_overlay.h"
#include "ui/dialogs/templates/confirm.h"
#include "genesis_style/style.h"
#include "api/api_rest.h"
#include "logic/context_root.h"
#include "logic/notification.h"
#include <logic/known_context_tag_names.h>

#include <QListWidget>
#include <QPushButton>
#include <QToolTip>
#include <QAction>

QMap<QString, QString> NamingPatternEditDialog::tagSampleMatch = {{"@lab_id#", tr("Laboratory_id")},
                                                                  {"@geo_id#", tr("Geological_id")},
                                                                  {"@field_title#", tr("Field")},
                                                                  {"@layer_title#", tr("Layer")},
                                                                  {"@well_cluster_title#", tr("Well_cluster")},
                                                                  {"@well_title#", tr("Well")},
                                                                  {"@date#", tr("Date")},
                                                                  {"@depth#", tr("Depth")},
                                                                  {"@comment#", tr("Comment")}};

NamingPatternEditDialog::NamingPatternEditDialog(QWidget *parent) :
  Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
  ui(new Ui::NamingPatternEditDialog),
  mContent(new QWidget(nullptr)),
  mTagRegularExpression(new QRegularExpression("@.*?#"))
{
  setupUi();
  loadCurrentPattern();
  Size = QSizeF(0.5, 0.3);
  UpdateGeometry();
}

NamingPatternEditDialog::~NamingPatternEditDialog()
{
  delete ui;
}

void NamingPatternEditDialog::Accept()
{
  using B = QDialogButtonBox::StandardButton;
  Dialogs::Templates::Confirm::Settings s;
  s.dialogHeader = tr("Warning");
  s.buttonsNames[B::Ok] = tr("Continue");
  s.buttonsProperties[B::Ok] = {{"blue", true}};
  auto content = new QLabel(tr("Names using the new template will be automatically updated in markups and tables.\nTo update names in mathematical models, you need to rebuild the model."));
  content->setWordWrap(true);
  auto dial = new Dialogs::Templates::Confirm(this, s, B::Ok | B::Cancel, content);
  connect(dial, &WebDialog::Accepted, this, &NamingPatternEditDialog::applyChangesAndAccept);
  dial->Open();
}

void NamingPatternEditDialog::updateSample(const QString &pattern)
{
  ui->chromaSamplePlainTextEdit->clear();
  QString sample = pattern;
  auto okBtn = ButtonBox->button(QDialogButtonBox::Ok);
  if(pattern.isEmpty())
  {
    ui->chromaSamplePlainTextEdit->appendHtml(mFormattingErrorFontOpenTag + tr("At least 1 characteristic must be added") + mFormattingFontCloseTag);
    okBtn->setEnabled(false);
    return;
  }
  if(!okBtn->isEnabled())
    okBtn->setEnabled(true);

  auto match = mTagRegularExpression->match(sample);
  while(match.hasMatch())
  {
    auto captured = match.captured(0);
    int offset;
    if(tagSampleMatch.contains(captured))
    {
      auto insertion = mFormattingInsertPattern.arg(tagSampleMatch[captured]);
      sample.remove(match.capturedStart(0), match.capturedLength(0));
      sample.insert(match.capturedStart(0), insertion);
      offset = match.capturedStart(0) + insertion.size();
    }
    else
    {
      sample.insert(match.capturedEnd(0), mFormattingFontCloseTag);
      sample.insert(match.capturedStart(0), mFormattingErrorFontOpenTag);

      offset = match.capturedEnd(0) + mFormattingErrorFontOpenTag.size() + mFormattingFontCloseTag.size();
    }
    match = mTagRegularExpression->match(sample, offset);
  }

  ui->chromaSamplePlainTextEdit->appendHtml(mFormattingFontOpenTag + sample + mFormattingFontCloseTag);
}

void NamingPatternEditDialog::applyChangesAndAccept()
{
  auto overlay = new WebOverlay(tr("Loading"), this);
  API::REST::saveProjectFileNameTemplate(Core::GenesisContextRoot::Get()->ProjectId(),
    ui->MDRadioButton->isChecked() ? "MD" : "TVD", ui->chromaTemplateLineEdit->text(),
    [this, overlay](QNetworkReply*, QJsonDocument doc)
    {
      auto root = doc.object();
      if(root["error"].toBool())
      {
        Notification::NotifyError(root["msg"].toString(), tr("Server error"));
        return;
      }
      Notification::NotifySuccess(tr("Project file name template saved"));
      delete overlay;
      WebDialog::Accept();
    },
    [overlay](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError(tr("Network error"), e);
      delete overlay;
    });
}

void NamingPatternEditDialog::setupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Project base settings");
  s.buttonsNames = {{QDialogButtonBox::Ok, tr("Apply")},
                    {QDialogButtonBox::Cancel, tr("Cancel")} };
  applySettings(s);
  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);

  mContent->setContentsMargins(2,0,12,0);
  ui->depthTypeLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  ui->chromaTemplateHeaderLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());

  QColor fontColor = Style::GetSASSColor("fontColorSecondary");
  QColor errorFontColor = QColor(235,87,87);
  QColor tagColor = Style::GetSASSColor("brandColor");
  mFormattingFontOpenTag = QString("<font color=%1>").arg(fontColor.name());
  mFormattingErrorFontOpenTag = QString("<font color=%1>").arg(errorFontColor.name());
  mFormattingFontCloseTag = QString("</font>");
  mFormattingInsertPattern = QString("<font color=%1>%2</font>").arg(tagColor.name());

  QFont font(Style::GetSASSValue("fontFaceNormal"));
  font.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toInt());
  ui->chromaSamplePlainTextEdit->setFont(font);

  mTagPopupWidget = new QWidget(mContent);
  mTagPopupWidget->installEventFilter(this);

  auto popupLayout = new QVBoxLayout(mTagPopupWidget);
  auto listView = new QListWidget;
  popupLayout->addWidget(listView);
  listView->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
  listView->setSpacing(0);
  listView->setStyleSheet(
    "QListWidget::item {padding-top: 5px; padding-bottom: 5px; margin: 0px; border: none;}"
    "QListWidget::item:hover {background-color: rgb(245, 247, 249);}"
    );

  auto appendTag = [listView](const QString& name, const QString& tag)
  {
    auto item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, tag);
    listView->addItem(item);
  };

  appendTag(tr("Laboratory id"), "@lab_id#");
  appendTag(tr("Geological id"), "@geo_id#");
  appendTag(tr("Field"), "@field_title#");
  appendTag(tr("Layer"), "@layer_title#");
  appendTag(tr("Well cluster"), "@well_cluster_title#");
  appendTag(tr("Well"), "@well_title#");
  appendTag(tr("Date"), "@date#");
  appendTag(tr("Depth"), "@depth#");
  appendTag(tr("Comment"), "@comment#");

  connect(listView, &QListWidget::itemClicked, this,
          [this](QListWidgetItem* item)
          {
            auto tag = item->data(Qt::UserRole).toString();
            ui->chromaTemplateLineEdit->insert(tag);
            mTagPopupWidget->hide();
            ui->chromaTemplateLineEdit->setFocus();
          });

  mTagPopupWidget->resize(mTagPopupWidget->width() + 2 * listView->style()->pixelMetric(QStyle::PM_ScrollBarExtent), listView->viewport()->height());
  mTagPopupWidget->hide();

  auto addTagAction = new QAction(QIcon(":/resource/icons/icon_action_add.png"),"",ui->chromaTemplateLineEdit);
  connect(addTagAction, &QAction::triggered, this,
          [this]()
          {
            auto geom = mTagPopupWidget->geometry();
            auto leGeom = ui->chromaTemplateLineEdit->geometry();
            geom.moveBottomRight(leGeom.bottomRight());
            if(geom.top() < 0)
              geom.setTop(0);
            mTagPopupWidget->setGeometry(geom);
            mTagPopupWidget->show();
          });

  ui->chromaTemplateLineEdit->addAction(addTagAction, QLineEdit::TrailingPosition);

  auto t = ui->depthTypeLabel->text();
  ui->depthTypeLabel->setText(t + "<a href=\"#\"><img src=\":/resource/icons/icon_action_information.png\" width=\"14\" height=\"14\"></a>");
  t = ui->chromaTemplateHeaderLabel->text();
  ui->chromaTemplateHeaderLabel->setText(t + "<a href=\"#\"><img src=\":/resource/icons/icon_action_information.png\" width=\"14\" height=\"14\"></a>");

  connect(ui->depthTypeLabel, &QLabel::linkHovered, this,
          [this]()
          {
              QToolTip::showText(QCursor::pos(),
                                 tr("The depth value of the selected type will be\n"
                                    "displayed in the project in the \"Depth\" column"),
                                 ui->depthTypeLabel);
          });
  connect(ui->chromaTemplateHeaderLabel, &QLabel::linkHovered, this,
          [this]()
          {
            QToolTip::showText(QCursor::pos(),
                               tr("The name based on the selected template will be\n"
                                  "displayed in markup, in the data table,\n"
                                  "in mathematical models, and when viewing\n"
                                  "chromatogram and fragment graphs. For fragments,\n"
                                  "the m/z value will be additionally displayed in the name"),
                               ui->depthTypeLabel);
          });

  connect(ui->chromaTemplateLineEdit, &QLineEdit::textChanged, this, &NamingPatternEditDialog::updateSample);
  updateSample(ui->chromaTemplateLineEdit->text());
}

void NamingPatternEditDialog::loadCurrentPattern()
{
  auto overlay = new WebOverlay(tr("Loading"), this);
  API::REST::getProjectFileNameTemplate(Core::GenesisContextRoot::Get()->ProjectId(),
    [this, overlay](QNetworkReply*, QJsonDocument doc)
    {
      auto root = doc.object();
      if(root["error"].toBool())
      {
        Notification::NotifyError(root["msg"].toString(), tr("Server error"));
        return;
      }
      ui->chromaTemplateLineEdit->setText(root["sample"].toString());
      if(root["depth_type"].toString() == "MD")
        ui->MDRadioButton->setChecked(true);
      else
        ui->TVDRadioButton->setChecked(true);
      delete overlay;
    },
    [overlay](QNetworkReply*, QNetworkReply::NetworkError e)
    {
      Notification::NotifyError(tr("Network error"), e);
      delete overlay;
    });
}


bool NamingPatternEditDialog::eventFilter(QObject *watched, QEvent *event)
{
  if(watched == mTagPopupWidget)
    if(event->type() == QEvent::Leave)
      mTagPopupWidget->hide();
  return false;
}
