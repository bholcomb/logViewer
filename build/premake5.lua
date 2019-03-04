solution "LogViewer"
   location("../")
   configurations { "Debug", "Release" }
   platforms{"x64"}
   startproject "logViewer"

   includedirs{"../include", "../3rdParty/include"}
   libdirs{"../lib", "../3rdParty/lib"}

   
  configuration { "Debug" }
    defines { "DEBUG", "TRACE"}
    symbols "On"
    optimize "Off"
 
  configuration { "Release" }
    optimize "Speed"
	   
project "logViewer"
	kind "ConsoleApp"
	language "C++"
	location "logViewer"
	targetdir("../bin")
	files{"../libsrc/logViewer/**.cxx", "../libsrc/logViewer/**.h", "../libsrc/imgui/**.cpp", "../libsrc/imgui/**.h", "../libsrc/glad/**.c"}
	links {"glfw3", "OpenGL32"}
	defines{"IMGUI_DISABLE_INCLUDE_IMCONFIG_H"}