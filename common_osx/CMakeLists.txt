project (common)
add_library(common esShader.c  esShapes.c  esTransform.c  esUtil.c)
install(TARGETS common DESTINATION lib)
install(FILES esUtil.h DESTINATION include)

