target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET HEADERS
    FILES
      taktile/constants.hpp
      taktile/functions.hpp
      taktile/types.hpp
      taktile/url.hpp
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Boost::system
    simpleio::base
    simpleio::messages::xml
    ${PROJECT_NAME}::takproto
)
