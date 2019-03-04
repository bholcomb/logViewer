#include "ImGuiFileDialog.h"
#include "imgui/imgui.h"

#ifdef UNICODE
   #undef UNICODE
#endif

#ifdef _UNICODE
   #undef _UNICODE
#endif
#include "tinydir.h"

ImFilePicker::ImFilePicker()
{
   selectedFile = "";
   currentFolder = "./";
   returnOnClick = false;
}

ImFilePicker::~ImFilePicker()
{

}

const char* ImFilePicker::filePickerId = "###FilePicker";

bool ImFilePicker::doUi(std::string& selected)
{
   bool result = false;
   bool open = true;
   ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
   if(ImGui::BeginPopupModal(ImFilePicker::filePickerId, &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
   {
      result = drawFolder(selected);
      ImGui::EndPopup();
   }

   return result;
}

std::string fullPath(std::string& path)
{
   char buffer[1024];
   return _fullpath(buffer, path.c_str(), 1024);
}

bool ImFilePicker::drawFolder(std::string& selected)
{
   ImGui::Text( (std::string("Current Folder: ") + fullPath(currentFolder)).c_str() );
   bool result = false;

   ImGui::BeginChildFrame(1, ImVec2(600, 300));
   {
      tinydir_dir dir;
      int ret = tinydir_open_sorted(&dir, (const TCHAR*)currentFolder.c_str());

      for(int i = 0; i < dir.n_files; i++)
      {
         tinydir_file file;
         tinydir_readfile_n(&dir, &file, i);

         if(file.is_dir)
         {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
            if(ImGui::Selectable((std::string((const char*)file.name) + std::string("/")).c_str(), false, ImGuiSelectableFlags_DontClosePopups))
            {
               currentFolder = fullPath(currentFolder + std::string("/") + std::string((const char*)file.name));
            }
            ImGui::PopStyleColor();
         }

         if(file.is_reg)
         {
            std::string name = (const char*)file.name;
            if(filterExtensions.size() > 0)
            {
               bool found = false;
               for(int j = 0; j < filterExtensions.size(); j++)
               {
                  if(strcmp(filterExtensions[j].c_str(), (const char*)file.extension) == 0)
                  {
                     found = true;
                  }
               }

               if(found == false)
               {
                  continue;
               }
            }

            bool isSelected = selectedFile == std::string((const char*)file.name);
            if(ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_DontClosePopups))
            {
               selectedFile =  name;
               if(returnOnClick)
               {
                  result = true;
                  selected = currentFolder + std::string("\\") + selectedFile;
                  ImGui::CloseCurrentPopup();
               }
            }

            if(ImGui::IsMouseDoubleClicked(0))
            {
               result = true;
               selected = currentFolder + std::string("\\") + selectedFile;
               ImGui::CloseCurrentPopup();
            }
         }
      }
      
      tinydir_close(&dir);

   }
   ImGui::EndChildFrame();

   if(ImGui::Button("Cancel"))
   {
      result = false;
      ImGui::CloseCurrentPopup();
   }

   if(selectedFile != "")
   {
      ImGui::SameLine();
      if(ImGui::Button("Open"))
      {
         result = true;
         selected = currentFolder + std::string("\\") + selectedFile;
         ImGui::CloseCurrentPopup();
      }
   }

   return result;
}
