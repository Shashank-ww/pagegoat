
cmake_minimum_required(VERSION 3.20)

project(PageGoat
    VERSION 0.1.0
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

include_directories(include)

# Shared PageGoat engine and services.
set(PAGEGOAT_ENGINE_SOURCES
    # Core
    engine/coregoat/company.c
    engine/coregoat/context.c
    engine/coregoat/status.c
    engine/coregoat/engine.c
    engine/coregoat/pipeline.c

    # Platform
    platform/logger/logger.c
    platform/config/config.c
    platform/network/http.c
    platform/network/http_headers.c
    platform/network/url_encode.c
    platform/network/serializer.c

    # Normalize
    services/normalize/normalize.c

    # Extractor
    services/extractor/extractor.c
    services/extractor/parser.c
    services/extractor/modules/metadata.c
    services/extractor/modules/contact.c
    services/extractor/modules/social.c
    services/extractor/modules/branding.c
    services/extractor/modules/technology.c
    services/extractor/modules/seo.c

    # Crawl
    services/crawl/crawl.c
    services/crawl/downloader.c
    services/crawl/queue.c

    # Storage and export
    services/storage/storage.c
    services/export/export.c
)

# Existing CLI.
add_executable(pagegoat
    apps/cli/main.c
    ${PAGEGOAT_ENGINE_SOURCES}
)

target_include_directories(pagegoat PRIVATE include)
target_link_libraries(pagegoat PRIVATE CURL::libcurl)

# New HTTP service.
add_executable(pagegoat-service
    apps/service/main.c
    ${PAGEGOAT_ENGINE_SOURCES}
)

target_include_directories(pagegoat-service PRIVATE include)
target_link_libraries(pagegoat-service PRIVATE CURL::libcurl)

if(WIN32)
    target_link_libraries(pagegoat-service PRIVATE ws2_32)
    target_link_libraries(pagegoat PRIVATE ws2_32)
endif()
