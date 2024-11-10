#include "json_service_package.h"

#include "service_locator.h"
#include "ijson_serializer.h"
#include "ijson_combiner.h"
#include "logic/serializers/json_combiner.h"
#include "logic/serializers/pls_fit_response_json_serializer.h"
#include "logic/models/pls_fit_response_model.h"
#include "logic/serializers/pls_fit_request_json_serializer.h"
#include "logic/models/pls_fit_request_model.h"
#include "logic/models/pls_analysis_list_model.h"
#include "logic/models/pls_analysis_identity_model.h"
#include "logic/models/pls_predict_response_model.h"
#include "logic/models/pls_predict_analysis_model.h"
#include "logic/serializers/pls_analysis_list_json_serializer.h"
#include "logic/serializers/pls_analysis_identity_model_json_serializer.h"
#include "logic/serializers/pls_predict_response_json_serializer.h"
#include "logic/serializers/pls_predict_analysis_json_serializer.h"
#include "logic/models/pls_predict_parent_analysis_list_model.h"
#include "logic/serializers/pls_predict_parent_analysis_list_json_serializer.h"
#include "logic/serializers/mcr_analysis_request_model_json_serializer.h"
#include "logic/models/mcr_analysis_request_model.h"
#include "logic/serializers/mcr_analysis_response_model_json_serializer.h"
#include "logic/models/mcr_analysis_response_model.h"
#include "logic/models/markup_reference_model.h"
#include "logic/serializers/markup_reference_model_json_serializer.h"
#include "logic/models/mcr_analysis_identity_model.h"
#include "logic/models/reference_analysis_data_table_model.h"
#include "logic/models/pca_analysis_request_model.h"
#include "logic/models/pca_analysis_response_model.h"
#include "logic/models/hotteling_request_model.h"
#include "logic/models/hotteling_response_model.h"
#include "logic/models/pca_analysis_identity_model.h"
#include "logic/models/coefficient_calculation_model.h"
#include "logic/serializers/mcr_analysis_identity_model_json_serializer.h"
#include "logic/serializers/reference_analysis_data_table_model_json_serializer.h"
#include "logic/serializers/pca_analysis_request_model_json_serializer.h"
#include "logic/serializers/pca_analysis_response_model_json_serializer.h"
#include "logic/serializers/hotteling_request_json_serializer.h"
#include "logic/serializers/hotteling_response_json_serializer.h"
#include "logic/serializers/pca_analysis_identity_model_json_serializer.h"
#include "logic/serializers/coefficient_calculation_model_json_serializer.h"

namespace Service
{
  using namespace Model;

  JsonServicePackage::JsonServicePackage()
  {
    ServiceLocator::Instance().RegisterService<IJsonCombiner, JSonCombiner>();
    ServiceLocator::Instance().RegisterService<PlsFitResponseModel, IJsonSerializer, PlsFitResponseJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PlsFitRequestModel, IJsonSerializer, PlsFitRequestJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PlsAnalysisListModel, IJsonSerializer, PlsAnalysisListSerializer>();
    ServiceLocator::Instance().RegisterService<PlsAnalysisIdentityModel, IJsonSerializer, PlsAnalysisIdentityModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PlsAnalysisIdentityModel, IJsonSerializer, PlsAnalysisIdentityModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PlsPredictResponseModel, IJsonSerializer, PlsPredictResponseJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PlsPredictAnalysisModel, IJsonSerializer, PlsPredictAnalysisJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PlsPredictParentAnalysisListModel, IJsonSerializer, PlsPredictParentAnalysisListJsonSerializer>();
    ServiceLocator::Instance().RegisterService<McrAnalysisRequestModel, IJsonSerializer, McrAnalysisRequestModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<McrAnalysisResponseModel, IJsonSerializer, McrAnalysisResponseModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<McrAnalysisIdentityModel, IJsonSerializer, McrAnalysisIdentityJsonSerializer>();
    ServiceLocator::Instance().RegisterService<MarkupReferenceModel, IJsonSerializer, MarkupReferenceModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<ReferenceAnalysisDataTableModel, IJsonSerializer, ReferenceAnalysisDataTableModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PcaAnalysisRequestModel, IJsonSerializer, PcaAnalysisRequestModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PcaAnalysisResponseModel, IJsonSerializer, PcaAnalysisResponseModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<HottelingRequestModel, IJsonSerializer, HottelingRequestJsonSerialzier>();
    ServiceLocator::Instance().RegisterService<HottelingResponseModel, IJsonSerializer, HottelingResponseJsonSerializer>();
    ServiceLocator::Instance().RegisterService<PcaAnalysisIdentityModel, IJsonSerializer, PcaAnalysisIdentitytModelJsonSerializer>();
    ServiceLocator::Instance().RegisterService<CoefficientCalculationModel, IJsonSerializer, CoefficientCalculationModelJsonSerializer>();
  }
}
