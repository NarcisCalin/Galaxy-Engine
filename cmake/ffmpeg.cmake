set(FFMPEG_URL_BASE https://github.com/BtbN/FFmpeg-Builds/releases/download/latest)

if(WIN32)
    set(FFMPEG_ARCHIVE ffmpeg-master-latest-win64-lgpl-shared.zip)
else()
    set(FFMPEG_ARCHIVE ffmpeg-master-latest-linux64-lgpl-shared.tar.xz)
endif()

FetchContent_Declare(
    ffmpeg-fetch
    URL ${FFMPEG_URL_BASE}/${FFMPEG_ARCHIVE}
)

FetchContent_MakeAvailable(ffmpeg-fetch)

add_library(ffmpeg INTERFACE)

target_include_directories(ffmpeg SYSTEM INTERFACE ${ffmpeg-fetch_SOURCE_DIR}/include)
target_link_directories(ffmpeg INTERFACE ${ffmpeg-fetch_SOURCE_DIR}/lib)
target_link_directories(ffmpeg INTERFACE ${ffmpeg-fetch_SOURCE_DIR}/bin)

target_link_libraries(ffmpeg INTERFACE 
    avcodec 
    avformat 
    avutil 
    swscale 
    swresample
)