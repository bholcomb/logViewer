#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include "imGuiController.h"
#include "logViewer.h"

#include <iostream>

void dropFunc(GLFWwindow* win, int count , const char** files)
{
   if(count > 0)
   {
      const char* filename = files[0];
      parseLogFile(filename);
   }
}

int main(int argc, char** argv)
{
   GLFWwindow* window;

   if(!glfwInit())
   {
      std::cout << "Failed to initialize GLFW" << std::endl;
      return -1;
   }

   /* Create a windowed mode window and its OpenGL context */
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
   window = glfwCreateWindow(1280, 720, "MAK Log Viewer", NULL, NULL);
   if(!window)
   {
      glfwTerminate();
      return -1;
   }

   /* Make the window's context current */
   glfwMakeContextCurrent(window);

   /* Initialize the library */
   gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

   glfwSetDropCallback(window, dropFunc);

   glfwSwapInterval(2); //30hz


   ImGui::CreateContext();
   ImGui::StyleColorsDark();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 130");

   //turn off loading/saving ini file
   ImGui::GetIO().IniFilename = 0;

#if DEBUG
    parseLogFile("log.txt");
#endif

   /* Loop until the user closes the window */
   while(!glfwWindowShouldClose(window))
   {
      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();


      doUi();

        
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);
      glViewport(0, 0, display_w, display_h);
      glClearColor(0.2, 0.2, 0.2, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      // Rendering
      ImGui::Render();

      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(window);

      /* Poll for and process events */
      glfwPollEvents();
   }

   // Cleanup
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();

   glfwTerminate();
   
   return 0;
}