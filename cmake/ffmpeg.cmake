set(FFMPEG_URL_BASE https://github.com/BtbN/FFmpeg-Builds/releases/download/latest)

if(LINUX)
	set(FFMPEG_ARCHIVE ffmpeg-master-latest-linux64-lgpl-shared.tar.xz)
elseif(WIN32)
	set(FFMPEG_ARCHIVE ffmpeg-master-latest-win64-lgpl-shared.zip)
endif()

FetchContent_Declare(ffmpeg-fetch URL ${FFMPEG_URL_BASE}/${FFMPEG_ARCHIVE})
FetchContent_MakeAvailable(ffmpeg-fetch)

add_library(ffmpeg INTERFACE)

add_dependencies(ffmpeg ffmpeg-fetch)

target_include_directories(ffmpeg INTERFACE ${ffmpeg-fetch_SOURCE_DIR}/include)

target_link_directories(ffmpeg INTERFACE ${ffmpeg-fetch_SOURCE_DIR}/lib)
target_link_directories(ffmpeg INTERFACE ${ffmpeg-fetch_SOURCE_DIR}/bin)
target_link_libraries(ffmpeg INTERFACE avcodec avformat avutil swscale swresample)
