#include "logViewer.h"
#include "ImGuiFileDialog.h"

#include <imgui/imgui.h>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

struct LogLine
{
   enum LogLevel { FATAL, WARN, INFO, VERBOSE, DDEBUG, UNKNOWN };

   LogLevel level;
   std::string channel;
   std::string timestamp;
   std::string message;
};

std::vector<LogLine> messages;
typedef std::map<std::string, bool> ChannelMap;
ChannelMap channels;

bool myFatalIsEnabled = true;
bool myWarnIsEnabled = true;
bool myInfoIsEnabled = true;
bool myVerbosIsEnabled = false;
bool myDebugIsEnabled = false;
bool myUnknownIsEnabled = true;
bool myColorizeText = true;

std::string selectedFile;

ImFilePicker myOpenFileDialog;

void addCategory(const std::string& c)
{
   std::map<std::string, bool>::iterator iter = channels.find(c);
   if(iter != channels.end())
   {
      return;
   }

   channels[c] = true;
}

ImVec4 levelColor(LogLine::LogLevel level)
{
   switch(level)
   {
   case LogLine::FATAL: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
   case LogLine::WARN: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
   case LogLine::INFO: return ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
   case LogLine::VERBOSE: return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
   case LogLine::DDEBUG: return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
   case LogLine::UNKNOWN: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
   default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
   }
}

void clearMessages()
{
   messages.clear();
   channels.clear();
   addCategory("Unknown");
}

LogLine parseLine(const std::string& line)
{
   LogLine ll;
   ll.level = LogLine::UNKNOWN;
   int start = 0;

   int format = 0; //0 = unknown, 1=VRE, 2=VRV

   //VRE Formating
   if(line.compare(0, 6, "FATAL[") == 0) { ll.level = LogLine::FATAL; start = 6; format = 1; }
   if(line.compare(0, 5, "WARN[") == 0) { ll.level = LogLine::WARN; start = 5; format = 1; }
   if(line.compare(0, 5, "INFO[") == 0) { ll.level = LogLine::INFO; start = 5; format = 1; }
   if(line.compare(0, 8, "VERBOSE[") == 0) { ll.level = LogLine::VERBOSE; start = 8; format = 1; }
   if(line.compare(0, 6, "DEBUG[") == 0) { ll.level = LogLine::DDEBUG; start = 6; format = 1; }

   //VRV formating
   if(line.compare(0, 6, "Fatal:") == 0) { ll.level = LogLine::FATAL; start = 6; format = 2; }
   if(line.compare(0, 8, "Warning:") == 0) { ll.level = LogLine::WARN; start = 8; format = 2; }
   if(line.compare(0, 5, "Info:") == 0) { ll.level = LogLine::INFO;  start = 5; format = 2; }
   if(line.compare(0, 8, "Verbose:") == 0) { ll.level = LogLine::VERBOSE; start = 8; format = 2; }
   if(line.compare(0, 6, "Debug:") == 0) { ll.level = LogLine::DDEBUG; start = 6; format = 2; }

   //DIGUY formating
   if(line.compare(0, 6, "FATAL:") == 0) { ll.level = LogLine::FATAL; start = 6; format = 3; }
   if(line.compare(0, 8, "WARNING:") == 0) { ll.level = LogLine::WARN; start = 8; format = 3; }
   if(line.compare(0, 5, "INFO:") == 0) { ll.level = LogLine::INFO; start = 5; format = 3; }
   if(line.compare(0, 8, "VERBOSE:") == 0) { ll.level = LogLine::VERBOSE; start = 8; format = 3; }
   if(line.compare(0, 6, "DEBUG:") == 0) { ll.level = LogLine::DDEBUG; start = 6; format = 3; }


   switch(format)
   {
   case 0:
   {
      ll.level = LogLine::UNKNOWN;
      ll.channel = "Unknown";
      ll.timestamp = "";
      ll.message = line;
   }
   break;
   case 1:
   {
      int end = line.find_first_of(']');
      ll.channel = line.substr(start, end - start);
      addCategory(ll.channel);
      start = end + 2;
      ll.timestamp = line.substr(start, 12);
      start += 13; //add one for space
      ll.message += line.substr(start);
   }
   break;
   case 2:
   {
      ll.channel = "VR-Vantage";
      addCategory(ll.channel);
      ll.timestamp = "";
      start += 1;
      ll.message += line.substr(start);
   }
   break;
   case 3:
   {
      ll.channel = "DI-Guy";
      addCategory(ll.channel);
      ll.timestamp = "";
      start += 1;
      ll.message += line.substr(start);
   }
   break;
   }


   return ll;
}

void parseLogFile(const char* filename)
{
   clearMessages();

   std::ifstream file(filename);
   if(file.is_open())
   {
      while(!file.eof())
      {
         std::string line;
         std::getline(file, line);
         LogLine ll = parseLine(line);
         messages.push_back(ll);
      }

      file.close();
   }
}

const char* levelToString(LogLine::LogLevel level)
{
   switch(level)
   {
   case LogLine::FATAL: return "FATAL";
   case LogLine::WARN: return "WARN";
   case LogLine::INFO: return "INFO";
   case LogLine::VERBOSE: return "VERBOSE";
   case LogLine::DDEBUG: return "DEBUG";
   case LogLine::UNKNOWN: return "UNKNOWN";
   }
}

void doUi()
{
   bool openFileDialog = false;
   if(ImGui::BeginMainMenuBar())
   {
      if(ImGui::BeginMenu("File"))
      {
         if(ImGui::MenuItem("Open"))
         {
            openFileDialog = true;
            selectedFile = "./";
         }

         if(ImGui::MenuItem("Close"))
         {
            clearMessages();
         }

         ImGui::Separator();
         if(ImGui::MenuItem("Exit"))
         {
            exit(0);
         }

         ImGui::EndMenu();
      }

      if(ImGui::BeginMenu("View"))
      {
         ImGui::Checkbox("Colorize Text", &myColorizeText);
         ImGui::EndMenu();
      }


      ImGui::EndMainMenuBar();
   }

   ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
   ImVec2 sz = ImGui::GetIO().DisplaySize;
   sz.y -= 20;
   ImGui::SetNextWindowSize(sz, ImGuiCond_Always);

   if(ImGui::Begin("Entity View", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
   {
      //render severity interface
      {
         if(ImGui::Button("All Levels"))
         {
            myFatalIsEnabled = true;
            myWarnIsEnabled = true;
            myInfoIsEnabled = true;
            myVerbosIsEnabled = true;
            myDebugIsEnabled = true;
            myUnknownIsEnabled = true;
         }
         ImGui::SameLine();

         ImGui::PushStyleColor(ImGuiCol_Text, levelColor(LogLine::FATAL));
         if(ImGui::Checkbox("FATAL", &myFatalIsEnabled))
         {
            if(ImGui::GetIO().KeyShift)
            {
               myFatalIsEnabled = true;
               myWarnIsEnabled = false;
               myInfoIsEnabled = false;
               myVerbosIsEnabled = false;
               myDebugIsEnabled = false;
               myUnknownIsEnabled = false;
            }
         }
         ImGui::PopStyleColor();
         ImGui::SameLine();

         ImGui::PushStyleColor(ImGuiCol_Text, levelColor(LogLine::WARN));
         if(ImGui::Checkbox("WARN", &myWarnIsEnabled))
         {
            if(ImGui::GetIO().KeyShift)
            {
               myFatalIsEnabled = false;
               myWarnIsEnabled = true;
               myInfoIsEnabled = false;
               myVerbosIsEnabled = false;
               myDebugIsEnabled = false;
               myUnknownIsEnabled = false;
            }
         }
         ImGui::PopStyleColor();
         ImGui::SameLine();

         ImGui::PushStyleColor(ImGuiCol_Text, levelColor(LogLine::INFO));
         if(ImGui::Checkbox("INFO", &myInfoIsEnabled))
         {
            if(ImGui::GetIO().KeyShift)
            {
               myFatalIsEnabled = false;
               myWarnIsEnabled = false;
               myInfoIsEnabled = true;
               myVerbosIsEnabled = false;
               myDebugIsEnabled = false;
               myUnknownIsEnabled = false;
            }
         }
         ImGui::PopStyleColor();
         ImGui::SameLine();

         ImGui::PushStyleColor(ImGuiCol_Text, levelColor(LogLine::VERBOSE));
         if(ImGui::Checkbox("VERBOSE", &myVerbosIsEnabled))
         {
            if(ImGui::GetIO().KeyShift)
            {
               myFatalIsEnabled = false;
               myWarnIsEnabled = false;
               myInfoIsEnabled = false;
               myVerbosIsEnabled = true;
               myDebugIsEnabled = false;
               myUnknownIsEnabled = false;
            }
         }
         ImGui::PopStyleColor();
         ImGui::SameLine();

         ImGui::PushStyleColor(ImGuiCol_Text, levelColor(LogLine::DDEBUG));
         if(ImGui::Checkbox("DEBUG", &myDebugIsEnabled))
         {
            if(ImGui::GetIO().KeyShift)
            {
               myFatalIsEnabled = false;
               myWarnIsEnabled = false;
               myInfoIsEnabled = false;
               myVerbosIsEnabled = false;
               myDebugIsEnabled = true;
               myUnknownIsEnabled = false;
            }
         }
         ImGui::PopStyleColor();
         ImGui::SameLine();

         ImGui::PushStyleColor(ImGuiCol_Text, levelColor(LogLine::UNKNOWN));
         if(ImGui::Checkbox("UNKNOWN", &myUnknownIsEnabled))
         {
            if(ImGui::GetIO().KeyShift)
            {
               myFatalIsEnabled = false;
               myWarnIsEnabled = false;
               myInfoIsEnabled = false;
               myVerbosIsEnabled = false;
               myDebugIsEnabled = false;
               myUnknownIsEnabled = true;
            }
         }
         ImGui::PopStyleColor();
      }

      ImGui::Separator();

      if(ImGui::Button("All Channels"))
      {
         ChannelMap::iterator iter = channels.begin();
         for(iter; iter != channels.end(); ++iter)
         {
            iter->second = true;
         }
      }
      ImGui::SameLine();

      int j = 0;
      ChannelMap::iterator iter = channels.begin();
      for(iter; iter != channels.end(); ++iter)
      {
         bool val = iter->second;;
         if(ImGui::Checkbox(iter->first.c_str(), &val))
         {
            if(ImGui::GetIO().KeyShift)
            {
               for(ChannelMap::iterator iter2 = channels.begin(); iter2 != channels.end(); ++iter2)
               {
                  iter2->second = false;
               }
               iter->second = true;
            }
            else
            {
               iter->second = val;
            }
         }

         j++;
         float x = ImGui::GetCursorPos().x;
         float mx = ImGui::GetContentRegionAvailWidth();

         if(j % 11 != 0)
         {
            ImGui::SameLine();
         }
         else
         {
            ImGui::NewLine();
         }
      }
      ImGui::NewLine();
      ImGui::Separator();

      float h1 = 75;
      float h2 = 125;
      float h3 = 100;
      float h4 = ImGui::GetWindowContentRegionMax().x - h1 - h2 - h3;

      ImGui::Columns(4, "Column Headers", false);
      ImGui::SetColumnWidth(0, h1);
      ImGui::SetColumnWidth(1, h2);
      ImGui::SetColumnWidth(2, h3);
      ImGui::SetColumnWidth(3, h4);

      ImGui::Text("Severity"); ImGui::NextColumn();
      ImGui::Text("Channel"); ImGui::NextColumn();
      ImGui::Text("Timestamp"); ImGui::NextColumn();
      ImGui::Text("Message"); ImGui::NextColumn();

      ImGui::Columns(1);

      ImGui::Separator();

      if(ImGui::BeginChild("Log Text"))
      {
         ImGui::Columns(4, "Log Messages", false);
         ImGui::SetColumnWidth(0, h1);
         ImGui::SetColumnWidth(1, h2);
         ImGui::SetColumnWidth(2, h3);
         ImGui::SetColumnWidth(3, h4);

         for(int i = 0; i < messages.size(); i++)
         {
            if(messages[i].level == LogLine::FATAL && !myFatalIsEnabled) continue;
            if(messages[i].level == LogLine::WARN && !myWarnIsEnabled) continue;
            if(messages[i].level == LogLine::INFO && !myInfoIsEnabled) continue;
            if(messages[i].level == LogLine::VERBOSE && !myVerbosIsEnabled) continue;
            if(messages[i].level == LogLine::DDEBUG && !myDebugIsEnabled) continue;
            if(messages[i].level == LogLine::UNKNOWN && !myUnknownIsEnabled) continue;

            if(channels[messages[i].channel] == false) continue;

            ImGui::PushStyleColor(ImGuiCol_Text, levelColor(messages[i].level));
            ImGui::Text(levelToString(messages[i].level)); ImGui::NextColumn();
            ImGui::Text(messages[i].channel.c_str()); ImGui::NextColumn();
            ImGui::Text(messages[i].timestamp.c_str()); ImGui::NextColumn();
            ImGui::PopStyleColor();

            if(myColorizeText)
               ImGui::PushStyleColor(ImGuiCol_Text, levelColor(messages[i].level));

            ImGui::TextWrapped(messages[i].message.c_str()); ImGui::NextColumn();

            if(myColorizeText)
               ImGui::PopStyleColor();
         }

         ImGui::EndChild();
      }

      ImGui::End();


      if(openFileDialog)
      {
         myOpenFileDialog.filterExtensions.clear();
         myOpenFileDialog.filterExtensions.push_back("txt");
         myOpenFileDialog.filterExtensions.push_back("log");
         ImGui::OpenPopup(ImFilePicker::filePickerId);
      }

      //handle the open file dialog
      std::string selected = "";
      if(myOpenFileDialog.doUi(selected) == true)
      {
         parseLogFile(selected.c_str());
      }
   }
}
