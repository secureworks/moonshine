#ifndef MOONSHINE_SERVER_DTO_PAGE_H_
#define MOONSHINE_SERVER_DTO_PAGE_H_

#include "implant.h"
#include "listener.h"
#include "task.h"
#include "job.h"
#include "service.h"

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

template<class T>
struct page : public oatpp::DTO {
  DTO_INIT(page, DTO);

  DTO_FIELD(UInt32, offset);
  DTO_FIELD(UInt32, limit);
  DTO_FIELD(UInt32, count);
  DTO_FIELD(Vector < T >, items);
};

class implants_page : public page<oatpp::Object<dto::implant>> {
  DTO_INIT(implants_page, page<oatpp::Object<dto::implant>>)
};

class listeners_page : public page<oatpp::Object<dto::listener>> {
  DTO_INIT(listeners_page, page<oatpp::Object<dto::listener>>)
};

class tasks_page : public page<oatpp::Object<dto::task>> {
  DTO_INIT(tasks_page, page<oatpp::Object<dto::task>>)
};

class jobs_page : public page<oatpp::Object<dto::job>> {
  DTO_INIT(jobs_page, page<oatpp::Object<dto::job>>)
};

class services_page : public page<oatpp::Object<dto::service>> {
  DTO_INIT(services_page, page<oatpp::Object<dto::service>>)
};

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_DTO_PAGE_H_
