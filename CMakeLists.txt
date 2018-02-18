cmake_minimum_required(VERSION 3.0)

add_executable(app app.c ffmpeg.c draw.c mongoose.c server.c)
target_link_libraries(app cairo pthread)
