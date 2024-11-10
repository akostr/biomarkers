#include "web_dialog_box_with_radio.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegExp>
#include <QButtonGroup>

#include <genesis_style/style.h>


WebDialogBoxWithRadio::WebDialogBoxWithRadio(QWidget *parent, const QString &captionLable)
    : WebDialog(parent, nullptr, QSizeF(), QDialogButtonBox::Ok|QDialogButtonBox::Cancel)
{

//    QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok|QDialogButtonBox::Cancel
    init(captionLable);

}

void WebDialogBoxWithRadio::init(const QString &captionLable)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_body = new QWidget(Content);
    Content->layout()->addWidget(m_body);

    Content->setStyleSheet(Style::Genesis::GetUiStyle());

    m_bodyLayout = new QVBoxLayout(m_body);

    setMinimumHeight(8);
    m_bodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
    m_bodyLayout->setSpacing(20);

    if (!captionLable.isEmpty()){
      auto lable = new QLabel;
      lable->setText(captionLable);
      m_bodyLayout->addWidget(lable);
      lable->setStyleSheet(Style::Genesis::GetH1());
    }

}



QList<QRadioButton*> WebDialogBoxWithRadio::makeRadioButtonsWithHandle(QList< QPair <QString, std::function<void ()> >> onClickedButtons,
                                                                        const QString &lableForRadioButons,
                                                                        QList<std::function<void ()>> handlesAccept,
                                                                        std::function<void ()> handleReject)
{
  QList<QRadioButton*> radioButtons;
  auto buttonGroup = new QButtonGroup(this);
  for (auto &pair: onClickedButtons){

    auto buttonName = pair.first;
    auto buttonHandle = pair.second;
    auto radioButton = new QRadioButton(buttonName);
    radioButton->setObjectName(buttonName);
    connect(radioButton, &QRadioButton::clicked, buttonHandle);

    radioButtons << radioButton;
    buttonGroup->addButton(radioButton, buttonGroup->buttons().size());
    m_hashNameButtonPtr.insert(buttonName, radioButton);
  }
  auto layout = m_bodyLayout;

  if (!lableForRadioButons.isEmpty()){
    auto lable = new QLabel;
    lable->setText(lableForRadioButons);
    m_bodyLayout->addWidget(lable);
  }


  for (auto &rBtn: radioButtons){
    m_bodyLayout->addWidget(rBtn, 0, Qt::AlignLeft);
  }
  m_bodyLayout->addStretch();

  if (handleReject){
    onReject = handleReject;
  }
  if (!handlesAccept.isEmpty()){
    skipOnAccept = true;
  }

  if (handlesAccept.size() == radioButtons.size()){
    connect(this, &WebDialogBoxWithRadio::Accepted, [buttonGroup, handlesAccept](){
        int checkedId = buttonGroup->checkedId();
        if (checkedId != -1){
            handlesAccept.at(checkedId)();
        }
        //qDebug() << buttonGroup;
        //qDebug() << checkedId;
    });
  }

  return radioButtons;
}

void WebDialogBoxWithRadio::Accept()
{
  if (skipOnAccept){
    emit Accepted();
    return;
  }
  WebDialog::Accept();
}

void WebDialogBoxWithRadio::Reject()
{
  if (onReject){
    onReject();
    return;
  }
  WebDialog::Reject();
}



QWidget *WebDialogBoxWithRadio::body()
{
    return m_body;
}

QDialogButtonBox *WebDialogBoxWithRadio::buttonBox()
{
    return ButtonBox;
}

QWidget *WebDialogBoxWithRadio::makeChromotogramRadioBox(QList<uint> gkList,
                                                         QStringList listCriterions,
                                                         const std::function<void (uint currentGK, QString criterion)> &handleCalcHotRestApi,
                                                         const std::function<void (uint currentGK)> &handleCalcHotManual)
{
    auto chromotogramRadioBox = new WebDialogBoxWithRadio();
    chromotogramRadioBox->setWindowTitle("Параметры критерия Хоттелинга");

    QString firstRadioButtonName = "Группировка МГК по критерию";
    QString secondRadioButtonName = "Ручная настройка группировки";

    auto headerContentLayout = new QHBoxLayout;
    auto label = new QLabel("Количество ГК");
    auto cBox = new QComboBox;
    for (auto &gk: gkList){
        cBox->addItem(QString::number(gk), gk);
    }

    headerContentLayout->addWidget(label);
    headerContentLayout->addSpacing(50);
    headerContentLayout->addWidget(cBox);
    headerContentLayout->addStretch();

    chromotogramRadioBox->m_bodyLayout->addLayout(headerContentLayout);
    chromotogramRadioBox->m_bodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
    chromotogramRadioBox->m_bodyLayout->setSpacing(0);

    auto calcHotSettingsWidget = new QWidget;
    auto calcHotSettingsWidgetLayout = new QHBoxLayout(calcHotSettingsWidget);

    auto cBoxGroupByCrit = new QComboBox;
    auto lableError = new QLabel;

    for (auto &criteria: listCriterions){
        cBoxGroupByCrit->addItem(criteria, criteria);
    }
    calcHotSettingsWidgetLayout->addWidget(cBoxGroupByCrit);
    calcHotSettingsWidgetLayout->addWidget(lableError);
//    calcHotSettingsWidgetLayout->addStretch();


    auto calcHot = [&, cBox, cBoxGroupByCrit, chromotogramRadioBox, handleCalcHotRestApi, calcHotSettingsWidget](){

        if (calcHotSettingsWidget){
            calcHotSettingsWidget->show();
        }

        auto okButton = chromotogramRadioBox->buttonBox()->button(QDialogButtonBox::StandardButton::Ok);
        if (!okButton){
            return;
        }
        okButton->setText("Рассчитать критерий Хотеллинга");
        okButton->setDisabled(false);

        if (!handleCalcHotRestApi){
            return;
        }
        okButton->disconnect();

        auto onClicked = [cBox, cBoxGroupByCrit, handleCalcHotRestApi, chromotogramRadioBox](){

            handleCalcHotRestApi(cBox->currentData().toUInt(), cBoxGroupByCrit->currentData().toString());
            chromotogramRadioBox->Accept();

        };


        connect(okButton, &QPushButton::clicked, onClicked);
    };


    auto manualHotSettings = [chromotogramRadioBox, calcHotSettingsWidget, handleCalcHotManual, cBox](){

        if (calcHotSettingsWidget){
            calcHotSettingsWidget->hide();
        }
        auto okButton = chromotogramRadioBox->buttonBox()->button(QDialogButtonBox::StandardButton::Ok);
        if (!okButton){
            return;
        }
        okButton->disconnect();
        okButton->setText("Перейти к настройке группирови");
        okButton->setDisabled(false);

        if (!handleCalcHotManual){
            return;
        }
        connect(okButton, &QPushButton::clicked, [handleCalcHotManual, chromotogramRadioBox, cBox](){
            handleCalcHotManual(cBox->currentData().toUInt());
            chromotogramRadioBox->Accept();
        });

    };

    auto rButtons = chromotogramRadioBox->makeRadioButtonsWithHandle({
                                          { firstRadioButtonName, calcHot},
                                          { secondRadioButtonName, manualHotSettings}
                                      });
    for (auto &i: rButtons){
        chromotogramRadioBox->m_bodyLayout->addWidget(i);
        chromotogramRadioBox->m_bodyLayout->addStretch();
        if (i->text() == firstRadioButtonName){
            chromotogramRadioBox->m_body->layout()->addWidget(calcHotSettingsWidget);
        }
    }
//    chromotogramRadioBox->m_body->layout()->addWidget(calcHotSettingsWidget);
    chromotogramRadioBox->m_bodyLayout->addStretch();
    calcHot();

    auto okButton = chromotogramRadioBox->buttonBox()->button(QDialogButtonBox::StandardButton::Ok);
    if (okButton){
        okButton->setDisabled(true);
    }

//    testD->show();
    return chromotogramRadioBox;
}



