#include "base/log.h"
#include "base/system.h"
#include <GLFW/glfw3.h>

using namespace ssp::base;

void ProcessInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        logger.Info("esc key has pressed");
        glfwSetWindowShouldClose(window, true);
    }
}

void Repaint(GLFWwindow *window)
{   
}

int main(int argc, char const *argv[])
{
    System::SetThreadName("main");
    auto &mainLog = Logger::GetLogger();
    auto  _au     = std::make_shared<StdoutLog>();
    mainLog.BasicConfig(LogLevel::Info, "T:(%(appname)-%(threadName))[%(asctime):%(levelname)]%(message)", "filename",
                        "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    GLFWwindow *window = nullptr;
    if (glfwInit() != GLFW_TRUE) {
        logger.Info("glfwInit failed");
        return -1;
    }

    window = glfwCreateWindow(640, 480, "这是我的第一个opengl程序", nullptr, nullptr);
    // 检查窗口创建是否成功
    if (window == nullptr) {
        logger.Error("glfwCreateWindow failed");
        glfwTerminate();
        return -1;
    }

    // 使窗口成为当前上下文
    glfwMakeContextCurrent(window);

    // 窗口大小被更改时触发回调函数注册
    glfwSetWindowSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        logger.Info("window size changed to %d %d", width, height);
    });

    while (!glfwWindowShouldClose(window)) {
        ProcessInput(window);
        Repaint(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理并退出
    glfwDestroyWindow(window);
    glfwTerminate();
    logger.Info("opengl window exited!");
    return 0;
}
