cmake_minimum_required (VERSION 3.0)

include(Coverage)

macro(module_test)
  set(id ${ARGV0}_example)

  add_executable(${id} ${id}.cpp)
  add_test(${id} ${id} COMMAND $<TARGET_FILE:${id}>)
  target_link_libraries(${id}
    PRIVATE
      jinject
  )

  # EnableCoverage(${id})
  # AddCoverage(${id})

  unset(id)
endmacro()

module_test(dependency_injection_with_service_locator)
module_test(dependency_injection_with_custom_service_locator)
module_test(dependency_injection_with_custom_instantiation)
module_test(dependency_injection_using_inheritance)
module_test(dependency_injection_using_single_instance)
module_test(dependency_injection_using_lazy_instance)
