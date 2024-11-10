#include "web_dialog_intermarkers_transfer.h"
#include <genesis_style/style.h>


WebDialogIntermarkersTransfer::WebDialogIntermarkersTransfer(QWidget* parent_widget, int intervals_count)
    : WebDialog(parent_widget, nullptr, QSizeF(), QDialogButtonBox::Ok|QDialogButtonBox::Cancel)
    , _parent_widget(parent_widget)
    , intervals(intervals_count)
{
    lay_intervals = 0;
    parameter = IntermarkerParameter::Index;
    SetupUi();
}

void WebDialogIntermarkersTransfer::Accept()
{
    WebDialog::Accept();
}

void WebDialogIntermarkersTransfer::Reject()
{
    WebDialog::Reject();
}

int WebDialogIntermarkersTransfer::getParameter()
{
    return parameter;
}

QVector<GenesisMarkup::IntermarkerInterval> WebDialogIntermarkersTransfer::getIntevals()
{
    QVector<GenesisMarkup::IntermarkerInterval> intervals;

    foreach (IntermarkerWidget spinbox, spinboxes)
    {
        GenesisMarkup::IntermarkerInterval interval;
        interval.start = spinbox.start->value();
        interval.end = spinbox.end->value();
        interval.window = spinbox.window->value();

        intervals.append(interval);
    }

    return intervals;
}

void WebDialogIntermarkersTransfer::SetupUi()
{
    {
        if (auto ok = ButtonBox->button(QDialogButtonBox::Ok))
          {
            ok->setDefault(true);
            ok->setText(tr("Transfer intermarkers"));
          }
    }
    ButtonLayout->setAlignment(Qt::AlignRight);

    //// Body
    Body = new QWidget(Content);

    Body->setMinimumWidth(500);

    Body->setStyleSheet(Style::Genesis::GetUiStyle());

    Content->layout()->addWidget(Body);

    BodyLayout = new QVBoxLayout(Body);
    BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
    BodyLayout->setSpacing(0);

    //// Content
    {
      ////Header
        {
            QLabel* caption = new QLabel(tr("InterMarkers transfer"), Body);
            QHBoxLayout * header = new QHBoxLayout();
            caption->setAlignment(Qt::AlignLeft);
            caption->setStyleSheet(Style::Genesis::GetH2());
            header->addWidget(caption);

            header->addSpacing(200);

            QPushButton *exit = new QPushButton(Body);
            exit->setIcon(QIcon(":/resource/icons/icon_action_cross.png"));

            exit->setMinimumSize(20,20);
            exit->setMaximumSize(20,20);

            header->addWidget(exit);

            connect(exit, &QPushButton::clicked, ButtonBox->button(QDialogButtonBox::Cancel), &QPushButton::click);
            BodyLayout->addLayout(header);
            BodyLayout->addSpacing(Style::Scale(15));

        }
        ////Body
        {

            QLabel *label = new QLabel(tr("Tranfer parameter"), Body);
            BodyLayout->addWidget(label);

            QCheckBox * checkbox_index = new QCheckBox(tr("Index"), Body);
            BodyLayout->addWidget(checkbox_index);

            checkbox_index->setChecked(true);

            QCheckBox * checkbox_values = new QCheckBox(tr("Close values"), Body);
            BodyLayout->addWidget(checkbox_values);

            checkbox_values->setChecked(false);

            BodyLayout->addSpacing(Style::Scale(15));

            connect(checkbox_index, &QCheckBox::clicked, this, [this,checkbox_index, checkbox_values]
            {
                if(checkbox_index->isChecked())
                {
                    checkbox_values->setCheckState(Qt::Unchecked);
                    parameter = IntermarkerParameter::Index;
                }
            });

            connect(checkbox_values, &QCheckBox::clicked, this, [this,checkbox_index, checkbox_values]
            {
                if(checkbox_values->isChecked())
                {
                    checkbox_index->setCheckState(Qt::Unchecked);
                    parameter = IntermarkerParameter::Value;
                }
            });

            QLabel *label_1 = new QLabel(tr("Set intervals"), Body);
            BodyLayout->addWidget(label_1);


            QLabel *empty_label = new QLabel("              ", Body);
            QLabel *start_label = new QLabel(tr("Start"), Body);
            QLabel *end_label = new QLabel(tr("End"), Body);
            QLabel *window_label = new QLabel(tr("Window"), Body);

            QHBoxLayout *labels_layout = new QHBoxLayout(Body);

            labels_layout->addWidget(empty_label, Qt::AlignmentFlag::AlignCenter);
            labels_layout->addSpacing(50);
            labels_layout->addWidget(start_label, Qt::AlignmentFlag::AlignCenter);
            labels_layout->addWidget(end_label, Qt::AlignmentFlag::AlignCenter);
            labels_layout->addWidget(window_label, Qt::AlignmentFlag::AlignCenter);

            BodyLayout->addLayout(labels_layout);

            QGridLayout *grid_layout = new QGridLayout();

            ////@todo add + icon to button
            QPushButton * add_interval = new QPushButton(tr("Add interval"));

            add_interval->setCheckable(true);

            QWidget *scroll_widget = new QWidget(Body);

            QScrollArea *scrollArea = new QScrollArea(Body);

            scrollArea->setMinimumHeight(250);

            scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);

            scroll_widget->setLayout(grid_layout);

            scrollArea->setWidget(scroll_widget);

            scrollArea->setWidgetResizable(true);

            BodyLayout->addWidget(scrollArea);

            BodyLayout->addWidget(add_interval);

            connect(add_interval, &QPushButton::clicked, this, [this, scrollArea, grid_layout]
            {
                if(lay_intervals < intervals )
                {

                    QLabel *label = new QLabel(tr("Interval ")+ QString::number(lay_intervals + 1));
                    QDoubleSpinBox* start = new QDoubleSpinBox();
                    QDoubleSpinBox* end = new QDoubleSpinBox();
                    QDoubleSpinBox* window = new QDoubleSpinBox();

                    start->setMaximum(999999);
                    start->setSingleStep(0.1);
                    start->setDecimals(1);
                    start->setStepType(QDoubleSpinBox::StepType::AdaptiveDecimalStepType);

                    end->setMaximum(999999);
                    end->setSingleStep(0.1);
                    end->setDecimals(1);
                    end->setStepType(QDoubleSpinBox::StepType::AdaptiveDecimalStepType);

                    window->setRange(0.01,100);
                    window->setStepType(QDoubleSpinBox::StepType::AdaptiveDecimalStepType);


                    window->setValue(0.5);


                    QHBoxLayout *layout = new QHBoxLayout();

                    layout->addWidget(label);
                    layout->addWidget(start);
                    layout->addWidget(end);
                    layout->addWidget(window);

                    grid_layout->addWidget(label, lay_intervals, 0);
                    grid_layout->addWidget(start, lay_intervals, 1);
                    grid_layout->addWidget(end, lay_intervals, 2);
                    grid_layout->addWidget(window, lay_intervals, 3);

                    connect(start, &QDoubleSpinBox::valueChanged, end, [end](double value)
                    {
                       end->setMinimum(value);
                    });

                    if(!spinboxes.isEmpty())
                    {
                        QDoubleSpinBox *prev_end = spinboxes.last().end;

                        connect(prev_end, &QDoubleSpinBox::valueChanged, start, [start](double value)
                        {
                            start->setMinimum(value);
                        });

                        start->setMinimum(prev_end->value());
                    }

                    lay_intervals++;

                    IntermarkerWidget widgets;
                    widgets.start = start;
                    widgets.end = end;
                    widgets.window = window;

                    spinboxes.append(widgets);
                }
            });

            add_interval->click();
        }
    }
}
