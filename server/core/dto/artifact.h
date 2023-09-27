#ifndef MOONSHINE_SERVER_CORE_DTO_ARTIFACT_H_
#define MOONSHINE_SERVER_CORE_DTO_ARTIFACT_H_

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#include <oatpp-sqlite/Types.hpp>

namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(operating_system, v_int32,
     VALUE(WINDOWS, 1, "windows"),
     VALUE(LINUX, 2, "linux"),
     VALUE(MACOS, 3, "macos")
);

ENUM(artifact_type, v_int32,
     VALUE(EXE, 1, "exe"),
     VALUE(DLL, 2, "dll")
);

ENUM(architecture, v_int32,
     VALUE(i386, 1, "i386"),
     VALUE(x86_64, 2, "x86_64")
);

#include OATPP_CODEGEN_END(DTO)

}

#endif //MOONSHINE_SERVER_CORE_DTO_ARTIFACT_H_
