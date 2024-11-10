#include "logic/app.h"
#include "ui/genesis_window.h"
#include "logic/service/json_service_package.h"
#include "logic/service/plot_service_package.h"
#include <ui/widgets/markup/dialog_add_slave_chromatogramm.h>
#include <logic/tree_model_dynamic_project_chromatogramms.h>
#include <QTreeView>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

// #define SANDBOX
#ifdef SANDBOX
#include <ui/widgets/plot_template_widget.h>
#include <logic/models/plot_template_model.h>
// #include <genesis_style/style.h>
// #include <QPushButton>
// #include <api/api_rest.h>

int main(int argc, char *argv[])
{
  Service::ServicePackage package;
  qputenv("QT_DEBUG_PLUGINS", "1");

  //// App
  GenesisApp app(argc, argv);

// #define DIAL
#ifdef DIAL

  // QWidget root;
  // API::REST::Authorize("belyaevskiy.ao", "ba12345", [&root](QNetworkReply*, QVariantMap)
  //   {
  //     root.setStyleSheet(Style::Genesis::GetUiStyle());
  //     auto l = new QHBoxLayout(&root);
  //     root.setLayout(l);
  //     auto w = new AddCompoundWidget(&root);
  //     l->addWidget(w);
  //     l->addWidget(new AddClassifierWidget(&root));
  //     root.show();
  //   },
  //   [](QNetworkReply*, QNetworkReply::NetworkError)
  //   {
  //     qDebug() << "AUTH ERROR";
  //   });

#else
  // GenesisWindow mainWindow;
  // mainWindow.setWindowIcon(QIcon(":/resource/genesis_32.ico"));
  // mainWindow.show();
  // auto dial = new Dialogs::WebDialogAddChromatogramms({}, 0, &mainWindow);
  // auto overlayId = mainWindow.ShowOverlay("Loading");
  // dial->connect(dial, &Dialogs::WebDialogAddChromatogramms::modelsLoaded, &mainWindow, [dial, overlayId, &mainWindow]()
  //         {
  //           mainWindow.RemoveOverlay(overlayId);
  //           dial->Open();
  //         });
  // dial->loadModels();
  PlotTemplateWidget wgt;
  PlotTemplateModel model(&wgt);
  wgt.setModel(&model);
  wgt.show();
#endif



//  WebDialog dial(&mainWindow, &w, QSizeF(), QDialogButtonBox::Ok);
//  dial.Open();
  return app.exec();
#else

QScopedPointer<QFile>   m_logFile;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream out(m_logFile.data());
    out << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss ");

    switch (type)
    {
      case QtInfoMsg:     out << "INF "; break;
      case QtDebugMsg:    out << "DBG "; break;
      case QtWarningMsg:  out << "WRN "; break;
      case QtCriticalMsg: out << "CRT "; break;
      case QtFatalMsg:    out << "FTL "; break;
    }
//    out << QString("Func: %1 Line:%2 msg:\n").arg(context.function).arg(context.line);
    out << context.category << ": "<< msg << "\n";
    out.flush();
}

int main(int argc, char *argv[])
{
  Service::JsonServicePackage jsonPackages;
  Service::PlotServicePackage plotPackages;
  qputenv("QSG_RHI_BACKEND", "opengl");
//  qputenv("QT_DEBUG_PLUGINS", "1");

  //// App
  GenesisApp app(argc, argv);

  GenesisWindow mainWindow;
  mainWindow.setWindowState(Qt::WindowState::WindowMaximized);
  //mainWindow.setWindowFlag(Qt::FramelessWindowHint); // disable mainwindow toolbar
  mainWindow.setWindowIcon(QIcon(":/resource/genesis_32.ico"));

#ifndef PLATFORM_WASM
    #ifdef LOGGER_ENABLE

  QString currentPath =  QDir().currentPath();

  if(!QDir(currentPath + "/logs").exists())
  {
    QDir().mkdir(currentPath + "/logs");
  }

  QString filename = QString("%1/%2/%3.log").arg(currentPath).arg("logs").arg(QDateTime::currentDateTime().toString("yyyy-mm-dd hh-mm-ss"));

  // Try write logs to current app folder, if failed, write to APP_DATA

  QFile *file = new QFile(filename);
  if (file->open( QIODevice::ReadWrite | QIODevice::Append |  QIODevice::Text))
  {
    qDebug() << "Move logs to file: " + filename;
    m_logFile.reset(file);
    qInstallMessageHandler(messageHandler);
    mainWindow.SetLogFilename(filename);
  }
  else
  {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if(!QDir(appDataPath).exists())
    {
        QDir().mkdir(appDataPath);
    }

    if(!QDir(appDataPath + "/logs").exists())
    {
        QDir().mkdir(appDataPath + "/logs");
    }

    filename = QString("%1/%2.log").arg(appDataPath + "/logs").arg(QDateTime::currentDateTime().toString("yyyy-mm-dd hh-mm-ss"));
    QFile *file = new QFile(filename);
    if (file->open( QIODevice::ReadWrite | QIODevice::Append |  QIODevice::Text))
    {
        qDebug() << "Move logs to file: " + filename;
        m_logFile.reset(file);
        qInstallMessageHandler(messageHandler);
        mainWindow.SetLogFilename(filename);
    }
    else
    {
        qDebug() << file->errorString();
    }
  }

    #endif
#endif
  //// Ui
  mainWindow.show();

//// Exec
  return app.exec();
#endif
}
