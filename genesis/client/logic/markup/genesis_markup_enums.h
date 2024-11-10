#ifndef GENESIS_MARKUP_ENUMS_H
#define GENESIS_MARKUP_ENUMS_H
#include "logic/markup/curve_data_model.h"
#include "logic/markup/abstract_data_model.h"
#include "qcolor.h"
#include "qpoint.h"
#include <QMetaObject>

namespace GenesisMarkup{
class BaseLineDataModel;
Q_NAMESPACE
////!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// !!изменение имен членов енумов приведет к потере совместимости,!!
/// !!поскольку они напрямую через QMetaEnum используются для сох -!!
/// !!ранения и загрузки данных, что бы эти данные были хорошо чи -!!
/// !!таемы в жисончиках.                                          !!
/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

using BaseLineDataModelPtr = QSharedPointer<BaseLineDataModel>;

using TEntityType = uint32_t;
using TPeakStart = QPointF;
using TPeakEnd = QPointF;
using TPeakTop = QPointF;
using TPeakBottom = QPointF;
using TPeakRetentionTime = double;
using TPeakCovatsIndex = double;
using TPeakMarkerWindow = double;
using TPeakArea = double;
using TPeakHeight = double;
using TPeakFormFactor = double;
using TPeakTitle = QString;
using TPeakId = int;
using TPeakType = uint;
using TPeakColor = QColor;
using TBaseLineDataModel = BaseLineDataModelPtr;
using TIntervalSettings = uint; //stub
using TIntervalLeft = double;
using TIntervalRight = double;
using TIntervalType = uint;
using TCurveData = CurveDataModelPtr;
using TPassportTitle = QString;
using TPassportFilename = QString;
using TPassportData = QDateTime;
using TPassportWell = QString;
using TPassportField = QString;
using TPassportLayer = QString;
using TPassportFileId = int;
using TChromatogrammIdRole = int;
using TChromatogrammCutPoint = double;
using TMarkupMasterIdRole = int;
using TKeySelectorKey = double;

enum PeakTypes
{
  PTNone = 0,   //0
  PTPeak,       //1
  PTMarker,     //2
  PTInterMarker,//3
  PTFake        //4
};
Q_ENUM_NS(PeakTypes)

enum IntervalTypes
{
  ITNone = 0,   //0
  ITCustomParameters,       //1
  ITMarkupDeprecation,     //2
};
Q_ENUM_NS(IntervalTypes)

enum ChromatogrammModes
{
  CMNone = 0,   //0
  CMMarkup,       //1
  CMIntervals,     //2
};
Q_ENUM_NS(ChromatogrammModes)

enum ChromatogrammEntityDataRoles
{
  //Peak
  PeakStart = AbstractEntityDataModel::EntityDataRoles::RoleEntityLast + 1, //3 pointf
  PeakEnd,                        //4 pointf
  PeakTop,                        //5 pointf
  PeakBottom,                     //6 pointf
  PeakRetentionTime,              //7 double
  PeakCovatsIndex,                //8 double
  PeakMarkerWindow,               //9?double?
  PeakArea,                       //10 double
  PeakHeight,                     //11 double
  PeakFormFactor,                 //12 double
  PeakTitle,                      //13 string
  PeakId,                         //14 signed int server id
  PeakType,                       //15 id type
  PeakColor,                      //16 QColor
  //BaseLine
  BaseLineData,                   //17 SharedPointer of BaseLineDataModel
  //Interval
  IntervalSettings,               //18?some parameters, maybe QVariantMap or something else
  IntervalLeft,                   //19 left x-coordinate
  IntervalRight,                  //20 right x-coordinate
  IntervalType,                   //21 id interval type
  //Curve
  CurveData,                      //22 SharedPointer to CurveDataModel

  //More interval
  Coelution,                      //23 int
  DegreeOfApproximation,          //24 int
  BoundsByMedian,                 //25 bool
  MinimalPeakHeight,              //26 double
  Noisy,                          //27 bool
  SignalToMedianRatio,            //28 double
  ViewSmooth,                     //29 bool
  BaseLineSmoothLvl,              //30 int
  MedianFilterWindowSize,         //31 int

  IntervalIsApplied,              //32

  KeySelectorKey,                 //33
  PeakLibraryGroupId,             //34
  PeakCompoundTitle,             //35

  ChromatogrammEntityLast         //35
};
Q_ENUM_NS(ChromatogrammEntityDataRoles)

enum PassportDataRoles
{
  PassportTitle = ChromatogrammEntityLast + 1,    //34 string
  PassportFilename,                               //35 string
  PassportDate,                                   //36 dateTime
  PassportWell,                                   //37?string
  PassportField,                                  //38?string
  PassportLayer,                                  //39?string
  PassportFileId,                                 //40?int
  PassportLast                                    //41
};
Q_ENUM_NS(PassportDataRoles)

enum ChromatogrammTitlesDisplayModes
{
  TitleModeDefault = 0,
  TitleModeOrder,
  TitleModeIndex,
  TitleModeRetentionTime,
  TitleModeCompoundTitle,
};
Q_ENUM_NS(ChromatogrammTitlesDisplayModes)
using TChromatogrammTitlesDisplayMode = ChromatogrammTitlesDisplayModes;

enum ChromatogrammDataRoles
{
  ChromatogrammIdRole = PassportLast + 1,     //42
  ChromatogrammCutPoint,                      //43
  ChromatogrammSettingsBaseLineSmoothLvl,     //44 int
  ChromatogrammSettingsMinimalPeakHeight,     //45 double
  ChromatogrammSettingsNoisy,                 //46 bool
  ChromatogrammSettingsMedianFilterWindowSize,//47 int
  ChromatogrammSettingsSignalToMedianRatio,   //48 double
  ChromatogrammSettingsDegreeOfApproximation, //49 int
  ChromatogrammSettingsBoundsByMedian,        //50 bool
  ChromatogrammSettingsCoelution,             //51 int
  ChromatogrammSettingsViewSmooth,            //52 bool
  ChromatogrammMode,                          //53 int (markup/intervals mode)
  ChromatogrammReferenceId,                   //54
  ChromatogrammLast                           //55
};
Q_ENUM_NS(ChromatogrammDataRoles)

enum MarkupDataRoles
{
  //there is a case, when MarkupMasterIdRole is valid, but
  //this chromatogramm is missed in model, because it is a
  //copy of original model without copiyng master.
  //in that case you should use ONLY getMaster() to get
  //proper pointer to proper master, wich uses MarkupExternalMasterPtr
  MarkupMasterIdRole = ChromatogrammLast + 1, //56
//  MarkupReferenceIdRole,                    //57
  MarkupValuablePeaksSortedList,              //58
  MarkupExternalMasterPtr,                    //59
  MarkupLastTableId,                          //60
  MarkupIdentificationData,                   //61
  MarkupLast                                  //62
};
Q_ENUM_NS(MarkupDataRoles)

enum MarkupEntityTypes
{
  TypePeak,     //0
  TypeInterval, //1
  TypeCurve,    //2
  TypeBaseline, //3
  TypeKeySelector//4
};
Q_ENUM_NS(MarkupEntityTypes)

enum PassportEntityTypes
{
  DataPassport  //0
};
Q_ENUM_NS(PassportEntityTypes)

enum Steps
{
  /*0*/Step0DataLoad,                 // loadChromotogramData (load all states from db) (curves shown): time to detect peaks ... (old name:PeakDetection)
  /*1*/Step1PeakDetection,            // chromo curve taken, sended to db, detected peaks return, (detected peaks & baseline shown): time to choose master
  /*2*/Step2MasterMarkersMarkup,      // kovats index, pick 3 markers or more (2 min)
  /*3*/Step3MasterInterMarkersMarkup,
  /*4*/Step4SlavePeaksEditing,
  /*5*/Step5MarkersTransferToSlave,
  /*6*/Step6InterMarkersTransferToSlave,
  /*6*/Step6Identification,
  /*7*/Step7Complete,
  /*8*/StepEnumLast
};
Q_ENUM_NS(Steps)

enum StepSlaveInteractions
{
  //SI  = step interactions
  //SIS = StepInteractions Slave
  SISNone                       = 0,
  SISCutTail                    = 0x1,
  SISAddRemoveBaseLinePoint     = SISCutTail<<1,
  SISBaseLinePointInteractions  = SISCutTail<<2,
  SISAddRemoveMarker            = SISCutTail<<3,
  SISAddRemoveInterMarker       = SISCutTail<<4,
  SISAddRemovePeak              = SISCutTail<<5,
  SISSetUnsetMarker             = SISCutTail<<6,
  SISSetUnsetInterMarker        = SISCutTail<<7,
  SISExportAsImage              = SISCutTail<<8,
  SISModifyPeaks                = SISCutTail<<9,
  SISModifyMarkers              = SISCutTail<<10,
  SISModifyInterMarkers         = SISCutTail<<11,
  SISSetupParameters            = SISCutTail<<12,
  SISAddRemoveInterval          = SISCutTail<<13,
  SISModifyParamIntervals       = SISCutTail<<14,
  SISModifyMarkupIntervals      = SISCutTail<<15,
  SISEnabledStyle               = SISCutTail<<16,

  SISLastPossibleFlag            = SISCutTail<<31,
  //no more rooms in that enum

};

Q_DECLARE_FLAGS(StepSlaveInteractionsFlags, StepSlaveInteractions)

enum StepMasterInteractions
{
  //SIM = StepInteractions Master
  SIMNone                       = 0,
  SIMCutTail                    = 0x1,
  SIMAddRemoveBaseLinePoint     = SIMCutTail<<1,
  SIMBaseLinePointInteractions  = SIMCutTail<<2,
  SIMAddRemoveMarker            = SIMCutTail<<3,
  SIMAddRemoveInterMarker       = SIMCutTail<<4,
  SIMAddRemovePeak              = SIMCutTail<<5,
  SIMSetUnsetMarker             = SIMCutTail<<6,
  SIMSetUnsetInterMarker        = SIMCutTail<<7,
  SIMExportAsImage              = SIMCutTail<<8,
  SIMModifyPeaks                = SIMCutTail<<9,
  SIMModifyMarkers              = SIMCutTail<<10,
  SIMModifyInterMarkers         = SIMCutTail<<11,
  SIMSetupParameters            = SIMCutTail<<12,
  SIMAddRemoveInterval         = SISCutTail<<13,
  SIMModifyParamIntervals       = SISCutTail<<14,
  SIMModifyMarkupIntervals      = SISCutTail<<15,
  SIMEnabledStyle               = SISCutTail<<16,
  // SIMKeySelectorInteractions    = SISCutTail<<17,

  SIMLastPossibleFlag            = SIMCutTail<<31,
};

Q_DECLARE_FLAGS(StepMasterInteractionsFlags, StepMasterInteractions)

enum StepGuiInteractions
{
  //SIG = StepInteractions GUI
  SIGPickMaster                  = 0x1,
  SIGImportChromatogramms        = SIGPickMaster<<1,
  SIGViewPassport                = SIGPickMaster<<2,
  SIGMarkersTable                = SIGPickMaster<<3,
  SIGSaveMaster                  = SIGPickMaster<<4,
  SIGMatrix                      = SIGPickMaster<<5,//матрица соотношений
  SIGEditNameAndComment          = SIGPickMaster<<6,
  SIGEditCopy                    = SIGPickMaster<<7,
  SIGDetectPeaks                 = SIGPickMaster<<8,
  SIGRepeatMarkerTransfer        = SIGPickMaster<<9,
  SIGDigitalDataTable            = SIGPickMaster<<11,
  SIGPrevStep                    = SIGPickMaster<<12,
  SIGNextStep                    = SIGPickMaster<<13,
  SIGComplete                    = SIGPickMaster<<14,
  SIGRepeatInterMarkerTransfer   = SIGPickMaster<<15,
  SIGSwitchToTable               = SIGPickMaster<<16,
  SIGRemarkupSlaves              = SIGPickMaster<<17,
  SIGSaveMarkup                  = SIGPickMaster<<18,
  SIGSaveMarkupAs                = SIGPickMaster<<19,
  SIGSetIntervals                = SIGPickMaster<<20,
  SIGAbortIntervals              = SIGPickMaster<<21,
  SIGRemoveSlaveChromatogramm    = SIGPickMaster<<22,
  SIGRemoveMasterChromatogramm   = SIGPickMaster<<23,
  SIGIdentification              = SIGPickMaster<<24,

  SIGLastPossibleFlag            = SIGPickMaster<<31,
};

Q_DECLARE_FLAGS(StepGuiInteractionsFlags, StepGuiInteractions)

struct StepInfo
{
  QString stepName;
  QString toolTip;
  int stepNumber;
  StepSlaveInteractionsFlags slaveInteractions;
  StepMasterInteractionsFlags masterInteractions;
  StepGuiInteractionsFlags guiInteractions;
  Steps step;
};
}//namespace GenesisMarkup
#endif // GENESIS_MARKUP_ENUMS_H
