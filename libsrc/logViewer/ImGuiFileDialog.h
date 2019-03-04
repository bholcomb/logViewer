#pragma once

#include <string>
#include <vector>

class ImFilePicker
{
public:
   
   ImFilePicker();
   ~ImFilePicker();

   static const char* filePickerId;

   bool returnOnClick;
   std::string selectedFile;
   std::string currentFolder;
   std::vector<std::string> filterExtensions;

   bool doUi(std::string& selected);

protected:
   bool drawFolder(std::string& selected);
};